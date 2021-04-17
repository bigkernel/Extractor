// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/http_parser1.h"

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
int find_by_wild(const std::vector<int>& wild_index,
                 const std::string& key,
                 const std::vector<Application>& apps) {
  for (size_t i = 0; i < wild_index.size(); ++i) {
    int idx = wild_index[i];
    const std::string& pattern =
        SafeFind(apps[idx].attribute, HttpAttributes::kHost);
    if (wildcard_match(key, pattern))
      return idx;
  }
  return -1;
}

int find_by_wild(const std::vector<int>& wild_index,
                 const std::string& key,
                 const std::vector<Category>& cates) {
  for (size_t i = 0; i < wild_index.size(); ++i) {
    int idx = wild_index[i];
    const std::string& pattern =
        SafeFindOrDie(cates[idx].attribute, HttpAttributes::kUrl);
    if (wildcard_match(key, pattern))
      return idx;
  }
  return -1;
}

const Application* find_app(const RuleTree* rt, const std::string& host) {
  int off = -1;
  auto iter = rt->index.find(host);
  if (iter != rt->index.end()) {
    off = iter->second;
  } else {
    off = find_by_wild(rt->wild_index, host, rt->apps);
  }
  return off != -1 ? &(rt->apps[off]) : NULL;
}

const Category* find_cate(const Application* app, const std::string& url) {
  int off = -1;
  auto iter = app->index.find(url);
  if (iter != app->index.end()) {
    off = iter->second;
  } else {
    off = find_by_wild(app->wild_index, url, app->cates);
  }
  return off != -1 ? &(app->cates[off]) : NULL;
}

template<typename Map0, typename Map1>
inline void Copy(Map0& dst, const std::string& to,
                 const Map1& src, const std::string& from) {
  dst[to] = SafeFind(src, from);
}

} // Anonymous namespace

HttpParser::HttpParser(const RuleTree* rt)
    : Parser(rt) {}

HttpParser::~HttpParser() {}

int HttpParser::Parse(Message* msg, RecordSet* res, Record* attrib) {
  assert(msg->type != Protocol::Type::UNKNOWN);
  const std::string& host = Slice(msg, Message::Slice::Type::HTTP_HOST).str;
  const std::string& url = Slice(msg, Message::Slice::Type::HTTP_URL).str;
  if (host.empty() || url.empty())
    return INCOMPLETE_MSG;

  const Application* app = find_app(rt_, host);
  if (!app || app->protocol != msg->type)
    return NOT_FOUND_RULE;
  const Category* cate = find_cate(app, url);
  if (!cate)
    return NOT_FOUND_RULE;

  // application layer attributes
  Copy(*attrib, "HOST_ID", app->attribute, ApplicationLayer::kID);
  Copy(*attrib, "SPECIAL_LABLE", app->attribute, ApplicationLayer::kLabel);
  (*attrib)["HOST"] = Slice(msg, SliceType::HTTP_HOST).str;
  Copy(*attrib, "PROTOCOL",
       app->attribute, ApplicationLayer::kProtocol);

  // category layer attributes
  Copy(*attrib, "URL_ID", cate->attribute, CategoryLayer::kID);
  (*attrib)["URL"] = Slice(msg, SliceType::HTTP_URL).str;
  Copy(*attrib, "PROTOCOL_ACTION",
      cate->attribute, HttpAttributes::kProtocolAction);
  Copy(*attrib, "APP_NAME", cate->attribute, HttpAttributes::kAppName);
  Copy(*attrib, "ACTION", cate->attribute, HttpAttributes::kAction);
  (*attrib)["USER_AGENT"] = Slice(msg, SliceType::HTTP_USERAGENT).str;

  auto const& rules = cate->rules;
  // temporary result of the normal(unknown) rule
  Record record;
  for (size_t i = 0; i < rules.size(); ++i) {
    auto const& rule = rules[i];

    const std::string rule_id = SafeFindOrDie(rule.attribute, RuleLayer::kID);
    RuleStat tmp;
    tmp.rule_id = rule_id;
    tmp.url_id = SafeFindOrDie(cate->attribute, CategoryLayer::kID);
    tmp.host_id = SafeFindOrDie(app->attribute, ApplicationLayer::kID);
    tmp.key = rule.rule_key;
    tmp.host = SafeFindOrDie(app->attribute, HttpAttributes::kHost);
    tmp.url = SafeFindOrDie(cate->attribute, HttpAttributes::kUrl);
    tmp.app_name = SafeFindOrDie(cate->attribute, HttpAttributes::kAppName);
    ++tmp.appear;

    // codec
    std::vector<Codec::Type> codec;
    Message::Slice* slice;
    switch (rule.data_src) {
    case DataSource::Type::URL:
      slice = &Slice(msg, SliceType::HTTP_QUERY);
      codec.push_back(Codec::Type::URL);
      break;
    case DataSource::Type::COOKIE:
      slice = &Slice(msg, SliceType::HTTP_COOKIE);
      codec.push_back(Codec::Type::URL);
      break;
    case DataSource::Type::REQ_HEAD:
      slice = &Slice(msg, SliceType::HTTP_REQ_HEAD);
      codec.push_back(Codec::Type::URL);
      break;
    case DataSource::Type::REQ_CONTENT:
      slice = &Slice(msg, SliceType::HTTP_REQ);
      codec = cate->req_codec;
      break;
    case DataSource::Type::RES_HEAD:
      slice = &Slice(msg, SliceType::HTTP_RES_HEAD);
      codec.push_back(Codec::Type::URL);
      break;
    case DataSource::Type::RES_CONTENT:
      slice = &Slice(msg, SliceType::HTTP_RES);
      codec = cate->res_codec;
      break;
    default: UNREACHABLE_CODE;
    }

    if (slice->str.empty())
      continue;
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

#ifdef OUTPUT_RULE_ID
    if (tmp.hit > 0) {
      if (!(*attrib)["RULE_ID"].empty())
        (*attrib)["RULE_ID"].push_back('|');
      (*attrib)["RULE_ID"].append(tmp.rule_id);
    }
#endif

    if (g_extract_stat)
      AddRuleStat(tmp);
  } // end of for loop

  if (!record.empty())
    res->push_back(record);
  return SUCCESS;
}

} // namespace ext
