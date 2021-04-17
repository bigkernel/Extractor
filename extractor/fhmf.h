// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#ifndef EXTRACTOR_FHMF_H_
#define EXTRACTOR_FHMF_H_

#include <vector>
#include <string>
#include <map>

namespace ext {

// FHMF file format
// +---------------------------------------+
// |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
// +---------------------------------------+

// +---------------------------------------+
// |  F |  H |  M |  F | version |  fcount |  head
// +---------------------------------------+

// +---------------------------------------+
// | \r | \n | \r | \n |  - |  - |  - |  - |  field
// +---------------------------------------+
// |  - |  - |  - |  - |  - |  - |  - |  - |
// +---------------------------------------+
// |  - |  - |  - |  - | \r | \n |opt count|
// +---------------------------------------+
// |  T |  L |         option value        | <- options 0
// +---------------------------------------+
// |  T |  L |         option value        | <- options 1
// +---------------------------------------+
// |  T |  L |         option value        | <- options 2
// +---------------------------------------+
// | \r | \n | \r | \n |      Payload      | <- payload separated
// +---------------------------------------+

// fhmf file = head + filed[0..n]

// head = head magic + version + field count
//        head magic is 4-bytes string value with "FHMF"
//        version is 2-bytes, big-endian, current value 1
//        field count is number of field by 2-bytes, big-endian digits

// field = field magic + options count + option + payload
//        field magic is 22-bytes string value with
//                     "\r\n\r\n----------------\r\n"
//        options count is number of options by 2-bytes, big-endian digits
//        option = 1-bytes type + 1-bytes length + string value of option
//                type: 1 - MIME_TYPE
//                      2 - FILE_NAME
//                      3 - PAYLOAD_LEN
//                      4 - DOMAIN
//                      5 - SERVER IP
//                      6 - SERVER PORT
//                length means length of the option value, when type has
//                PAYLOAD_LEN, octal digits of option value is length of the
//                below payload.

struct Fhmf {
  static const int kVersion = 1;

  struct Field {
      enum Type {
        MIME_TYPE   = 1,
        FILE_NAME   = 2,
        PAYLOAD_LEN = 3,
        DOMAIN1     = 4, // DOMAIN conflict
        SERV_IP     = 5,
        SERV_PORT   = 6,
        UNKNOWN     = 0x7F,
      };

    std::map<Type, std::string> options;
    const char* payload_ptr;
    size_t payload_len;

    Field(): payload_ptr(NULL), payload_len(0) {}
  };

  int version;
  std::vector<Field> fields;
};

bool ParseFhmf(const char* buf, size_t size, Fhmf* res);

} // namespace ext

#endif // EXTRACTOR_FHMF_H_
