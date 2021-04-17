// Author: yuyue/X3130 (X3130@njsecnet.com)

#ifndef EXTRACTOR_HTTP_PARSER1_H_
#define EXTRACTOR_HTTP_PARSER1_H_

#include "extractor/parser.h"

namespace ext {
class HttpParser: public Parser {
public:
  HttpParser(const RuleTree* rt);
  virtual ~HttpParser();

  virtual int Parse(Message* msg, RecordSet* res, Record* attrib);
};

} // namespace ext

#endif // EXTRACTOR_HTTP_PARSER1_H_
