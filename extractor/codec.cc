// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/codec.h"

#include <zlib.h>
#ifndef z_const
#define z_const
#endif
#include <iconv.h>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <iterator>
#include <system_error>

#include "extractor/trivial.h"

namespace ext {
int Utf16Decode(const char* s, size_t n, std::string* out);

namespace {
typedef int (*CodecFunc)(const char*, size_t, std::string*);

enum CompressFormat {
  ZLIB,
  GZIP,
};

int Inflate(CompressFormat fmt, const char* s, size_t n, std::string* out) {
  static const size_t buf_size = 256 << 10;
  char buf[buf_size];
  z_stream strm;
  memset(&strm, 0, sizeof(strm));
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  int wbits = MAX_WBITS;
  if (fmt == CompressFormat::GZIP)
    wbits += 16;
  int ret = inflateInit2(&strm, wbits);
  if (ret != Z_OK)
    throw std::runtime_error(strm.msg);
  strm.next_in = (z_const Bytef*)s;
  strm.avail_in = n;

  do {
    strm.next_out = (Bytef*)buf;
    strm.avail_out = buf_size;
    ret = inflate(&strm, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR) {
      throw std::logic_error("stream error");
    } else if (ret == Z_DATA_ERROR
        || ret == Z_MEM_ERROR
        || ret == Z_NEED_DICT) {
      ret = inflateEnd(&strm);
      if (ret == Z_STREAM_ERROR)
        throw std::logic_error("stream error");
      return UNCOMPRESS_FAILED;
    }
    size_t have = buf_size - strm.avail_out;
    out->append(buf, have);
  } while (ret != Z_STREAM_END && ret != Z_OK);
  ret = inflateEnd(&strm);
  if (ret == Z_STREAM_ERROR)
    throw std::logic_error("stream error");
  return SUCCESS;
}

// This array must have signed type.
const int8_t kBase64Bytes[128] = {
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
     -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  0x3E,  -1,   -1,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,  -1,   -1,
     -1,   -1,   -1,   -1,   -1,  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,  -1,   -1,   -1,   -1,  0x3F,
     -1,  0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33,  -1,   -1,   -1,   -1,   -1
};

const char kBase64UrlSafeChars[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

const char kPadChar = '=';

// Converts a char (8 bits) into a 6-bit value for decoding. If the input char
// is invalid for base64 encoding, the return value has at least its upper 25
// bits set.
inline uint32_t Convert(char x) {
  // If x < 128, then we look up x in the table. If x is valid, then the table
  // will have a value <= 0x3F, otherwise the table will have -1. If x >= 128,
  // we still do some table lookup, but the value is ignored since we explicitly
  // set the high bit of y to 1. Either way, y is negative (high bit set) in
  // case of error.
  const int8_t y = kBase64Bytes[x & 0x7F] | (x & 0x80);
  // Casting from int8 to int32 preserves sign by sign extension. If y was
  // negative, at least its 25 high bits of the return value are set.
  const int32_t z = static_cast<int32_t>(y);
  return static_cast<uint32_t>(z);
}

int DecodeThreeChars(const char* codes, char* result) {
  const uint32_t packed = (Convert(codes[0]) << 18) | (Convert(codes[1]) << 12) |
                        (Convert(codes[2]) << 6) | (Convert(codes[3]));
  // Convert() return value has upper 25 bits set if input is invalid.
  // Therefore `packed` has high bits set iff at least one of code is invalid.
  if ((packed & 0xFF000000) != 0) {
    return DECODE_FAILED;
  }
  result[0] = static_cast<char>(packed >> 16);
  result[1] = static_cast<char>(packed >> 8);
  result[2] = static_cast<char>(packed);
  return SUCCESS;
}

inline bool ascii_isodigit(unsigned char c) {
  return c >= '0' && c <= '7';
}

inline bool ascii_isddigit(unsigned char c) {
  return c >= '0' && c <= '9';
}

bool ascii_isddigit_n(const char* s, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (!ascii_isddigit(s[i]))
      return false;
  }
  return true;
}

inline bool ascii_isxdigit(unsigned char c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

bool ascii_isxdigit_n(const char* s, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (!ascii_isxdigit(s[i]))
      return false;
  }
  return true;
}

inline int hex_digit_to_int(char c) {
  int x = static_cast<unsigned char>(c);
  if (x > '9')
    x += 9;
  return x & 0x0F;
}

inline int dec_digit_to_int(char c) {
  return c - '0';
}

std::string UTF16HexStringToUTF8(const char* s) {
  std::string buf;
  const uint16_t value = (hex_digit_to_int(s[0]) << 12) |
                         (hex_digit_to_int(s[1]) << 8)  |
                         (hex_digit_to_int(s[2]) << 4)  |
                         (hex_digit_to_int(s[3]) << 0);
  if (Utf16Decode(reinterpret_cast<const char*>(&value), 2, &buf) != SUCCESS)
    buf.clear();
  return buf;
}
} // Anonymous namespace

int ZlibUncompress(const char* s, size_t n, std::string* out) {
  assert(s && n > 0 && out);
  uint16_t value = *reinterpret_cast<const uint16_t*>(s);
  bool is_zlib = (value & 0x0F00) == 0x0800 && value % 31 == 0;
  if (!is_zlib)
    return UNCOMPRESS_FAILED;
  return Inflate(CompressFormat::ZLIB, s, n, out);;
}

int GzipUncompress(const char* s, size_t n, std::string* out) {
  assert(s && n > 0 && out);
  uint16_t value = *reinterpret_cast<const uint16_t*>(s);
  bool is_gzip = value == 0x8B1F;
  if (!is_gzip)
    return UNCOMPRESS_FAILED;
  return Inflate(CompressFormat::GZIP, s, n, out);
}

int UrlDecode(const char* s, size_t n, std::string* out) {
  assert(s && n > 0 && out);
  size_t i = 0;
  while (i < n) {
    switch (s[i]) {
    case '%': {
      if (i + 2 < n) {
        if (ascii_isxdigit(s[i + 1]) && ascii_isxdigit(s[i + 2])) {
          ++i;
          out->push_back(hex_digit_to_int(s[i]) << 4 |
                         hex_digit_to_int(s[i + 1]));
          i += 2;
        } else {
          out->push_back(s[i]);
          out->push_back(s[i + 1]);
          out->push_back(s[i + 2]);
          i += 3;
        }
        break;
      }
    }
    /* no break, save origin character when it has broken */
    default:
      out->push_back(s[i] == '+' ? ' ' : s[i]);
      ++i;
      break;
    }
  }
  return SUCCESS;
}

int Base64Decode(const char* s, size_t n, std::string* out) {
  assert(s && n > 0 && out);
  // This decoding procedure will write 3 * ceil(data.size() / 4) bytes to be
  // output buffer, then truncate if necessary. Therefore we must overestimate
  // and allocate sufficient amount. Currently max_decoded_size may overestimate
  // by up to 3 bytes.
  const size_t max_decoded_size = 3 * (n / 4) + 3;
  std::unique_ptr<char[]> buffer(new char[max_decoded_size]);
  char* current = buffer.get();
  if (!current)
    throw std::runtime_error("No enough memory");

  const char* b64 = s;
  const char* end = s + n;
  int ret = SUCCESS;

  while (end - b64 > 4) {
    ret = DecodeThreeChars(b64, current);
    if (ret != SUCCESS)
      return ret;
    b64 += 4;
    current += 3;
  }

  if (end - b64 == 4) {
    // The data length is a multiple of 4. Check for padding.
    // Base64 cannot have more than 2 paddings.
    if (b64[2] == kPadChar && b64[3] == kPadChar) {
      end -= 2;
    }
    if (b64[2] != kPadChar && b64[3] == kPadChar) {
      end -= 1;
    }
  }

  const int remain = static_cast<int>(end - b64);
  if (remain == 1) {
    // Check this condition early by checking data.size() % 4 == 1.
    return DECODE_FAILED;
  }

  // A valid base64 character will replace paddings, if any.
  char tail[4] = {kBase64UrlSafeChars[0], kBase64UrlSafeChars[0],
                  kBase64UrlSafeChars[0], kBase64UrlSafeChars[0]};
  // Copy tail of the input into the array, then decode.
  memcpy(tail, b64, remain * sizeof(*b64));
  ret = DecodeThreeChars(tail, current);
  if (ret != SUCCESS)
    return ret;
  // How many parsed characters are valid.
  current += remain - 1;

  out->assign(buffer.get(), current - buffer.get());
  return SUCCESS;
}

int Utf16Decode(const char* s, size_t n, std::string* out) {
  assert(s && n > 0 && out);
  iconv_t id = iconv_open("UTF-8", "UTF-16");
  if (id == iconv_t(-1)) {
    throw std::system_error(errno,
        std::system_category(), "iconv_open failed");
  }
  out->resize(n * 2);
  size_t out_size = out->size();
  char* in_ptr = const_cast<char*>(s);
  char* out_ptr = const_cast<char*>(out->data());
  if (iconv(id, &in_ptr, &n, &out_ptr, &out_size) == size_t(-1)) {
    iconv_close(id);
    return DECODE_FAILED;
  }
  out->erase(out->size() - out_size);
  iconv_close(id);
  return SUCCESS;
}

int ConvertDecode(const char* s, size_t n, std::string* out) {
  assert(s && n > 0 && out);
  out->resize(n);
  std::reverse_copy(s - 1, s + n - 1, std::back_inserter(*out));
  return SUCCESS;
}

int EscapeDecode(const char* s, size_t n, std::string* out) {
  assert(s && n > 0 && out);
  out->resize(n);
  char* d = const_cast<char*>(out->data());
  const char* end = s + n;

  while (s < end) {
    if (s[0] == '\\' && s + 1 < end) {
      // \\, \', \", \xXX, \uXXXX
      switch (s[1]) {
      case '\\':
        *d++ = '\\';
        s += 2;
        break;
      case '\'':
        *d++ = '\'';
        s += 2;
        break;
      case '"':
        *d++ = '\"';
        s += 2;
        break;
      case 'x': {
        if (s + 3 < end && ascii_isxdigit_n(s + 2, 2)) {
          unsigned int ch = (hex_digit_to_int(s[2]) << 4) | hex_digit_to_int(s[3]);
          ch &= 0x000000FF;
          *d++ = ch;
          s += 4;
        } else {
          *d++ = *s++;
        }
        break;
      }
      case 'u': {
        if (s + 5 < end && ascii_isxdigit_n(s + 2, 4)) {
          std::string buf = UTF16HexStringToUTF8(s + 2);
          if (!buf.empty()) {
            memcpy(d, buf.data(), buf.size());
            d += buf.size();
            s += 6;
          } else {
            *d++ = *s++;
          }
        } else {
          *d++ = *s++;
        }
        break;
      }
      default:
        *d++ = *s++;
        break;
      } // end switch
    }  else if (s[0] == '%' && s[1] == 'u' && s + 5 < end && ascii_isxdigit_n(s + 2, 4)) {
      // %uXXXX
      std::string buf = UTF16HexStringToUTF8(s + 2);
      if (!buf.empty()) {
        memcpy(d, buf.data(), buf.size());
        d += buf.size();
        s += 6;
      } else {
        *d++ = *s++;
      }
    } else if (s[0] == '&' && s[1] == '#' && s + 6 < end && ascii_isddigit_n(s + 2, 5)) {
      // &#\XXXXX
      uint16_t value = 0;
      for (size_t i = 2; i <= 6; ++i)
        value = value * 10 + dec_digit_to_int(s[i]);
      std::string buf;
      if (Utf16Decode(reinterpret_cast<const char*>(&value), 2, &buf) == SUCCESS) {
        memcpy(d, buf.data(), buf.size());
        d += buf.size();
        s += 7;
      } else {
        *d++ = *s++;
      }
    } else {
      *d++ = *s++;
    }
  }
  out->erase(d - out->data());
  return SUCCESS;
}

int QpDecode(const char* s, size_t n, std::string* out) {
  assert(s && n > 0 && out);
  out->resize(n + 2);
  const char* end = s + n;
  char* d = const_cast<char*>(out->data());
  while (s < end) {
    if (*s != '=') {
      *d++ = *s++;
    } else {
      if (s + 2 < end && s[1] == '\r' && s[2] == '\n') {
        // skip
        s += 3;
      } else if (s + 2 < end && ascii_isxdigit(s[1]) && ascii_isxdigit(s[2])){
        *d++ = (hex_digit_to_int(s[1]) << 4) | hex_digit_to_int(s[2]);
        s += 3;
      } else {
        *d++ = *s++;
      }
    }
  }
  out->erase(d - out->data());
  return SUCCESS;
}

int Codecode(Codec::Type type, std::string* s) {
  if (type == Codec::UTF8)
    return 0;

  static const std::map<Codec::Type, CodecFunc> map{
      { Codec::GZIP,    GzipUncompress },
      { Codec::ZLIB,    ZlibUncompress },
      { Codec::DEFLATE, ZlibUncompress },

      { Codec::URL,     UrlDecode      },
      { Codec::BASE64,  Base64Decode   },
      { Codec::UNICODE, Utf16Decode    },
      { Codec::ESCAPE,  EscapeDecode   },
      { Codec::QP,      QpDecode       },
      { Codec::CONVERT, ConvertDecode  },
  };

  CodecFunc func = SafeFindOrDie(map, type);
  std::string buf;
  int ret = func(s->data(), s->size(), &buf);
  if (ret == SUCCESS)
    s->swap(buf);
  return ret;
}

int Codecode(const std::vector<Codec::Type>& types, std::string* s) {
  int ret = SUCCESS;
  for (size_t i = 0; i < types.size(); ++i) {
    ret = Codecode(types[i], s);
    if (ret != SUCCESS)
      break;
  }
  return ret;
}

} // namespace ext
