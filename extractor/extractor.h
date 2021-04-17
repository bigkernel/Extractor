// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#ifndef EXTRACTOR_EXTRACTOR_H_
#define EXTRACTOR_EXTRACTOR_H_

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace ext {
typedef std::map<std::string, std::string> Record;
typedef std::vector<Record> RecordSet;

class Extractor {
public:
  Extractor();
  Extractor(const char* buf, size_t size);
  ~Extractor();

  void LoadRule(const char* buf, size_t size);

  void Stats(std::string* buf) const;

  // attrib - attributes of the buffer can occurred
  //          PROTOCOL_ACTION (100-1, 1000001, 99-1),
  //          ACTION,
  //          HOST,
  //          HOST_ID,
  //          URL(HTTP protocol only),
  //          URL_ID,
  //          APP_NAME,
  //          SEVER_IP(TCP/UDP protocol only),
  //          SERVER_PORT(TCP/UDP protocol only),
  //          PROTOCOL
  int Extract(const char* buf,
              size_t size,
              RecordSet* res,
              Record* attrib) const;

  int Extract(const char* up, size_t up_size,
              const char* down, size_t down_size,
              RecordSet* res, Record* attrib) const;

  static const char* StringError(int code);

private:
  class Impl;
  std::shared_ptr<Impl> impl_;
};

} // namespace ext

#endif // EXTRACTOR_EXTRACTOR_H_
