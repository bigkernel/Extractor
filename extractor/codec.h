// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#ifndef EXTRACTOR_CODEC_H_
#define EXTRACTOR_CODEC_H_

#include <vector>
#include <string>

#include "extractor/rule_define.h"

namespace ext {
// Decode s with the type and result will be written back to s.
// returns zero on success, on error, error code is returned.
int Codecode(Codec::Type type, std::string* s);

// Like as Codecode, but it decoded with more types.
int Codecode(const std::vector<Codec::Type>& types, std::string* s);
} // namespace ext

#endif // EXTRACTOR_CODEC_H_
