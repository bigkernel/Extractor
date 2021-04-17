// Author: yuyue/X3130 (X3130@njsecnet.com)

#include "extractor/rule_ops.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

#include "extractor/rule.h"
#include "extractor/filter.h"
#include "extractor/trivial.h"

namespace ext {
namespace {
inline int hex_digit_to_int(char c) {
  int x = static_cast<unsigned char>(c);
  if (x > '9') {
    x += 9;
  }
  return x & 0x0F;
}

std::string hex_to_string(const char* s, size_t n) {
  if (n % 2 != 0) {
    std::string err("Too short hex strings: ");
    err.append(s, n);
    throw std::invalid_argument(err);
  }

  std::string res;
  for (size_t i = 0; i < n; i += 2) {
    char c = hex_digit_to_int(s[i]) << 4 |
             hex_digit_to_int(s[i + 1]);
    res.push_back(c);
  }
  return res;
}

inline std::string safe_string(const char* s, size_t n) {
  static const std::string empty_str;
  return n != 0 ? std::string(s, n) : empty_str;
}

std::string PieHexToString(const char* s, size_t n) {
  static const char* kPieHexMagic = "#PIE_HEX#";
  if (n > strlen(kPieHexMagic) &&
      !memcmp(kPieHexMagic, s, strlen(kPieHexMagic))) {
    return hex_to_string(s + strlen(kPieHexMagic),
                         n - strlen(kPieHexMagic));
  } else {
    return safe_string(s, n);
  }
}

inline std::string PieHexToString(const std::string& s) {
  return PieHexToString(s.data(), s.size());
}

bool GetCodec(const std::string& s, std::vector<Codec::Type>* out) {
  std::vector<std::string> vec;
  split(s.data(), s.size(), ",", &vec, true);

  for (size_t i = 0; i < vec.size(); ++i) {
    Codec::Type type = Codec::Mapped(vec[i]);
    if (type == Codec::Type::UNKNOWN)
      return false;
    out->push_back(type);
  }
  return true;
}

// HTTP protocol
int AppendHttpApp(RuleTree* rt, Attributes& attrs) {
  const std::string& host = attrs[HttpAttributes::kHost];
  if (host.empty())
    return INVALID_RULE;
  bool wildcard = host.find("*") != std::string::npos;
  Application app;
  app.attribute = attrs;
  app.protocol = Protocol::Type::HTTP;
  size_t index = rt->apps.size();
  rt->apps.push_back(app);
  if (!wildcard) {
    rt->index.insert({host, index});
  } else {
    rt->wild_index.push_back(index);
  }
  return SUCCESS;
}

int AppendHttpCate(RuleTree* rt, Attributes& attrs) {
  const std::string& url = attrs[HttpAttributes::kUrl];
  if (url.empty())
    return INVALID_RULE;

  bool wildcard = url.find("*") != std::string::npos;
  Category cate;
  cate.attribute = attrs;
  std::string codec =
      attrs[HttpAttributes::kReqCntCompress] + "," +
      attrs[HttpAttributes::kReqCntEncode];
  if (!GetCodec(codec, &cate.req_codec))
    return UNDEFINE_METHOD;
  codec =
      attrs[HttpAttributes::kResCntCompress] + "," +
      attrs[HttpAttributes::kResCntEncode];
  if (!GetCodec(codec, &cate.res_codec))
    return UNDEFINE_METHOD;

  Application& last_app = rt->apps.back();
  size_t index = last_app.cates.size();
  last_app.cates.push_back(cate);
  if (!wildcard) {
    last_app.index.insert({url, index});
  } else {
    last_app.wild_index.push_back(index);
  }
  return SUCCESS;
}

// TCP/UDP protocol
int AppendBinaryApp(RuleTree* rt, Attributes& attrs, bool tcp) {
  const std::string& ip = attrs[BinaryAttributes::kIP];
  const std::string& port = attrs[BinaryAttributes::kPort];
  if (ip.empty() || port.empty())
    return INVALID_RULE;

  std::string& cipher_key = attrs[BinaryAttributes::kCipherKey];
  if (!cipher_key.empty())
    cipher_key = hex_to_string(cipher_key.data(), cipher_key.size());
  std::string& plain_text = attrs[BinaryAttributes::kPlaintextFeature];
  if (cipher_key.empty() && plain_text.empty())
    return INVALID_RULE;

  Application app;
  app.attribute = attrs;
  app.protocol = tcp ? Protocol::Type::TCP : Protocol::Type::UDP;
  size_t index = rt->apps.size();
  rt->apps.push_back(app);
  rt->index.insert({ip + port, index});
  return SUCCESS;
}

int AppendBinaryCate(RuleTree* rt, Attributes& attrs) {
  const std::string& action = attrs[BinaryAttributes::kAction];
  if (action.empty())
    return INVALID_RULE;

  Category cate;
  cate.attribute = attrs;
  std::string codec =
      attrs[HttpAttributes::kReqCntCompress] + "," +
      attrs[HttpAttributes::kReqCntEncode];
  if (!GetCodec(codec, &cate.req_codec))
    return UNDEFINE_METHOD;
  codec =
      attrs[HttpAttributes::kResCntCompress] + "," +
      attrs[HttpAttributes::kResCntEncode];
  if (!GetCodec(codec, &cate.res_codec))
    return UNDEFINE_METHOD;

  Application& last_app = rt->apps.back();
  size_t index = last_app.cates.size();
  last_app.cates.push_back(cate);
  last_app.index.insert({action, index});
  return SUCCESS;
}

} // anonymous namespace

int AppendApplication(RuleTree* rt, Attributes& attrs) {
  if (attrs[ApplicationLayer::kProtocol].empty())
    attrs[ApplicationLayer::kProtocol] = Protocol::kHTTP;
  Protocol::Type protocol =
      Protocol::Mapped(attrs[ApplicationLayer::kProtocol]);

  switch (protocol) {
  case Protocol::Type::HTTP:
    return AppendHttpApp(rt, attrs);
  case Protocol::Type::UDP:
  case Protocol::Type::TCP:
    return AppendBinaryApp(rt, attrs, protocol == Protocol::Type::TCP);
  default:
    return UNDEFINE_PROTO;
  }
  return SUCCESS;
}

int AppendCategory(RuleTree* rt, Attributes& attrs) {
  assert(!rt->apps.empty());
  auto& last_app = rt->apps.back();
  int protocol = last_app.protocol;

  switch (protocol) {
  case Protocol::Type::HTTP:
    return AppendHttpCate(rt, attrs);
  case Protocol::Type::UDP:
  case Protocol::Type::TCP:
    return AppendBinaryCate(rt, attrs);
  default:
    return UNDEFINE_PROTO;
  }
  return SUCCESS;
}

int AppendRule(RuleTree* rt, Attributes& attrs,
               const std::vector<Pair>& step_attrs) {
  assert(!rt->apps.empty());
  auto& last_app = rt->apps.back();
  assert(!last_app.cates.empty());
  auto& last_cate = last_app.cates.back();

  bool active = attrs[RuleLayer::kIsEffect].empty() ||
      attrs[RuleLayer::kIsEffect] == Effection::kActive ||
      attrs[RuleLayer::kIsEffect] == Effection::kAuto;
  if (!active)
    return NEGATIVE_RULE;

  Rule rule;
  std::vector<Step>& steps = rule.steps;
  for (size_t i = 0; i < step_attrs.size(); ++i) {
    const std::string& k = step_attrs[i].first;
    const std::string& v = step_attrs[i].second;
    if (v.empty())
      continue;

    Step step;
    step.type = StepLayer::Mapped(k);
    if (step.type == StepLayer::Type::UNKNOWN)
      return UNDEFINE_STEP;

    switch (step.type) {
    case StepLayer::Type::PREFIX:
    case StepLayer::Type::SUFFIX: {
      std::string buf = PieHexToString(v);
      size_t off = std::string::npos;
      step.step = std::stoi(buf, &off);
      if (off == buf.size() - 1)
        continue;
      // at the least has one byte pattern
      if (off == buf.size() || buf[off] != '-')
        return INVALID_STEP;
      ++off;
      step.s_pattern.assign(buf, off, buf.size() - off);
      break;
    }

    case StepLayer::Type::START_POS:
    case StepLayer::Type::END_POS: {
      step.s_offset = std::stoi(v);
      if (step.s_offset == 0)
        continue;
      break;
    }

    case StepLayer::Type::SKIP:
    case StepLayer::Type::RSKIP: {
      step.s_skip = Skip::Mapped(v);
      if (step.s_skip == Skip::Type::UNKNOWN)
        return INVALID_STEP;
      break;
    }

    case StepLayer::Type::VALUE_ENCODE:
      if (!GetCodec(v, &rule.value_encode))
        return UNDEFINE_METHOD;
      continue;

    case StepLayer::Type::ENDIAN:
      rule.big_endian = v[0] == '1' ? true : false;
      continue;

    case StepLayer::Type::TYPE_LENGTH:
      rule.type_len = std::stoul(v);
      continue;

    case StepLayer::Type::LEN_LENGTH:
      step.s_length_len = std::stoul(v);
      break;

    case StepLayer::Type::INDEX:
      rule.index = std::stoul(v);
      if (rule.index == 0)
        return INVALID_RULE;
      continue;

    case StepLayer::Type::TYPE:
      rule.tlv_type = PieHexToString(v);
      continue;

    case StepLayer::Type::SPLIT:
      if (v.empty())
        return INVALID_STEP;
      step.s_split = v[0];
      break;

    case StepLayer::Type::KEY: {
      Rule::Key key;
      key.key = v;
      key.type = MakeType(v);
      key.filter = FilterFactory(key.type);
      rule.keys.push_back(key);
      continue;
    }

    case StepLayer::Type::JSON:
    case StepLayer::Type::XML: {
      if (rule.keys.empty())
        return INVALID_STEP;
      Rule::Key& last_key = rule.keys.back();
      if (!last_key.mapped.empty())
        return INVALID_STEP;
      last_key.mapped = v;
      continue;
    }

    case StepLayer::Type::JSON_HEAD:
    case StepLayer::Type::XML_HEAD:
    case StepLayer::Type::HEAD:
      rule.head = PieHexToString(v);
      continue;

    case StepLayer::Type::XML_END:
    case StepLayer::Type::TAIL:
      rule.tail = PieHexToString(v);
      continue;

    case StepLayer::Type::GROUP_SPLIT:
      rule.group_split = v;
      continue;

    case StepLayer::Type::WORD_SPLIT:
      rule.word_split = v;
      continue;

    default: continue;
    } // end switch

    steps.push_back(step);
  } // end for

  rule.rule_key = attrs[RuleLayer::kKey];
  std::vector<std::string> vec;
  split(rule.rule_key.data(), rule.rule_key.size(), "-", &vec, false);
  assert(!vec.empty());
  rule.type = RuleLayer::Mapped(vec[0]);

  switch (rule.type) {
  case RuleLayer::Type::JSON:
  case RuleLayer::Type::XML: {
    for (size_t i = 0; i < rule.keys.size(); ++i) {
      if (rule.keys[i].mapped.empty())
        return INVALID_RULE;
    }
    break;
  }

  case RuleLayer::Type::F0: {
    if ((rule.group_split.empty() ||
         rule.word_split.empty())) {
      return INVALID_RULE;
    }
  } // @suppress("No break at end of case")

  case RuleLayer::Type::F1: {
    if (rule.keys.empty()) {
      Rule::Key key;

      key.key = "RELATIONSHIP_NAME";
      key.type = MakeType(key.key);
      key.filter = FilterFactory(key.type);
      rule.keys.push_back(key);

      key.key = "RELATIONSHIP_MOBILEPHONE";
      key.type = MakeType(key.key);
      key.filter = FilterFactory(key.type);
      rule.keys.push_back(key);
    }
    break;
  }

  default: {
    Rule::Key key;
    key.key = attrs[RuleLayer::kKey];
    key.type = MakeType(key.key);
    key.filter = FilterFactory(key.type);

    rule.keys.clear();
    rule.keys.push_back(key);
    break;
  }
  } // end switch

  if (rule.keys.empty())
    return INVALID_RULE;

  if (rule.type == RuleLayer::Type::JSON ||
      rule.type == RuleLayer::Type::XML) {
    for (size_t i = 0; i < rule.keys.size(); ++i) {
      if (rule.keys[i].mapped.empty())
        return INVALID_RULE;
    }
  }

  rule.attribute = attrs;
  rule.data_src = DataSource::Mapped(attrs[RuleLayer::kDataSource]);
  if (rule.data_src == DataSource::Type::UNKNOWN)
    return INVALID_RULE;
  rule.coordinate = Coordinate::Mapped(attrs[RuleLayer::kCoordinate]);
  if (!attrs[RuleLayer::kConfidence].empty())
    rule.confidence = std::stoul(attrs[RuleLayer::kConfidence]);
  if (!attrs[RuleLayer::kPriority].empty())
    rule.priority = std::stoul(attrs[RuleLayer::kPriority]);
  rule.charset = attrs[RuleLayer::kCharacterSet];

  rule.gid = -1;
  if (!attrs[RuleLayer::kGroup].empty())
    rule.gid = std::stoi(attrs[RuleLayer::kGroup]);
  if (rule.gid < 0) {
    last_cate.rules.push_back(rule);
  } else {
    auto& gids = last_cate.gids;
    auto iter = gids.find(rule.gid);
    if (iter == gids.end()) {
      // first rule in the groups, make it as head rule.
      gids[rule.gid] = last_cate.rules.size();
      last_cate.rules.push_back(rule);
    } else {
      // otherwise, make it as sub rule.
      last_cate.rules[iter->second].sub_rules.push_back(rule);
    }
  }
  return SUCCESS;
}

} // namespace ext
