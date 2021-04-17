// Author: yuyue/X3130 (X3130@njsecnet.com)

#ifndef EXTRACTOR_MESSAGE_H_
#define EXTRACTOR_MESSAGE_H_

#include <string>
#include <memory>
#include <map>

#include "extractor/rule_define.h"
#include "extractor/trivial.h"

namespace ext {
struct Message {
public:
  struct Slice {
    enum Type {
      // HTTP protocol slices
      HTTP_HOST,
      // HTTP_URL + HTTP_QUERY = HTTP_URL_ORIGIN
      HTTP_URL_ORIGIN,
      HTTP_URL,
      HTTP_QUERY,
      HTTP_COOKIE,
      HTTP_USERAGENT,
      HTTP_REQ_HEAD,
      HTTP_REQ,
      HTTP_RES_HEAD,
      HTTP_RES,

      // TCP/UDP protocol slices
      BIN_DOMAIN,
      BIN_SERV_IP,
      BIN_SERV_PORT,
      BIN_REQ,
      BIN_RES,
    };

    bool codec;
    std::string str;
    Slice(): codec(false) {}
  };


  Protocol::Type type;
  std::map<Slice::Type, Slice> slices;

  Message(): type(Protocol::Type::UNKNOWN) {}
};

Message Probe(const char* s, size_t n);
Message MakeHttpMessage(const char* up, size_t up_size,
                        const char* down, size_t down_size);
Message::Slice& Slice(Message* msg, Message::Slice::Type type);

} // namespace ext

#endif // EXTRACTOR_MESSAGE_H_
