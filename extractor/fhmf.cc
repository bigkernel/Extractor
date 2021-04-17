// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include <arpa/inet.h>
#include <cstring>
#include <cassert>
#include "extractor/fhmf.h"

namespace ext {
const char* kMagic = "FHMF";

bool ParseFhmf(const char* buf, size_t size, Fhmf* res) {
#define STEP_AND_CHECK_OUT_OF_RANGE(pos, step, end)  \
do {                \
  pos += step;      \
  if (pos > end)    \
    return false;   \
} while (0)

  const char* end = buf + size;

  // FHMF file = File header + Field[0..n]

  /// Read file header
  /// +-------------------------------+
  /// | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
  /// +-------------------------------+
  /// | F | H | M | F | 0 | 1 | 0 | 2 |
  /// +-------------------------------+
  /// 01 = Version, 02 = File count

  // File magic
  if (memcmp(buf, kMagic, strlen(kMagic)))
    return false;
  STEP_AND_CHECK_OUT_OF_RANGE(buf, strlen(kMagic), end);

  // Version
  res->version = ntohs(*reinterpret_cast<const uint16_t*>(buf));
  //if (res->version != Fhmf::kVersion)
  //  return false;
  STEP_AND_CHECK_OUT_OF_RANGE(buf, sizeof(uint16_t), end);

  // File count
  uint16_t fcount = ntohs(*reinterpret_cast<const uint16_t*>(buf));
  STEP_AND_CHECK_OUT_OF_RANGE(buf, sizeof(uint16_t), end);

  /// Read file field
  /// +---------------------------------------+
  /// | \r | \n | \r | \n |  - |  - |  - |  - |  field
  /// +---------------------------------------+
  /// |  - |  - |  - |  - |  - |  - |  - |  - |
  /// +---------------------------------------+
  /// |  - |  - |  - |  - | \r | \n |opt count|
  /// +---------------------------------------+
  /// |  T |  L |         option value        | <- options 0
  /// +---------------------------------------+
  /// |  T |  L |         option value        | <- options 1
  /// +---------------------------------------+
  /// |  T |  L |         option value        | <- options 2
  /// +---------------------------------------+
  /// | \r | \n | \r | \n |      Payload      |
  /// +---------------------------------------+

  for (int i = 0; i < fcount; ++i) {
    Fhmf::Field field;

    // Read field magic
    static const char* kFieldMagic = "\r\n\r\n----------------\r\n";
    static const size_t kkFieldMagicSize = strlen(kFieldMagic);

    if (memcmp(buf, kFieldMagic, kkFieldMagicSize))
      return false;
    STEP_AND_CHECK_OUT_OF_RANGE(buf, kkFieldMagicSize, end);

    // option count, 1bytes
    int opt_count = *buf;
    STEP_AND_CHECK_OUT_OF_RANGE(buf, sizeof(uint8_t), end);

    // Read options
    size_t payload_len = 0;
    for (int j = 0; j < opt_count; ++j) {
      Fhmf::Field::Type type = Fhmf::Field::Type(*buf);
      STEP_AND_CHECK_OUT_OF_RANGE(buf, sizeof(*buf), end);

      int length = *buf;
      STEP_AND_CHECK_OUT_OF_RANGE(buf, sizeof(*buf), end);

      std::string value = std::string(buf, length);
      STEP_AND_CHECK_OUT_OF_RANGE(buf, length, end);

      if (type == Fhmf::Field::Type::PAYLOAD_LEN)
        payload_len = std::stoul(value);
      field.options[type] = value;
    } // options loop

    // read payload separate
    static const char* kPayloadSep = "\r\n\r\n";
    if (memcmp(buf, kPayloadSep, strlen(kPayloadSep)))
      return false;
    STEP_AND_CHECK_OUT_OF_RANGE(buf, strlen(kPayloadSep), end);

    field.payload_ptr = NULL;
    field.payload_len = 0;
    if (payload_len > 0) {
      const char* payload_ptr = buf;
      STEP_AND_CHECK_OUT_OF_RANGE(buf, payload_len, end);
      field.payload_ptr = payload_ptr;
      field.payload_len = payload_len;
    }

    res->fields.push_back(field);
  } // field loop

#undef STEP_AND_CHECK_OUT_OF_RANGE
  return true;
}

} // namespace ext
