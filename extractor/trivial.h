// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#ifndef EXTRACTOR_TRIVIAL_H_
#define EXTRACTOR_TRIVIAL_H_

#include <pthread.h>

#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <mutex>

namespace ext {
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(type)  \
    type(const type&) = delete;         \
    type& operator=(const type&) = delete
#endif

#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#define ARRAY_SIZE(a) \
  (sizeof(ext::trivial_internal::ArraySizeHelper(a)))
#endif

#define GXX_PREREQ(major, minor)  \
  ((__GNUC__ << 16) + __GNUC_MINOR__ >= (major << 16) + minor)

#define UNREACHABLE_CODE do {                 \
  assert(false && "Unreachable code");        \
  throw std::logic_error("Unreachable code"); \
} while (0)

namespace trivial_internal {
template<typename Tp, size_t N, typename Ret = char (&)[N]>
Ret ArraySizeHelper(const Tp (&)[N]);
} // namespace trivial_internal

// error code, error name, error string
#define ERROR_MAP(_X)                                     \
  _X(SUCCESS,             Success)                        \
  _X(UNKNOWN_ERROR,       Unknown error)                  \
  _X(UNDEFINE_PROTO,      Undefine protocol)              \
  _X(UNDEFINE_STEP,       Undefine step)                  \
  _X(INVALID_STEP,        Invalid step)                   \
  _X(INVALID_LAYOUT,      Invalid rule layout)            \
  _X(INVALID_RULE,        Invalid rule)                   \
  _X(NEGATIVE_RULE,       Negative rule)                  \
  _X(UNDEFINE_METHOD,     Undefine codec method)          \
  _X(UNCOMPRESS_FAILED,   Uncompress failed)              \
  _X(DECODE_FAILED,       Decode failed)                  \
  _X(NOT_FOUND_RULE,      Not found rule)                 \
  _X(INCOMPLETE_MSG,      Incomplete message)             \
  _X(NOT_IMPLEMENTED,     Feature not implemented)        \
  _X(UNKNOWN_MESSAGE,     Unknown message)

enum {
#define ERROR_CODE(n, _) n,
  ERROR_MAP(ERROR_CODE)
#undef ERROR_CODE
  ERROR_CODE_LAST
};

const char* string_error(int code);

void split(const char* s, size_t size, const char* sep,
           std::vector<std::string>* res, bool ignore_empty);

bool wildcard_match(const char* s, size_t s_len,
                    const char* p, size_t p_len);

inline bool wildcard_match(const std::string& s,
                    const std::string& p) {
  return wildcard_match(s.data(), s.size(), p.data(), p.size());
}

template<typename Map>
typename Map::mapped_type const& SafeFind(Map const& m,
    typename Map::key_type const& key) {
  static typename Map::mapped_type const def{};
  auto iter = m.find(key);
  if (iter == m.end())
    return def;
  return iter->second;
}

template<typename Map>
typename Map::mapped_type const& SafeFindOrDie(Map const& m,
    typename Map::key_type const& key) {
  auto iter = m.find(key);
  if (iter == m.end())
    throw std::logic_error("SafeFindOrDie failed: Not found " + key);
  return iter->second;
}

class rdlock_guard {
public:
  explicit rdlock_guard(pthread_rwlock_t* lock): lock_(lock) {
    int ret = pthread_rwlock_rdlock(lock_);
    if (ret != 0)
      throw std::runtime_error("pthread_rwlock_rdlock");
  }

  ~rdlock_guard(){
    int ret = pthread_rwlock_unlock(lock_);
    if (ret != 0)
      throw std::runtime_error("pthread_rwlock_unlock");
  }

private:
  pthread_rwlock_t* lock_;
  DISALLOW_COPY_AND_ASSIGN(rdlock_guard);
};

class wrlock_guard {
public:
  explicit wrlock_guard(pthread_rwlock_t* lock): lock_(lock) {
    int ret = pthread_rwlock_wrlock(lock_);
    if (ret != 0)
      throw std::runtime_error("pthread_rwlock_wrlock");
  }

  ~wrlock_guard(){
    int ret = pthread_rwlock_unlock(lock_);
    if (ret != 0)
      throw std::runtime_error("pthread_rwlock_unlock");
  }

private:
  pthread_rwlock_t* lock_;
  DISALLOW_COPY_AND_ASSIGN(wrlock_guard);
};

template<typename Tp>
class Singleton {
public:
  typedef Tp* pointer_type;
  typedef Tp& reference_type;

  static  pointer_type Pointer();
  static inline reference_type Reference() {
    return *Pointer();
  }
  static void Teardown() {
    delete ptr_;
    ptr_ = NULL;
  }

private:
  static pointer_type ptr_;
  static std::mutex lock_;
};

template<typename Tp>
typename Singleton<Tp>::pointer_type Singleton<Tp>::ptr_ = NULL;

template<typename Tp>
std::mutex Singleton<Tp>::lock_;

template<typename Tp>
typename Singleton<Tp>::pointer_type Singleton<Tp>::Pointer() {
  pointer_type ptr = ptr_;
  if (!ptr) {
    std::lock_guard<std::mutex> guard(lock_);
    ptr = ptr_;
    if (!ptr) {
      ptr = new Tp{};
      ptr_ = ptr;
    }
  }
  return ptr_;
}

} // namespace ext

#endif // EXTRACTOR_TRIVIAL_H_
