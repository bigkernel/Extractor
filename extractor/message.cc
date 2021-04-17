// Author: yuyue/X3130 (X3130@njsecnet.com)

#include "extractor/message.h"

#include <cassert>
#include <cstring>
#include <map>
#include <algorithm>

#include "extractor/fhmf.h"
#include "extractor/third_party/http_parser.h"

namespace ext {
typedef Message::Slice::Type SliceType;
typedef Fhmf::Field::Type FieldType;

namespace {
// HTTP probe
struct HttpOpaque {
  std::map<SliceType, Message::Slice>* slices;
  bool on_host;
  bool on_cookie;
  bool on_user_agent;
  const char* body_begin;
  const char* body_end;

  explicit HttpOpaque(std::map<SliceType, Message::Slice>* slices)
    : slices(slices),
      on_host(false),
      on_cookie(false),
      on_user_agent(false),
      body_begin(NULL),
      body_end(NULL) {}
};

int on_url(http_parser* hp, const char* s, size_t n) {
  if (n > 0) {
    HttpOpaque* op = reinterpret_cast<HttpOpaque*>(hp->data);
    std::map<SliceType, Message::Slice>* slices = op->slices;
    (*slices)[SliceType::HTTP_URL_ORIGIN].str.assign(s, n);

    const char* end = s + n;
    const char* pos = std::find(s, end, '?');

    if (pos != end) {
      (*slices)[SliceType::HTTP_URL].str.assign(s , pos);
      ++pos;
      (*slices)[SliceType::HTTP_QUERY].str
          .assign(pos, end);
    } else {
      (*slices)[SliceType::HTTP_URL].str.assign(s, end);
    }
  }
  return n > 0 ? 0 : -1;
}

int on_header_field(http_parser* hp, const char* s, size_t n) {
  if (n > 0) {
    HttpOpaque* op = reinterpret_cast<HttpOpaque*>(hp->data);
    const char* host = "Host";
    const char* cookie = "Cookie";
    const char* user_agent = "User-Agent";

    if (n == strlen(host)
        && !strncasecmp(host, s, n)) {
      op->on_host = true;
    } else if (n == strlen(cookie)
        && !strncasecmp(cookie, s, n)) {
      op->on_cookie = true;
    } else if (n == strlen(user_agent)
        && !strncasecmp(user_agent, s, n)) {
      op->on_user_agent = true;
    }
  }
  return 0;
}

int on_header_value(http_parser* hp, const char* s, size_t n) {
  HttpOpaque* op = reinterpret_cast<HttpOpaque*>(hp->data);
  std::map<SliceType, Message::Slice>* slices = op->slices;
  if (op->on_host) {
    if (n == 0)
      return -1;
    (*slices)[SliceType::HTTP_HOST].str.assign(s, n);
    op->on_host = false;
  } else if (op->on_cookie) {
    if (n > 0)
      (*slices)[SliceType::HTTP_COOKIE].str.assign(s, n);
    op->on_cookie = false;
  } else if (op->on_user_agent) {
    if (n > 0)
      (*slices)[SliceType::HTTP_USERAGENT].str.assign(s, n);
    op->on_user_agent = false;
  }
  return 0;
}

int on_body(http_parser* hp, const char* s, size_t n) {
  if (n > 0) {
    HttpOpaque* op = reinterpret_cast<HttpOpaque*>(hp->data);
    op->body_begin = s;
    op->body_end = s + n;
  }
  return 0;
}

void init_http_parser_settings(http_parser_settings* s) {
  http_parser_settings_init(s);
  s->on_url = on_url;
  s->on_header_field = on_header_field;
  s->on_header_value = on_header_value;
  s->on_body = on_body;
}

void ParseHttp(const char* s, size_t n,
               bool up_strm,
               std::map<SliceType, Message::Slice>* slices) {
  assert(s && n > 0);
  http_parser_type type =
      up_strm ? HTTP_REQUEST : HTTP_RESPONSE;
  http_parser hp;
  http_parser_init(&hp, type);
  HttpOpaque op(slices);
  hp.data = reinterpret_cast<void*>(&op);
  http_parser_settings settings;
  init_http_parser_settings(&settings);

  http_parser_execute(&hp, &settings, s, n);
  if (hp.http_errno == HPE_CB_url ||          // not found host
      hp.http_errno == HPE_CB_header_value) { // not found url
    return;
  }

  SliceType head = up_strm ?
      SliceType::HTTP_REQ_HEAD : SliceType::HTTP_RES_HEAD;
  SliceType payload = up_strm ?
      SliceType::HTTP_REQ : SliceType::HTTP_RES;

  if (op.body_begin && op.body_end) {
    assert(op.body_begin > s);
    assert(op.body_end > op.body_begin);

    n = op.body_begin - s;
    (*slices)[head].str.assign(s, n);
    n = op.body_end - op.body_begin;
    (*slices)[payload].str.assign(op.body_begin, n);
  } else {
    (*slices)[head].str.assign(s, n);
  }
}

void ProbeHttp(const Fhmf::Field& field,
                  std::map<SliceType, Message::Slice>* slices) {
  assert(field.payload_len != 0);
  const auto& opts = field.options;
  bool upstrm =
      SafeFind(opts, FieldType::FILE_NAME) == "Request.http";
  ParseHttp(field.payload_ptr, field.payload_len, upstrm, slices);
}

void ProbeBinary(const Fhmf::Field& field,
                 std::map<SliceType, Message::Slice>* slices) {
  const auto& opts = field.options;
  const std::string& fname =
      SafeFind(opts, FieldType::FILE_NAME);
  bool upstrm = fname == "Request.tcp" || fname == "Request.udp";
  if (upstrm) {
    (*slices)[SliceType::BIN_REQ].str
        .assign(field.payload_ptr, field.payload_len);
  } else {
    (*slices)[SliceType::BIN_RES].str
         .assign(field.payload_ptr, field.payload_len);
  }

  (*slices)[SliceType::BIN_DOMAIN].str =
      SafeFind(opts, FieldType::DOMAIN1);
  (*slices)[SliceType::BIN_SERV_IP].str =
      SafeFind(opts, FieldType::SERV_IP);
  (*slices)[SliceType::BIN_SERV_PORT].str =
      SafeFind(opts, FieldType::SERV_PORT);

}

Protocol::Type MessageTypeMapped(const std::string& key) {
  static const std::map<std::string, Protocol::Type> map{
    { "application/http", Protocol::Type::HTTP   },
    { "application/tcp",  Protocol::Type::TCP },
    { "application/udp",  Protocol::Type::UDP },
  };

  auto iter = map.find(key);
  if (iter != map.end())
    return iter->second;
  return Protocol::Type::UNKNOWN;
}

} // anonymous namespace

Message Probe(const char* s, size_t n) {
  Message msg;
  Fhmf fhmf;
  if (!ParseFhmf(s, n, &fhmf)) {
    msg.type = Protocol::Type::HTTP;
    ParseHttp(s, n, true, &msg.slices);
    return msg;
  }

  std::vector<Fhmf::Field>& fields = fhmf.fields;
  for (size_t i = 0; i < fields.size(); ++i) {
    if (fields[i].payload_len == 0)
      continue;

    const std::string& mime_type =
        fields[i].options[FieldType::MIME_TYPE];
    if (mime_type.empty())
      continue;
    msg.type = MessageTypeMapped(mime_type);
    switch (msg.type) {
    case Protocol::Type::HTTP:
      ProbeHttp(fields[i], &msg.slices);
      break;
    case Protocol::Type::TCP:
    case Protocol::Type::UDP:
      ProbeBinary(fields[i], &msg.slices);
      break;
    default: break;
    }
  }
  return msg;
}

Message MakeHttpMessage(const char* up, size_t up_size,
                        const char* down, size_t down_size) {
  Message msg;
  msg.type = Protocol::Type::HTTP;
  if (!up || !up_size)
    return msg;
  ParseHttp(up, up_size, true, &msg.slices);
  if (down && down_size)
    ParseHttp(down, down_size, false, &msg.slices);
  return msg;
}

Message::Slice& Slice(Message* msg, Message::Slice::Type type) {
  assert(msg->type != Protocol::Type::UNKNOWN);

  switch (msg->type) {
  case Protocol::Type::HTTP:
    assert(type >= SliceType::HTTP_HOST &&
           type <= SliceType::HTTP_RES);
    break;
  case Protocol::Type::TCP:
  case Protocol::Type::UDP:
    assert(type >= SliceType::BIN_DOMAIN &&
           type <= SliceType::BIN_RES);
    break;
  default: UNREACHABLE_CODE;
  }
  return msg->slices[type];
}

} // namespace ext
