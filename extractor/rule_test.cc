// Author: yuyue/X3130 (yuyue2200@hotmail.com)
// File: rule_test.cc

#include <string.h>
#include <cassert>
#include <fstream>
#include <iostream>

#include "extractor/trivial.h"
#include "extractor/rule.h"

#include "extractor/rule.h"

using namespace ext;

#define WILDCARD_HOST "*wildcard.com"
#define BEFORE_WILDCARD_URL "*/before_wildcard"
#define AFTER_WILDCARD_URL "/after_wildcard*"

static const char* valid_rule =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
    ""
    "<pIE_RULES>"
    "  <KEY_TYPE >"
    "    <MAP Key=\"PHONENUM\"  Type=\"0\"  />"
    "    <MAP Key=\"IMEI\"  Type=\"1\"  />"
    "  </KEY_TYPE>"
    ""
    "  <RULES_COUNTER>"
    "    <COUNTER C62_COUNT=\"1\" />"
    "    <COUNTER CAR_NUMBER_COUNT=\"2\" />"
    "  </RULES_COUNTER>"
    ""
    "  <HOST HostId=\"20000007927\"  Host=\"ifdist.zhiyoubao.com\" Empty=\"\" >"
    "    <URL UrlId=\"2000000792700000\"  \n"
    "          Url=\"/boss/service/code.htm.0\"  \n"
    "          ReqCntEncode=\"URL\"  ResCntEncode=\"URL\"  \n"
    "          ProtocolAction=\"(122-1,1000037116,99)\"  >"
    "      <RULE RuleId=\"1000000792700000000\"  \n"
    "            Key=\"ORDER_NUM\"  \n"
    "            DataSource=\"REQUESTCONTENT\"  \n"
    "            IsEffect=\"ACTIVE\"  \n"
    "            Time=\"2019-07-10 10:37:08\" \n"
    "            ProtocolAction=\"(122-2,1000037116,99)\"  \n"
    "            Priority=\"1\"  RuleFrom=\"HAND_TARGET\"  Required=\"1\"  >"
    "        <STEP Prefix=\"1-&lt;orderCode&gt;\"  />"
    "        <STEP Suffix=\"1-&lt;\"  />"
    "        <STEP StartPos=\"0\"  />"
    "      </RULE>"
    "      <RULE RuleId=\"1000000792700000001\"  \n"
    "             Key=\"SRC_CERTIFICATE_CODE\"  \n"
    "             DataSource=\"REQUESTCONTENT\"  \n"
    "             IsEffect=\"ACTIVE\"  \n"
    "             Time=\"2019-07-10 10:37:05\"  \n"
    "             ProtocolAction=\"(122-1,1000037116,99)\"  \n"
    "             Priority=\"1\"  RuleFrom=\"HAND_TARGET\"  Required=\"1\"  >"
    "        <STEP Prefix=\"1-&lt;certificateNo&gt;\"  />"
    "        <STEP Suffix=\"1-&lt;\"  />"
    "        <STEP StartPos=\"0\"  />"
    "      </RULE>"
    "    </URL>"
    "    <URL UrlId=\"2000000792700001\"  \n"
    "          Url=\"/boss/service/code.htm.1\"  \n"
    "          ReqCntEncode=\"URL\"  ResCntEncode=\"URL\"  \n"
    "          ProtocolAction=\"(122-1,1000037116,99)\"  >"
    "      <RULE RuleId=\"1000000792700000000\"  \n"
    "            Key=\"ORDER_NUM\"  \n"
    "            DataSource=\"REQUESTCONTENT\"  \n"
    "            IsEffect=\"ACTIVE\"  \n"
    "            Time=\"2019-07-10 10:37:08\" \n"
    "            ProtocolAction=\"(122-1,1000037116,99)\"  \n"
    "            Priority=\"1\"  RuleFrom=\"HAND_TARGET\"  Required=\"1\"  >"
    "        <STEP Prefix=\"1-&lt;orderCode&gt;\"  />"
    "        <STEP Suffix=\"1-&lt;\"  />"
    "        <STEP StartPos=\"0\"  />"
    "      </RULE>"
    "      <RULE RuleId=\"1000000792700000001\"  \n"
    "             Key=\"SRC_CERTIFICATE_CODE\"  \n"
    "             DataSource=\"REQUESTCONTENT\"  \n"
    "             IsEffect=\"ACTIVE\"  \n"
    "             Time=\"2019-07-10 10:37:05\"  \n"
    "             ProtocolAction=\"(122-1,1000037116,99)\"  \n"
    "             Priority=\"1\"  RuleFrom=\"HAND_TARGET\"  Required=\"1\"  >"
    "        <STEP Prefix=\"1-&lt;certificateNo&gt;\"  />"
    "        <STEP Suffix=\"1-&lt;\"  />"
    "        <STEP StartPos=\"0\"  />"
    "      </RULE>"
    "    </URL>"
    "  </HOST>"
    ""
    "  <HOST HostId=\"20000007929\"  Host=\"" WILDCARD_HOST "\"  >"
    "    <URL UrlId=\"2000000792700000\"  \n"
    "          Url=\"" BEFORE_WILDCARD_URL "\"  \n"
    "          ReqCntEncode=\"URL\"  ResCntEncode=\"URL\"  \n"
    "          ProtocolAction=\"(122-1,1000037116,99)\"  >"
    "      <RULE RuleId=\"1000000792700000000\"  \n"
    "            Key=\"ORDER_NUM\"  \n"
    "            DataSource=\"REQUESTCONTENT\"  \n"
    "            IsEffect=\"ACTIVE\"  \n"
    "            Time=\"2019-07-10 10:37:08\" \n"
    "            ProtocolAction=\"(122-2,1000037116,99)\"  \n"
    "            Priority=\"1\"  RuleFrom=\"HAND_TARGET\"  Required=\"1\"  >"
    "        <STEP Prefix=\"1-&lt;orderCode&gt;\"  />"
    "        <STEP Suffix=\"1-&lt;\"  />"
    "        <STEP StartPos=\"0\"  />"
    "      </RULE>"
    "    </URL>"
    "    <URL UrlId=\"2000000792700001\"  \n"
    "          Url=\"" AFTER_WILDCARD_URL "\"  \n"
    "          ReqCntEncode=\"URL\"  ResCntEncode=\"URL\"  \n"
    "          ProtocolAction=\"(122-1,1000037116,99)\"  >"
    "      <RULE RuleId=\"1000000792700000000\"  \n"
    "            Key=\"ORDER_NUM\"  \n"
    "            DataSource=\"REQUESTCONTENT\"  \n"
    "            IsEffect=\"ACTIVE\"  \n"
    "            Time=\"2019-07-10 10:37:08\" \n"
    "            ProtocolAction=\"(122-1,1000037116,99)\"  \n"
    "            Priority=\"1\"  RuleFrom=\"HAND_TARGET\"  Required=\"1\"  >"
    "        <STEP Prefix=\"1-&lt;orderCode&gt;\"  />"
    "        <STEP Suffix=\"1-&lt;\"  />"
    "        <STEP StartPos=\"0\"  />"
    "      </RULE>"
    "    </URL>"
    "  </HOST>"
    ""
    "</pIE_RULES>";

void Read(const char* fname, std::string* out) {
  char tmp[4096];
  size_t size = 0;
  std::ifstream in(fname);
  while ((size = in.readsome(tmp, 4096)) > 0)
    out->append(tmp, size);
}

static void RuleTreeTestCase(const char* s, size_t n) {
  RuleTree rt = MakeRuleTree(s, n);
  assert(rt.apps.size() == 2);
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    RuleTreeTestCase(valid_rule, strlen(valid_rule));
  } else {
    const char* rule_file = argv[1];
    std::string buf;
    Read(rule_file, &buf);
    RuleTree rt;
    rt = MakeRuleTree(buf.data(), buf.size());
  }
  return 0;
}


