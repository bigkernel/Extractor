// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/extractor.h"

#include <cassert>
#include "extractor/rule.h"
#include "extractor/codec.h"
#include "extractor/parser.h"
#include "extractor/message.h"
#include "extractor/trivial.h"

namespace ext {
class Extractor::Impl {
public:
  Impl();
  Impl(const char* buf, size_t size);
  ~Impl() {}

  inline void LoadRule(const char* buf, size_t size);

  void Stats(std::string* buf) const;

  int Extract(const char* buf, size_t size,
              RecordSet* res, Record* attrib) const;

  int Extract(const char* up, size_t up_size,
                const char* down, size_t down_size,
                RecordSet* res, Record* attrib) const;

private:
  mutable pthread_rwlock_t rw_lock_;
  RuleTree rt_;
  DISALLOW_COPY_AND_ASSIGN(Impl);
};

Extractor::Impl::Impl()
    : rw_lock_(PTHREAD_RWLOCK_INITIALIZER), rt_() {}

Extractor::Impl::Impl(const char* buf, size_t size)
    : rw_lock_(PTHREAD_RWLOCK_INITIALIZER), rt_() {
  rt_ = MakeRuleTree(buf, size);
}

void Extractor::Impl::LoadRule(const char* buf, size_t size) {
  wrlock_guard guard(&rw_lock_);
  rt_ = MakeRuleTree(buf, size);
}

void Extractor::Impl::Stats(std::string* buf) const {
  *buf = "{\"key\": \"test\"}";
}

int Extractor::Impl::Extract(const char* buf,
                             size_t size,
                             RecordSet* res,
                             Record* attrib) const {
  rdlock_guard guard(&rw_lock_);

  Message msg = Probe(buf, size);
  if (msg.type == Protocol::Type::UNKNOWN)
    return UNKNOWN_MESSAGE;
  return ParserFactory(&rt_, &msg, res, attrib);
}

int Extractor::Impl::Extract(const char* up, size_t up_size,
                             const char* down, size_t down_size,
                             RecordSet* res, Record* attrib) const {
  rdlock_guard guard(&rw_lock_);
  Message msg = MakeHttpMessage(up, up_size, down, down_size);
  assert(msg.type == Protocol::Type::HTTP);
  return ParserFactory(&rt_, &msg, res, attrib);
}

// Extractor interface
Extractor::Extractor(): impl_(new Impl) {}

Extractor::Extractor(const char* buf, size_t size)
    : impl_(new Impl(buf, size)) {}

Extractor::~Extractor() {}

void Extractor::LoadRule(const char* buf, size_t size) {
  impl_->LoadRule(buf, size);
}

void Extractor::Stats(std::string* buf) const {
  return impl_->Stats(buf);
}

int Extractor::Extract(const char* buf,
                       size_t size,
                       RecordSet* res,
                       Record* attrib) const {
  return impl_->Extract(buf, size, res, attrib);
}

int Extractor::Extract(const char* up, size_t up_size,
                       const char* down, size_t down_size,
                       RecordSet* res, Record* attrib) const {
  return impl_->Extract(up, up_size, down, down_size, res, attrib);
}

const char* Extractor::StringError(int code) {
  return string_error(code);
}

} // namespace ext

