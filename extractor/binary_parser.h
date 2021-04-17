// Author: yuyue/X3130 (X3130@njsecnet.com)

#ifndef EXTRACTOR_BINARY_PARSER_H_
#define EXTRACTOR_BINARY_PARSER_H_

#include "extractor/parser.h"

namespace ext {
class BinaryParser: public Parser {
public:
  BinaryParser(const RuleTree* rt);
  virtual ~BinaryParser();

  virtual int Parse(Message* msg, RecordSet* res, Record* attrib);

private:
  int ParseCate(Message* msg, const Application& app,
                const Category& cate, RecordSet* res);
};

} // namespace ext

#endif // EXTRACTOR_BINARY_PARSER_H_
