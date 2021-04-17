// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#ifndef EXTRACTOR_PARSER_H_
#define EXTRACTOR_PARSER_H_

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <mutex>

#include "extractor/rule.h"
#include "extractor/message.h"
#include "extractor/third_party/string_view.h"

namespace ext {
typedef std::map<std::string, std::string> Record;
typedef std::vector<Record> RecordSet;

struct RuleStat {
  std::string rule_id;
  std::string url_id;
  std::string host_id;
  std::string key;
  std::string host;
  std::string url;
  std::string serv_ip;
  std::string serv_port;
  std::string app_name;

  uint64_t appear;
  uint64_t hit;
  uint64_t fail;

  RuleStat(): appear(0), hit(0), fail(0) {}
};

typedef std::map<std::string, RuleStat> RuleStats;

void AddRuleStat(const RuleStat& st);
RuleStats GetRuleStats();

extern bool g_output_orign_lbs;
extern bool g_extract_stat;

class Parser {
public:
  Parser(const RuleTree* rt);
  virtual ~Parser();

  // parsing the message with the rule tree, the result and common
  // attributes will push to `res' and `attrib' on success.
  virtual int Parse(Message* msg, RecordSet* res, Record* attrib) = 0;

protected:
  // all types of rule parser that used to every parser of the protocol,
  // it's ensured on success, the result should be pushed to res,
  // otherwise no anything changed.
  void ParseUKN(const Rule& rule, string_view msg, Record* res, RuleStat* st);
  void ParseJSON(const Rule& rule, string_view msg, RecordSet* res, RuleStat* st);
  void ParseXML(const Rule& rule, string_view msg, RecordSet* res, RuleStat* st);
  void ParseF0(const Rule& rule, string_view msg, RecordSet* res, RuleStat* st);
  void ParseF1(const Rule& rule, string_view msg, RecordSet* res, RuleStat* st);

  const RuleTree* rt_;
};

int ParserFactory(const RuleTree* rt, Message* msg,
                  RecordSet* res, Record* attrib);

} // namespace ext

#endif // EXTRACTOR_PARSER_H_
