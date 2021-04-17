// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/rule.h"

#include <expat.h>

#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <iterator>

#include "extractor/rule_define.h"
#include "extractor/rule_ops.h"
#include "extractor/trivial.h"

namespace ext {
namespace {
enum Node {
  APP_NODE,
  CATE_NODE,
  RULE_NODE,
  STEP_NODE,
  UNK_NODE,
};

Node NodeMapped(const std::string& name) {
  const std::unordered_map<std::string, Node> map{
      {ApplicationLayer::kName,  Node::APP_NODE  },
      {CategoryLayer::kName,     Node::CATE_NODE },
      {RuleLayer::kName,         Node::RULE_NODE },
      {StepLayer::kName,         Node::STEP_NODE },
  };
  auto iter = map.find(name);
  if (iter != map.end())
    return iter->second;
  return Node::UNK_NODE;
}

struct XmlOpaque {
  bool app_closed;
  bool cate_closed;
  bool rule_closed;

  int error;
  XML_Parser* parser;
  Attributes rule;
  std::vector<Pair> step;
  RuleTree rt;

  explicit XmlOpaque(XML_Parser* p)
      : app_closed(true),
        cate_closed(true),
        rule_closed(true),
        error(SUCCESS),
        parser(p) {
    XML_SetUserData(*parser, this);
  }
};

Attributes get_attirbutes(const char** array) {
  Attributes attrs;
  for (int i = 0; array[i]; i += 2) {
    const char* name = array[i];
    const char* value = array[i + 1];
    if (name && value)
      attrs[name] = value;
  }
  return attrs;
}

void stop_parser(XmlOpaque* op, int err) {
  XML_Parser* parser = op->parser;
  XML_StopParser(*parser, XML_FALSE);
  op->error = err;
}

#define STOP_PARSER_IF_ERROR(fun) do {  \
    int ret = fun;                      \
    if (ret != SUCCESS) {               \
      stop_parser(op, ret);             \
      return;                           \
    }                                   \
} while (0)

#define STOP_PARSER_IF(cond, err) do {  \
    if (cond) {                         \
      stop_parser(op, err);             \
      return;                           \
    }                                   \
} while (0)

#if 0
void map_handler(XmlOpaque*, const char**) {
  // <MAP Key="PHONENUM"  Type="0" />
  //
  // array[0] = "Key"
  // array[1] = "PHONENUM"
  // array[2] = "Type"
  // array[3] = "0"
}

void counter_handler(XmlOpaque*, const char**) {
  // <COUNTER C62_COUNT="0" />
  //
  // array[0] = "C62_COUNT"
  // array[1] = "0"
}
#endif

void OnElemBegin(void* data, const char* name, const char** array) {
  Node node = NodeMapped(name);
  if (node == Node::UNK_NODE)
    return;

  XmlOpaque* op = reinterpret_cast<XmlOpaque*>(data);
  Attributes attrs = get_attirbutes(array);
  RuleTree& rt = op->rt;
  switch (node) {
  case Node::APP_NODE: {
    bool closed = op->app_closed  &&
                  op->cate_closed &&
                  op->rule_closed;
    STOP_PARSER_IF(!closed, INVALID_LAYOUT);
    op->app_closed = false;
    STOP_PARSER_IF_ERROR(AppendApplication(&rt, attrs));
    break;
  }

  case Node::CATE_NODE: {
    bool not_closed = op->app_closed;
    STOP_PARSER_IF(not_closed, INVALID_LAYOUT);
    bool closed = op->cate_closed &&
                  op->rule_closed;
    STOP_PARSER_IF(!closed, INVALID_LAYOUT);
    op->cate_closed = false;
    STOP_PARSER_IF_ERROR(AppendCategory(&rt, attrs));
    break;
  }

  case Node::RULE_NODE: {
    bool not_closed = op->app_closed ||
                      op->cate_closed;
    STOP_PARSER_IF(not_closed, INVALID_LAYOUT);
    bool closed = op->rule_closed;
    STOP_PARSER_IF(!closed, INVALID_LAYOUT);
    op->rule_closed = false;
    op->rule.swap(attrs);
    break;
  }

  case Node::STEP_NODE: {
    bool not_closed = op->app_closed  ||
                      op->cate_closed ||
                      op->rule_closed;
    STOP_PARSER_IF(not_closed, INVALID_LAYOUT);
    std::copy(attrs.begin(), attrs.end(), std::back_inserter(op->step));
    break;
  }

  default: UNREACHABLE_CODE;
  } // end switch
}

void OnElemEnd(void* data, const char* name) {
  XmlOpaque* op = reinterpret_cast<XmlOpaque*>(data);
  if (op->error != SUCCESS)
    return;

  Node node = NodeMapped(name);
  if (node == Node::UNK_NODE)
    return;

  RuleTree& rt = op->rt;
  switch (node) {
  case Node::APP_NODE: {
    bool not_closed = op->app_closed;
    STOP_PARSER_IF(not_closed, INVALID_LAYOUT);
    bool closed = op->cate_closed &&
                  op->rule_closed;
    STOP_PARSER_IF(!closed, INVALID_LAYOUT);
    op->app_closed = true;
    break;
  }

  case Node::CATE_NODE: {
    bool not_closed = op->app_closed  ||
                      op->cate_closed;
    STOP_PARSER_IF(not_closed, INVALID_LAYOUT);
    bool closed = op->rule_closed;
    STOP_PARSER_IF(!closed, INVALID_LAYOUT);
    op->cate_closed = true;
    break;
  }

  case Node::RULE_NODE: {
    bool not_closed = op->app_closed  ||
                      op->cate_closed ||
                      op->rule_closed;
    STOP_PARSER_IF(not_closed, INVALID_LAYOUT);
    op->rule_closed = true;

    int ret = AppendRule(&rt, op->rule, op->step);
    STOP_PARSER_IF(ret != SUCCESS && ret != NEGATIVE_RULE, ret);

    // when an rule is constructed completely. clean temporary value
    // of rule and step in xml_opaque.
    op->rule.clear();
    op->step.clear();
    break;
  }

  case Node::STEP_NODE: {
    bool not_closed = op->app_closed  ||
                      op->cate_closed ||
                      op->rule_closed;
    STOP_PARSER_IF(not_closed, INVALID_LAYOUT);
    break;
  }

  default: UNREACHABLE_CODE;
  } // end switch
}

} // anonymous namespace

RuleTree MakeRuleTree(const char* buf, size_t len) {
  assert(buf && len > 0);
  XML_Parser parser = XML_ParserCreate("utf-8");
  XmlOpaque op(&parser);

  XML_SetElementHandler(parser, OnElemBegin, OnElemEnd);
  XML_Status st = XML_Parse(parser, buf, len, XML_TRUE);
  if (st != XML_STATUS_OK || op.error != SUCCESS) {
    std::ostringstream oss;
    XML_Error err = XML_GetErrorCode(parser);
    oss << (op.error != SUCCESS ?
            string_error(op.error) : XML_ErrorString(err))
        << " in line " << XML_GetCurrentLineNumber(parser);
    throw std::invalid_argument(oss.str());
  }
  XML_ParserFree(parser);
  return op.rt;
}

} // namespace ext
