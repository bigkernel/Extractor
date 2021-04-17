// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/parser.h"

#include <iconv.h>
#include <expat.h>
#include <endian.h>
#include <cassert>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <system_error>
#include <memory>
#include <utility>
#include <mutex>

#include "extractor/http_parser1.h"
#include "extractor/binary_parser.h"

#include "extractor/filter.h"
#include "extractor/codec.h"
#include "extractor/trivial.h"
#include "extractor/third_party/json.h"

namespace ext {
namespace {
bool iconv_to_utf8(char* ibuf, size_t* ilen,
                   char* obuf, size_t* olen,
                   const std::string& from) {
  iconv_t id = iconv_open("UTF-8", from.c_str());
  if (id == iconv_t(-1)) {
    throw std::system_error(errno,
        std::system_category(), "iconv_open failed");
  }
  size_t ret = iconv(id, &ibuf, ilen, &obuf, olen);
  iconv_close(id);
  return ret == size_t(-1) ? false : true;
}

bool IconvToUtf8(std::string* s, const std::string& from) {
  assert(!s->empty());

  size_t ilen = s->size();
  char* ibuf = &((*s)[0]);

  size_t olen = s->size() * 4;
  size_t olen_remain = olen;
  std::unique_ptr<char[]> out_buf(new char[olen]);
  char* obuf = out_buf.get();

  bool ret = iconv_to_utf8(ibuf, &ilen, obuf, &olen_remain, from);
  if (ret)
    s->assign(obuf, olen - olen_remain);
  return ret;
}

string_view Skipper(string_view msg, Skip::Type type) {
  assert(!msg.empty());
  switch (type) {
  case Skip::Type::ALL_DIGIT:
    while (!msg.empty() && std::isdigit(msg.front()))
      msg.remove_prefix(1);
    break;
  case Skip::Type::ALL_LOW_LETTER:
    while (!msg.empty() && std::islower(msg.front()))
      msg.remove_prefix(1);
    break;
  case Skip::Type::ALL_HIGH_LETTER:
    while (!msg.empty() && std::isupper(msg.front()))
      msg.remove_prefix(1);
    break;
  case Skip::Type::ALL_LETTER:
    while (!msg.empty() && std::isalpha(msg.front()))
      msg.remove_prefix(1);
    break;
  default: UNREACHABLE_CODE;
  }
  return msg;
}

string_view ReverseSkipper(string_view msg, Skip::Type type) {
  assert(!msg.empty());
  switch (type) {
  case Skip::Type::ALL_DIGIT:
    while (!msg.empty() && std::isdigit(msg.back()))
      msg.remove_suffix(1);
    break;
  case Skip::Type::ALL_LOW_LETTER:
    while (!msg.empty() && std::islower(msg.back()))
      msg.remove_suffix(1);
    break;
  case Skip::Type::ALL_HIGH_LETTER:
    while (!msg.empty() && std::isupper(msg.back()))
      msg.remove_suffix(1);
    break;
  case Skip::Type::ALL_LETTER:
    while (!msg.empty() && std::isalpha(msg.back()))
      msg.remove_suffix(1);
    break;
  default: UNREACHABLE_CODE;
  }
  return msg;
}

inline unsigned int local_digit(const char* s, int bytes, bool big_endian) {
  switch (bytes) {
  case 1: return *(uint8_t*)s;
  case 2: {
    uint16_t val = *(uint16_t*)s;
    return big_endian ? be16toh(val) : le16toh(val);
  }
  case 4: {
    uint32_t val = *(uint32_t*)s;
    return big_endian ? be32toh(val) : le32toh(val);
  }
  case 8: {
    uint64_t val = *(uint64_t*)s;
    return big_endian ? be64toh(val) : le64toh(val);
  }
  default:
    throw std::invalid_argument("invalid number of bytes");
  }
  return 0;
}

string_view Splitter(string_view msg, char sep, unsigned int exp) {
  assert(!msg.empty());
  if (msg[0] == sep)
    msg.remove_prefix(1);

  string_view::size_type p;
  while (exp-- > 0) {
    p = msg.find(sep);
    if (p == string_view::npos)
      return {};
    msg.remove_prefix(p + 1);
  }

  if (!msg.empty()) {
    p = msg.find(sep);
    if (p != string_view::npos)
      msg.remove_suffix(msg.size() - p);
  }
  return msg;
}

} // anonymous namespace

bool g_output_orign_lbs = false;
bool g_extract_stat = false;

void ParseUKNOnce(const Rule& rule, string_view msg, std::string* res) {
  auto const& key = rule.keys[0];

  for (size_t i = 0; i < rule.steps.size(); ++i) {
    if (msg.empty())
      return;
    auto const& step = rule.steps[i];

    switch (step.type) {
    case StepLayer::Type::PREFIX: {
      for (int j = 0; j < step.step; ++j) {
        auto pos = msg.find(step.s_pattern);
        if (pos == string_view::npos)
          return;
        msg.remove_prefix(pos + step.s_pattern.size());
      }
      break;
    }
    case StepLayer::Type::SUFFIX: {
      for (int j = 0; j < step.step; ++j) {
        auto pos = msg.find(step.s_pattern);
        if (pos == string_view::npos)
          return;
        msg.remove_suffix(msg.size() - pos);
      }
      break;
    }
    case StepLayer::Type::START_POS:
    case StepLayer::Type::END_POS: {
      int offset = step.s_offset;
      int max_size = msg.size();
      if (offset < 0)
        offset = max_size + offset - 1;
      if (offset < 0 || offset >= max_size)
        return;

      if (step.type == StepLayer::Type::START_POS) {
        msg.remove_prefix(offset);
      } else {
        msg.remove_suffix(max_size - offset);
      }
      break;
    }
    case StepLayer::Type::SKIP: {
      msg = Skipper(msg, step.s_skip);
      break;
    }
    case StepLayer::Type::RSKIP: {
      msg = ReverseSkipper(msg, step.s_skip);
      break;
    }
    case StepLayer::Type::LEN_LENGTH: {
#define STEP_AND_CHECK_OUT_OF_RANGE(p, s, e) \
  do {                  \
    p += s;             \
    if (p > e) return; \
  } while (0)

      if (!rule.tlv_type.empty()) { // have TLV type
        auto pos = msg.find(rule.tlv_type);
        if (pos == string_view::npos)
          return;
        const char* p = msg.data() + pos + rule.tlv_type.size();
        unsigned int len = local_digit(p, step.s_length_len, rule.big_endian);
        STEP_AND_CHECK_OUT_OF_RANGE(p, step.s_length_len, msg.end());
        msg = string_view(p, len);
        STEP_AND_CHECK_OUT_OF_RANGE(p, len, msg.end());
      } else { // not have TLV type, use rule.index
        const char* p = msg.data();
        const char* v = NULL;
        unsigned int len = 0;
        for (int j = 0; j < rule.index; ++j) {
          STEP_AND_CHECK_OUT_OF_RANGE(p, rule.type_len, msg.end());
          len = local_digit(p, step.s_length_len, rule.big_endian);
          STEP_AND_CHECK_OUT_OF_RANGE(p, step.s_length_len, msg.end());
          v = p;
          STEP_AND_CHECK_OUT_OF_RANGE(p, len, msg.end());
        }
        msg = string_view(v, len);
      }

#undef STEP_AND_CHECK_OUT_OF_RANGE
      break;
    }
    case StepLayer::Type::SPLIT: {
      msg = Splitter(msg, step.s_split, rule.index);
      break;
    }
    default: assert(false && "Unreachable code");
    }

  } // end steps loop

  if (msg.empty())
    return;
  std::string val(msg.data(), msg.size());

  if (!rule.value_encode.empty()) {
    if (Codecode(rule.value_encode, &val) != SUCCESS)
      return;
  }

  if (!rule.charset.empty()) {
    if (!IconvToUtf8(&val, rule.charset))
      return;
  }

  if (!key.filter || key.filter(&val))
    res->swap(val);
}

void Parser::ParseUKN(const Rule& rule, string_view msg, Record* res, RuleStat* st) {
  assert(rule.type == RuleLayer::Type::UNKNOWN);
  Record record;
  std::string val;
  ParseUKNOnce(rule, msg, &val);
  if (val.empty()) {
    ++st->fail;
    return;
  }
  record[rule.keys[0].key].swap(val);

  if (rule.gid > -1) {
    auto const& sub_rules = rule.sub_rules;
    for (size_t i = 0; i < sub_rules.size(); ++i) {
      val.clear();
      ParseUKNOnce(sub_rules[i], msg, &val);
      if (val.empty()) {
        ++st->fail;
        return;
      }
      record[sub_rules[i].keys[0].key].swap(val);
    }
  }

  if (!g_output_orign_lbs) {
    if (rule.keys[0].type == KeyType::LONGITUDE ||
        rule.keys[0].type == KeyType::LATITUDE) {
      if (rule.coordinate >= Coordinate::Type::UNKNOWN)
        return;
      if (rule.coordinate != Coordinate::Type::BD09) {
        std::string lon, lat;
        std::string* f_lon = &(record[rule.keys[0].key]);
        std::string* f_lat = &(record[rule.sub_rules[0].keys[0].key]);
        if (rule.keys[0].type == KeyType::LATITUDE)
          std::swap(f_lon, f_lat);
        int ret = CoordinateTranslate(
            rule.coordinate, *f_lon, *f_lat,
            Coordinate::Type::BD09, &lon, &lat);
        if (!ret) return;
        *f_lon = lon;
        *f_lat = lat;
      }
    }
  }

  res->insert(record.begin(), record.end());
  ++st->hit;
}

namespace {
Record read_json_object(const AJson::Value& root,
                        const std::vector<Rule::Key>& keys) {
  assert(root.type() == AJson::objectValue);
  Record records;
  for (size_t i = 0; i < keys.size(); ++i) {
    auto const&key = keys[i];
    if (root.isMember(key.mapped)) {
      if (root[key.mapped].type() == AJson::arrayValue
          || root[key.mapped].type() == AJson::objectValue) {
        continue;
      }
      auto value = root[key.mapped].asString();
      if (value.empty() || value == "null")
        continue;

      const auto&key = keys[i];
      if (!key.filter || key.filter(&value))
        records[key.key] = value;
    }
  }
  return records;
}

void parse_json_recursively(const AJson::Value& root,
                            const std::vector<Rule::Key>& keys,
                            RecordSet* res,
                            bool complete) {
  switch (root.type()) {
  case AJson::nullValue: break;
  case AJson::arrayValue: {
    for (auto iter = root.begin(); iter != root.end(); ++iter) {
      if (iter->type() != AJson::arrayValue
          && iter->type() != AJson::objectValue) {
        break;
      }
      parse_json_recursively(*iter, keys, res, complete);
    }
    break;
  }
  case AJson::objectValue: {
    Record records = read_json_object(root, keys);
    if (!records.empty()) {
      if (complete || res->empty()) {
        // first extract
        res->push_back(records);
      } else {
        res->back().insert(records.begin(), records.end());
        complete = false;
      }
    }

    for (auto iter = root.begin(); iter != root.end(); ++iter) {
      if (iter->type() == AJson::arrayValue
          || iter->type() == AJson::objectValue) {
        parse_json_recursively(*iter, keys, res, complete);
      }
    }

    // check numbers of record from back.
    if (complete && !res->empty()) {
      if (res->back().size() != keys.size())
        res->pop_back();
    }
  }
  break;
  default: break;
  }
}

string_view strip(string_view view,
                  const std::string& prefix,
                  const std::string& suffix) {
  if (!prefix.empty()) {
    auto pos = view.find(prefix);
    if (pos == string_view::npos)
      return {};
    view.remove_prefix(pos);
  }

  if (!suffix.empty()) {
    auto pos = view.find(suffix);
    if (pos == string_view::npos)
      return {};
    view.remove_suffix(view.size() - pos);
  }

  return view;
}

std::mutex g_rule_stat_lock;
RuleStats g_rule_stat;
} // anonymous namespace

void AddRuleStat(const RuleStat& st) {
  std::lock_guard<std::mutex> guard(g_rule_stat_lock);
  auto it = g_rule_stat.find(st.rule_id);
  if (it == g_rule_stat.end()) {
    g_rule_stat[st.rule_id] = st;
  } else {
    it->second.appear += st.appear;
    it->second.hit += st.hit;
    it->second.fail += st.fail;
  }
}

RuleStats GetRuleStats() {
  std::lock_guard<std::mutex> guard(g_rule_stat_lock);
  RuleStats ret;
  ret.swap(g_rule_stat);
  return ret;
}

void Parser::ParseJSON(const Rule& rule, string_view msg,
    RecordSet* res, RuleStat* st) {
  assert(rule.type == RuleLayer::Type::JSON);
  static auto const features = AJson::Features::all();
  AJson::Value root;
  AJson::Reader reader(features);

  msg = strip(msg, rule.head, "");
  if (!msg.empty()) {
    auto ret = reader.parse(msg.begin(), msg.end(), root, false);
    (void)ret;
    auto const& keys = rule.keys;
    size_t size = res->size();
    parse_json_recursively(root, keys, res, true);
    if (res->size() > size) {
      ++st->hit;
    } else {
      ++st->fail;
    }
  } else {
    ++st->fail;
  }
}

namespace {
struct XmlOpaque {
  const Rule* rule;
  std::vector<Record>* records;
  Record tmp;
};

void elem_begin(void* data, const char*, const char** array) {
  XmlOpaque* op = reinterpret_cast<XmlOpaque*>(data);
  Record& tmp = op->tmp;

  for (int i = 0; array[i]; i += 2) {
    const char* key = array[i];
    const char* value = array[i + 1];
    if (key && value) {
      tmp.insert({key, value});
    }
  }
}

void elem_end(void* data, const char*) {
  XmlOpaque* op = reinterpret_cast<XmlOpaque*>(data);
  const Rule* rule = op->rule;
  const auto& keys = rule->keys;
  Record& tmp = op->tmp;
  Record records;

  if (tmp.empty())
    return;

  for (size_t i = 0; i < keys.size(); ++i) {
    auto iter = tmp.find(keys[i].mapped);
    if (iter != tmp.end()) {
      if (iter->second.empty()) {
        records.clear();
        break;
      }

      auto& value = iter->second;
      const auto& key = keys[i];
      if (!key.filter || key.filter(&value))
        records[key.key] = iter->second;
    }
  }

  if (records.size() == keys.size())
    op->records->push_back(records);
  tmp.clear();
}

} // anonymous namespace

void Parser::ParseXML(const Rule& rule, string_view msg,
    RecordSet* res, RuleStat* st) {
  assert(rule.type == RuleLayer::Type::XML);
  msg = strip(msg, rule.head, rule.tail);
  if (msg.empty()) {
    ++st->fail;
    return;
  }

  XML_Parser parser = XML_ParserCreate("utf-8");
  XML_SetElementHandler(parser, elem_begin, elem_end);
  XmlOpaque opaque;
  opaque.rule = &rule;
  opaque.records = res;
  size_t size = res->size();
  XML_SetUserData(parser, &opaque);
  XML_Status ret = XML_Parse(parser, msg.data(), msg.size(), XML_TRUE);
  if (ret != XML_STATUS_OK) {
    XML_Error err = XML_GetErrorCode(parser);
    if (err == XML_ERROR_ABORTED) {
      // broken xml, but do nothings
    }
  }

  if (res->size() > size) {
    ++st->hit;
  } else {
    ++st->fail;
  }
}

void Parser::ParseF0(const Rule& rule, string_view msg,
    RecordSet* res, RuleStat* st) {
  assert(rule.type == RuleLayer::Type::F0);
  ++st->appear;
  msg = strip(msg, rule.head, rule.tail);
  if (msg.empty()) {
    ++st->fail;
    return;
  }

  size_t size = res->size();

  const auto& keys = rule.keys;
  std::vector<std::string> group;
  std::string buf;
  for (size_t i = 0; i < msg.size(); ++i) {
    if (msg[i] == rule.word_split[0]) {
      group.push_back(buf);
      buf.clear();
    } else if (msg[i] == rule.group_split[0]) {
      if (group.size() != keys.size())
        continue;
      Record records;
      for (size_t j = 0; j < keys.size(); ++j) {
        std::string& value = group[j];
        if (!value.empty()) {
          const auto& key = keys[i];
          if (!key.filter || key.filter(&value))
            records[key.key] = value;
        }
      }
      if (records.size() == keys.size())
        res->push_back(records);
      group.clear();
    } else {
      buf.push_back(msg[i]);
    }
  }

  if (res->size() > size) {
    ++st->hit;
  } else {
    ++st->fail;
  }
}

void Parser::ParseF1(const Rule& rule, string_view msg,
    RecordSet* res, RuleStat* st) {
  assert(rule.type == RuleLayer::Type::F1);
  msg = strip(msg, rule.head, rule.tail);
  if (msg.empty()) {
    ++st->fail;
    return;
  }

  size_t size = res->size();

  const auto& keys = rule.keys;
  const char* begin = msg.begin();
  const char* pos;
  const char* end;
  while (begin < msg.end()) {
    pos = begin;
    while (!(*pos >= '0' && *pos <= '9')) {
      if (pos >= msg.end())
        return;
      ++pos;
    }

    end = pos;
    while ((*pos >= '0' && *pos <= '9')) {
      if (end >= msg.end())
        return;
      ++end;
    }

    std::string values[] = {
        std::string(begin, pos),
        std::string(pos, end)
    };
    begin = end;
    if (values[0].empty() || values[1].empty())
      continue;

    Record records;
    for (size_t i = 0; i < 2; ++i) {
      const auto& key = keys[i];
      auto& value = values[i];
      if (!key.filter || key.filter(&value))
        records[key.key] = value;
    }
    if (records.size() == keys.size())
      res->push_back(records);
  }

  if (res->size() > size) {
    ++st->hit;
  } else {
    ++st->fail;
  }
}

Parser::Parser(const RuleTree* rt): rt_(rt) {}
Parser::~Parser() {}

int ParserFactory(const RuleTree* rt, Message* msg,
                  RecordSet* res, Record* attrib) {
  assert(msg->type != Protocol::Type::UNKNOWN);
  switch (msg->type) {
  case Protocol::Type::HTTP:
    return HttpParser{rt}.Parse(msg, res, attrib);
  case Protocol::Type::TCP:
  case Protocol::Type::UDP:
    return BinaryParser{rt}.Parse(msg, res, attrib);
  default: UNREACHABLE_CODE;
  }

  return 0;
}

} // namespace ext
