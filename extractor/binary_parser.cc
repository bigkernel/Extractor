// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/binary_parser.h"

#include <expat.h>
#include <cassert>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <iterator>

#include "extractor/codec.h"
#include "extractor/rule_define.h"
#include "extractor/rule.h"
#include "extractor/third_party/json.h"
#include "extractor/third_party/string_view.h"

namespace ext {
typedef Message::Slice::Type SliceType;
namespace {
const Application* find_app(const RuleTree* rt, const std::string& host) {
  int off = -1;
  auto iter = rt->index.find(host);
  if (iter != rt->index.end()) {
    off = iter->second;
  }
  return off != -1 ? &(rt->apps[off]) : NULL;
}

#if 0
const Category* find_cate(const Application* app, const std::string& url) {
  int off = -1;
  auto iter = app->index.find(url);
  if (iter != app->index.end()) {
    off = iter->second;
  }
  return off != -1 ? &(app->cates[off]) : NULL;
}
#endif

template<typename Map0, typename Map1>
inline void Copy(Map0& dst, const std::string& to,
    const Map1& src, const std::string& from) {
  dst[to] = SafeFind(src, from);
}

} // Anonymous namespace

BinaryParser::BinaryParser(const RuleTree* rt)
    : Parser(rt) {}

BinaryParser::~BinaryParser() {}

int BinaryParser::Parse(Message* msg, RecordSet* res, Record* attrib) {
  assert(msg->type == Protocol::Type::TCP ||
         msg->type == Protocol::Type::UDP);
  const std::string& ip_port =
      Slice(msg, Message::Slice::Type::BIN_SERV_IP).str +
      Slice(msg, Message::Slice::Type::BIN_SERV_PORT).str;
  if (ip_port.empty())
    return INCOMPLETE_MSG;

  const Application* app = find_app(rt_, ip_port);
  if (!app || app->protocol != msg->type)
    return NOT_FOUND_RULE;

  // application layer attributes
  Copy(*attrib, "HOST_ID", app->attribute, ApplicationLayer::kID);
  Copy(*attrib, "SPECIAL_LABLE", app->attribute, ApplicationLayer::kLabel);
  (*attrib)["HOST"] = Slice(msg, SliceType::BIN_DOMAIN).str;
  Copy(*attrib, "PROTOCOL", app->attribute, ApplicationLayer::kProtocol);

  int ret = SUCCESS;
  res->clear();
  for (size_t i = 0; i < app->cates.size(); ++i) {
    const Category& cate = app->cates[i];
    ret = ParseCate(msg, *app, cate, res);
    if (ret != SUCCESS) {
      if (ret == NOT_FOUND_RULE)
        continue;
      return ret;
    }

    // ignore below categories because message has a category(actions) only
    if (!res->empty()) {
      // category layer attributes
      Copy(*attrib, "URL_ID", cate.attribute, CategoryLayer::kID);
      Copy(*attrib, "URL", cate.attribute, BinaryAttributes::kUrl);
      Copy(*attrib, "PROTOCOL_ACTION",
          cate.attribute, BinaryAttributes::kProtocolAction);
      Copy(*attrib, "APP_NAME", cate.attribute, BinaryAttributes::kAppName);
      Copy(*attrib, "ACTION", cate.attribute, BinaryAttributes::kAction);
      break;
    }
  }
  return ret;
}

int BinaryParser::ParseCate(Message* msg, const Application& app,
                            const Category& cate, RecordSet* res) {
  auto const& rules = cate.rules;
  // temporary result of the normal(unknown) rule
  Record record;
  for (size_t i = 0; i < rules.size(); ++i) {
    auto const& rule = rules[i];

    const std::string rule_id = SafeFindOrDie(rule.attribute, RuleLayer::kID);
    RuleStat tmp;
    tmp.rule_id = rule_id;
    tmp.url_id = SafeFindOrDie(cate.attribute, CategoryLayer::kID);
    tmp.host_id = SafeFindOrDie(app.attribute, ApplicationLayer::kID);
    tmp.key = rule.rule_key;
    tmp.serv_ip = SafeFindOrDie(app.attribute, BinaryAttributes::kIP);
    tmp.serv_port = SafeFindOrDie(app.attribute, BinaryAttributes::kPort);
    tmp.app_name = SafeFindOrDie(cate.attribute, BinaryAttributes::kAppName);
    ++tmp.appear;

    // codec
    std::vector<Codec::Type> codec;
    Message::Slice* slice;
    switch (rule.data_src) {
    case DataSource::Type::REQ_CONTENT:
      slice = &Slice(msg, SliceType::BIN_REQ);
      codec = cate.req_codec;
      break;
    case DataSource::Type::RES_CONTENT:
      slice = &Slice(msg, SliceType::BIN_RES);
      codec = cate.res_codec;
      break;
    default: UNREACHABLE_CODE;
    }

    if (slice->str.empty())
      continue;

    const std::string& cipher_key =
        SafeFind(app.attribute, BinaryAttributes::kCipherKey);
    if (!cipher_key.empty() &&
        string_view(slice->str).find(cipher_key) == string_view::npos) {
      return NOT_FOUND_RULE;
    }

    const std::string& plain_key =
        SafeFind(app.attribute, BinaryAttributes::kPlaintextFeature);
    if (!plain_key.empty() &&
        string_view(slice->str).find(plain_key) == string_view::npos) {
      return NOT_FOUND_RULE;
    }

    const std::string& keyword =
        SafeFind(cate.attribute, BinaryAttributes::kKeyword);
    if (!keyword.empty() &&
        string_view(slice->str).find(keyword) == string_view::npos) {
      return NOT_FOUND_RULE;
    }

    if (!slice->codec) {
      int ret = Codecode(codec, &slice->str);
      if (ret != SUCCESS)
        return ret;
      slice->codec = true;
    }

    // Parse
    switch (rule.type) {
    case RuleLayer::Type::UNKNOWN:
      ParseUKN(rule, slice->str, &record, &tmp);
      break;
    case RuleLayer::Type::JSON:
      ParseJSON(rule, slice->str, res, &tmp);
      break;
    case RuleLayer::Type::XML:
      ParseXML(rule, slice->str, res, &tmp);
      break;
    case RuleLayer::Type::F0:
      ParseF0(rule, slice->str, res, &tmp);
      break;
    case RuleLayer::Type::F1:
      ParseF1(rule, slice->str, res, &tmp);
      break;
    default: UNREACHABLE_CODE;
    }

    if (g_extract_stat)
      AddRuleStat(tmp);
  } // end of rule loop

  if (!record.empty())
    res->push_back(record);
  return SUCCESS;
}

} // namespace ext
