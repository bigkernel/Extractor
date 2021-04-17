// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/trivial.h"

#include <cstring>
#include <cassert>
#include <algorithm>

namespace ext {
const char* string_error(int code) {
  static const char* error_str[] = {
#define ERROR_STRING(_, s) #s,
      ERROR_MAP(ERROR_STRING)
#undef ERROR_STRING
  };

  if (code < SUCCESS || code >= ERROR_CODE_LAST)
    code = UNKNOWN_ERROR;
  return error_str[code];
}

void split(const char* s, size_t size, const char* sep,
    std::vector<std::string>* res, bool ignore_empty) {
  const char* end = s + size;
  const char* sep_end = sep + strlen(sep);
  std::vector<std::string> result;
  std::string buf;
  for (; s != end; ++s) {
    auto pos = std::find(sep, sep_end, *s);
    if (pos == sep_end) {
      buf.push_back(*s);
    } else if (!buf.empty() || !ignore_empty) {
      result.push_back(buf);
      buf.clear();
    }
  }

  if (!buf.empty()) {
    result.push_back(buf);
    buf.clear();
  }
  res->swap(result);
}

bool wildcard_match(const char* s, size_t s_len, const char* p, size_t p_len) {
  // table[i][j] means what's different between s[0 ~ i - 1] and p[0 ~ j - 1]
  bool table[s_len + 1][p_len + 1];

  // On s.empty && p.empty
  table[0][0] = true;

  // On !s.empty && p.empty
  for (size_t i = 1; i <= s_len; ++i)
    table[i][0] = false;

  // On s.empty && !p.empty
  for (size_t j = 1; j <= p_len; ++j)
    table[0][j] = p[j - 1] == '*' && table[0][j - 1];

  for (size_t j = 1; j <= p_len; ++j) {
    for (size_t i = 1; i <= s_len; ++i) {
      if (p[j - 1] != '*') {
        table[i][j] = (table[i - 1][j - 1])
            && (s[i - 1] == p[j - 1] || p[j - 1] == '?');
      } else {
        table[i][j] = table[i - 1][j] || table[i][j - 1];
      }
    }
  }

  return table[s_len][p_len];
}

} // namespace ext
