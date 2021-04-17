// Author: yuyue/X3130 (yuyue2200@hotmail.com)
//
// This file forked from absl-cpp

//
// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// File: string_view.h
// -----------------------------------------------------------------------------
//
// This file contains the definition of the `absl::string_view` class. A
// `string_view` points to a contiguous span of characters, often part or all of
// another `std::string`, double-quoted string literal, character array, or even
// another `string_view`.
//
// This `absl::string_view` abstraction is designed to be a drop-in
// replacement for the C++17 `std::string_view` abstraction.

#ifdef __has_builtin
#define ABSL_HAVE_BUILTIN(x) __has_builtin(x)
#else
#define ABSL_HAVE_BUILTIN(x) 0
#endif

#if ABSL_HAVE_BUILTIN(__builtin_expect) || \
    (defined(__GNUC__) && !defined(__clang__))
#define ABSL_PREDICT_FALSE(x) (__builtin_expect(x, 0))
#define ABSL_PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#else
#define ABSL_PREDICT_FALSE(x) (x)
#define ABSL_PREDICT_TRUE(x) (x)
#endif

#if defined(NDEBUG)
#define ABSL_ASSERT(expr) (false ? (void)(expr) : (void)0)
#else
#define ABSL_ASSERT(expr)  \
  (ABSL_PREDICT_TRUE((expr)) ? (void)0 : (void)assert(false && #expr))
#endif

#if __cpulsplus < 201103L
#define NOEXCEPT
#define CONSTEXPR const
#define USE_CONSTEXPR
#define NULLPTR NULL
#else
#define NOEXCEPT noexcept
#define CONSTEXPR constexpr
#define USE_CONSTEXPR constexpr
#define NULLPTR nullptr
#endif

#ifndef ABSL_STRINGS_STRING_VIEW_H_
#define ABSL_STRINGS_STRING_VIEW_H_

#include <stdexcept>
#include <algorithm>
//#include "absl/base/config.h"

#ifdef ABSL_HAVE_STD_STRING_VIEW

#include <string_view>

namespace absl {
inline namespace lts_2018_12_18 {
using std::string_view;
}  // inline namespace lts_2018_12_18
}  // namespace absl

#else  // ABSL_HAVE_STD_STRING_VIEW

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <string>

//#include "absl/base/internal/throw_delegate.h"
//#include "absl/base/macros.h"
//#include "absl/base/port.h"

namespace absl {
//inline namespace lts_2018_12_18 {

// absl::string_view
//
// A `string_view` provides a lightweight view into the string data provided by
// a `std::string`, double-quoted string literal, character array, or even
// another `string_view`. A `string_view` does *not* own the string to which it
// points, and that data cannot be modified through the view.
//
// You can use `string_view` as a function or method parameter anywhere a
// parameter can receive a double-quoted string literal, `const char*`,
// `std::string`, or another `absl::string_view` argument with no need to copy
// the string data. Systematic use of `string_view` within function arguments
// reduces data copies and `strlen()` calls.
//
// Because of its small size, prefer passing `string_view` by value:
//
//   void MyFunction(absl::string_view arg);
//
// If circumstances require, you may also pass one by const reference:
//
//   void MyFunction(const absl::string_view& arg);  // not preferred
//
// Passing by value generates slightly smaller code for many architectures.
//
// In either case, the source data of the `string_view` must outlive the
// `string_view` itself.
//
// A `string_view` is also suitable for local variables if you know that the
// lifetime of the underlying object is longer than the lifetime of your
// `string_view` variable. However, beware of binding a `string_view` to a
// temporary value:
//
//   // BAD use of string_view: lifetime problem
//   absl::string_view sv = obj.ReturnAString();
//
//   // GOOD use of string_view: str outlives sv
//   std::string str = obj.ReturnAString();
//   absl::string_view sv = str;
//
// Due to lifetime issues, a `string_view` is sometimes a poor choice for a
// return value and usually a poor choice for a data member. If you do use a
// `string_view` this way, it is your responsibility to ensure that the object
// pointed to by the `string_view` outlives the `string_view`.
//
// A `string_view` may represent a whole string or just part of a string. For
// example, when splitting a string, `std::vector<absl::string_view>` is a
// natural data type for the output.
//
//
// When constructed from a source which is nul-terminated, the `string_view`
// itself will not include the nul-terminator unless a specific size (including
// the nul) is passed to the constructor. As a result, common idioms that work
// on nul-terminated strings do not work on `string_view` objects. If you write
// code that scans a `string_view`, you must check its length rather than test
// for nul, for example. Note, however, that nuls may still be embedded within
// a `string_view` explicitly.
//
// You may create a null `string_view` in two ways:
//
//   absl::string_view sv();
//   absl::string_view sv(nullptr, 0);
//
// For the above, `sv.data() == nullptr`, `sv.length() == 0`, and
// `sv.empty() == true`. Also, if you create a `string_view` with a non-null
// pointer then `sv.data() != nullptr`. Thus, you can use `string_view()` to
// signal an undefined value that is different from other `string_view` values
// in a similar fashion to how `const char* p1 = nullptr;` is different from
// `const char* p2 = "";`. However, in practice, it is not recommended to rely
// on this behavior.
//
// Be careful not to confuse a null `string_view` with an empty one. A null
// `string_view` is an empty `string_view`, but some empty `string_view`s are
// not null. Prefer checking for emptiness over checking for null.
//
// There are many ways to create an empty string_view:
//
//   const char* nullcp = nullptr;
//   // string_view.size() will return 0 in all cases.
//   absl::string_view();
//   absl::string_view(nullcp, 0);
//   absl::string_view("");
//   absl::string_view("", 0);
//   absl::string_view("abcdef", 0);
//   absl::string_view("abcdef" + 6, 0);
//
// All empty `string_view` objects whether null or not, are equal:
//
//   absl::string_view() == absl::string_view("", 0)
//   absl::string_view(nullptr, 0) == absl::string_view("abcdef"+6, 0)
class string_view {
public:
#if __cplusplus >= 201103L
  using traits_type = std::char_traits<char>;
  using value_type = char;
  using pointer = char*;
  using const_pointer = const char*;
  using reference = char&;
  using const_reference = const char&;
  using const_iterator = const char*;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
#else
  typedef std::char_traits<char> traits_type;
  typedef char value_type;
  typedef char* pointer;
  typedef const char* const_pointer;
  typedef char& reference;
  typedef const char& const_reference;
  typedef const char* const_iterator;
  typedef const_iterator iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef const_reverse_iterator reverse_iterator;
  typedef size_t size_type;
  typedef std::ptrdiff_t difference_type;
#endif

  static const size_type npos;

  // Null `string_view` constructor
  USE_CONSTEXPR string_view() NOEXCEPT : ptr_(NULLPTR), length_(0) {}

  // Implicit constructors

  template <typename Allocator>
  string_view(  // NOLINT(runtime/explicit)
      const std::basic_string<char, std::char_traits<char>, Allocator>&
          str) NOEXCEPT
      : ptr_(str.data()), length_(CheckLengthInternal(str.size())) {}

  // Implicit constructor of a `string_view` from nul-terminated `str`. When
  // accepting possibly null strings, use `absl::NullSafeStringView(str)`
  // instead (see below).
#if ABSL_HAVE_BUILTIN(__builtin_strlen) || \
    (defined(__GNUC__) && !defined(__clang__))
  // GCC has __builtin_strlen according to
  // https://gcc.gnu.org/onlinedocs/gcc-4.7.0/gcc/Other-Builtins.html, but
  // ABSL_HAVE_BUILTIN doesn't detect that, so we use the extra checks above.
  // __builtin_strlen is constexpr.
  USE_CONSTEXPR string_view(const char* str)  // NOLINT(runtime/explicit)
      : ptr_(str),
        length_(CheckLengthInternal(str ? __builtin_strlen(str) : 0)) {}
#else
  USE_CONSTEXPR string_view(const char* str)  // NOLINT(runtime/explicit)
      : ptr_(str), length_(CheckLengthInternal(str ? strlen(str) : 0)) {}
#endif

  // Implicit constructor of a `string_view` from a `const char*` and length.
  USE_CONSTEXPR string_view(const char* data, size_type len)
      : ptr_(data), length_(CheckLengthInternal(len)) {}

  USE_CONSTEXPR string_view(const char* begin, const  char* end)
      : ptr_(begin), length_(CheckLengthInternal(end - begin)) {}

  // NOTE: Harmlessly omitted to work around gdb bug.
  //   constexpr string_view(const string_view&) noexcept = default;
  //   string_view& operator=(const string_view&) noexcept = default;

  // Iterators

  // string_view::begin()
  //
  // Returns an iterator pointing to the first character at the beginning of the
  // `string_view`, or `end()` if the `string_view` is empty.
  USE_CONSTEXPR const_iterator begin() const NOEXCEPT { return ptr_; }

  // string_view::end()
  //
  // Returns an iterator pointing just beyond the last character at the end of
  // the `string_view`. This iterator acts as a placeholder; attempting to
  // access it results in undefined behavior.
  USE_CONSTEXPR const_iterator end() const NOEXCEPT { return ptr_ + length_; }

  // string_view::cbegin()
  //
  // Returns a const iterator pointing to the first character at the beginning
  // of the `string_view`, or `end()` if the `string_view` is empty.
  USE_CONSTEXPR const_iterator cbegin() const NOEXCEPT { return begin(); }

  // string_view::cend()
  //
  // Returns a const iterator pointing just beyond the last character at the end
  // of the `string_view`. This pointer acts as a placeholder; attempting to
  // access its element results in undefined behavior.
  USE_CONSTEXPR const_iterator cend() const NOEXCEPT { return end(); }

  // string_view::rbegin()
  //
  // Returns a reverse iterator pointing to the last character at the end of the
  // `string_view`, or `rend()` if the `string_view` is empty.
  const_reverse_iterator rbegin() const NOEXCEPT {
    return const_reverse_iterator(end());
  }

  // string_view::rend()
  //
  // Returns a reverse iterator pointing just before the first character at the
  // beginning of the `string_view`. This pointer acts as a placeholder;
  // attempting to access its element results in undefined behavior.
  const_reverse_iterator rend() const NOEXCEPT {
    return const_reverse_iterator(begin());
  }

  // string_view::crbegin()
  //
  // Returns a const reverse iterator pointing to the last character at the end
  // of the `string_view`, or `crend()` if the `string_view` is empty.
  const_reverse_iterator crbegin() const NOEXCEPT { return rbegin(); }

  // string_view::crend()
  //
  // Returns a const reverse iterator pointing just before the first character
  // at the beginning of the `string_view`. This pointer acts as a placeholder;
  // attempting to access its element results in undefined behavior.
  const_reverse_iterator crend() const NOEXCEPT { return rend(); }

  // Capacity Utilities

  // string_view::size()
  //
  // Returns the number of characters in the `string_view`.
  USE_CONSTEXPR size_type size() const NOEXCEPT {
    return length_;
  }

  // string_view::length()
  //
  // Returns the number of characters in the `string_view`. Alias for `size()`.
  USE_CONSTEXPR size_type length() const NOEXCEPT { return size(); }

  // string_view::max_size()
  //
  // Returns the maximum number of characters the `string_view` can hold.
  USE_CONSTEXPR size_type max_size() const NOEXCEPT { return kMaxSize; }

  // string_view::empty()
  //
  // Checks if the `string_view` is empty (refers to no characters).
  USE_CONSTEXPR bool empty() const NOEXCEPT { return length_ == 0; }

  // std::string:view::operator[]
  //
  // Returns the ith element of an `string_view` using the array operator.
  // Note that this operator does not perform any bounds checking.
  USE_CONSTEXPR const_reference operator[](size_type i) const { return ptr_[i]; }

  // string_view::front()
  //
  // Returns the first element of a `string_view`.
  USE_CONSTEXPR const_reference front() const { return ptr_[0]; }

  // string_view::back()
  //
  // Returns the last element of a `string_view`.
  USE_CONSTEXPR const_reference back() const { return ptr_[size() - 1]; }

  // string_view::data()
  //
  // Returns a pointer to the underlying character array (which is of course
  // stored elsewhere). Note that `string_view::data()` may contain embedded nul
  // characters, but the returned buffer may or may not be nul-terminated;
  // therefore, do not pass `data()` to a routine that expects a nul-terminated
  // std::string.
  USE_CONSTEXPR const_pointer data() const NOEXCEPT { return ptr_; }

  // Modifiers

  // string_view::remove_prefix()
  //
  // Removes the first `n` characters from the `string_view`. Note that the
  // underlying std::string is not changed, only the view.
  void remove_prefix(size_type n) {
    assert(n <= length_);
    ptr_ += n;
    length_ -= n;
  }

  // string_view::remove_suffix()
  //
  // Removes the last `n` characters from the `string_view`. Note that the
  // underlying std::string is not changed, only the view.
  void remove_suffix(size_type n) {
    assert(n <= length_);
    length_ -= n;
  }

  // string_view::swap()
  //
  // Swaps this `string_view` with another `string_view`.
  void swap(string_view& s) NOEXCEPT {
    auto t = *this;
    *this = s;
    s = t;
  }

  // Explicit conversion operators

  // Converts to `std::basic_string`.
  template <typename A>
  operator std::basic_string<char, traits_type, A>() const {
    if (!data()) return {};
    return std::basic_string<char, traits_type, A>(data(), size());
  }

  // string_view::copy()
  //
  // Copies the contents of the `string_view` at offset `pos` and length `n`
  // into `buf`.
  size_type copy(char* buf, size_type n, size_type pos = 0) const;

  // string_view::substr()
  //
  // Returns a "substring" of the `string_view` (at offset `pos` and length
  // `n`) as another string_view. This function throws `std::out_of_bounds` if
  // `pos > size`.
  string_view substr(size_type pos, size_type n = npos) const {
    if (ABSL_PREDICT_FALSE(pos > length_))
      throw std::out_of_range("absl::string_view::substr");
    n = std::min(n, length_ - pos);
    return string_view(ptr_ + pos, n);
  }

  // string_view::compare()
  //
  // Performs a lexicographical comparison between the `string_view` and
  // another `absl::string_view`, returning -1 if `this` is less than, 0 if
  // `this` is equal to, and 1 if `this` is greater than the passed std::string
  // view. Note that in the case of data equality, a further comparison is made
  // on the respective sizes of the two `string_view`s to determine which is
  // smaller, equal, or greater.
  int compare(string_view x) const NOEXCEPT {
    auto min_length = std::min(length_, x.length_);
    if (min_length > 0) {
      int r = memcmp(ptr_, x.ptr_, min_length);
      if (r < 0) return -1;
      if (r > 0) return 1;
    }
    if (length_ < x.length_) return -1;
    if (length_ > x.length_) return 1;
    return 0;
  }

  // Overload of `string_view::compare()` for comparing a substring of the
  // 'string_view` and another `absl::string_view`.
  int compare(size_type pos1, size_type count1, string_view v) const {
    return substr(pos1, count1).compare(v);
  }

  // Overload of `string_view::compare()` for comparing a substring of the
  // `string_view` and a substring of another `absl::string_view`.
  int compare(size_type pos1, size_type count1, string_view v, size_type pos2,
              size_type count2) const {
    return substr(pos1, count1).compare(v.substr(pos2, count2));
  }

  // Overload of `string_view::compare()` for comparing a `string_view` and a
  // a different  C-style std::string `s`.
  int compare(const char* s) const { return compare(string_view(s)); }

  // Overload of `string_view::compare()` for comparing a substring of the
  // `string_view` and a different std::string C-style std::string `s`.
  int compare(size_type pos1, size_type count1, const char* s) const {
    return substr(pos1, count1).compare(string_view(s));
  }

  // Overload of `string_view::compare()` for comparing a substring of the
  // `string_view` and a substring of a different C-style std::string `s`.
  int compare(size_type pos1, size_type count1, const char* s,
              size_type count2) const {
    return substr(pos1, count1).compare(string_view(s, count2));
  }

  // Find Utilities

  // string_view::find()
  //
  // Finds the first occurrence of the substring `s` within the `string_view`,
  // returning the position of the first character's match, or `npos` if no
  // match was found.
  size_type find(string_view s, size_type pos = 0) const NOEXCEPT;

  // Overload of `string_view::find()` for finding the given character `c`
  // within the `string_view`.
  size_type find(char c, size_type pos = 0) const NOEXCEPT;

  // string_view::rfind()
  //
  // Finds the last occurrence of a substring `s` within the `string_view`,
  // returning the position of the first character's match, or `npos` if no
  // match was found.
  size_type rfind(string_view s, size_type pos = npos) const
      NOEXCEPT;

  // Overload of `string_view::rfind()` for finding the last given character `c`
  // within the `string_view`.
  size_type rfind(char c, size_type pos = npos) const NOEXCEPT;

  // string_view::find_first_of()
  //
  // Finds the first occurrence of any of the characters in `s` within the
  // `string_view`, returning the start position of the match, or `npos` if no
  // match was found.
  size_type find_first_of(string_view s, size_type pos = 0) const
      NOEXCEPT;

  // Overload of `string_view::find_first_of()` for finding a character `c`
  // within the `string_view`.
  size_type find_first_of(char c, size_type pos = 0) const
      NOEXCEPT {
    return find(c, pos);
  }

  // string_view::find_last_of()
  //
  // Finds the last occurrence of any of the characters in `s` within the
  // `string_view`, returning the start position of the match, or `npos` if no
  // match was found.
  size_type find_last_of(string_view s, size_type pos = npos) const
      NOEXCEPT;

  // Overload of `string_view::find_last_of()` for finding a character `c`
  // within the `string_view`.
  size_type find_last_of(char c, size_type pos = npos) const
      NOEXCEPT {
    return rfind(c, pos);
  }

  // string_view::find_first_not_of()
  //
  // Finds the first occurrence of any of the characters not in `s` within the
  // `string_view`, returning the start position of the first non-match, or
  // `npos` if no non-match was found.
  size_type find_first_not_of(string_view s, size_type pos = 0) const NOEXCEPT;

  // Overload of `string_view::find_first_not_of()` for finding a character
  // that is not `c` within the `string_view`.
  size_type find_first_not_of(char c, size_type pos = 0) const NOEXCEPT;

  // string_view::find_last_not_of()
  //
  // Finds the last occurrence of any of the characters not in `s` within the
  // `string_view`, returning the start position of the last non-match, or
  // `npos` if no non-match was found.
  size_type find_last_not_of(string_view s,
                                          size_type pos = npos) const NOEXCEPT;

  // Overload of `string_view::find_last_not_of()` for finding a character
  // that is not `c` within the `string_view`.
  size_type find_last_not_of(char c, size_type pos = npos) const
      NOEXCEPT;

private:
  static const size_type kMaxSize;

  static USE_CONSTEXPR size_type CheckLengthInternal(size_type len) {
    return ABSL_ASSERT(len <= kMaxSize), len;
  }

  const char* ptr_;
  size_type length_;
};

// This large function is defined inline so that in a fairly common case where
// one of the arguments is a literal, the compiler can elide a lot of the
// following comparisons.
inline bool operator==(string_view x, string_view y) NOEXCEPT {
  auto len = x.size();
  if (len != y.size()) {
    return false;
  }
  return x.data() == y.data() || len <= 0 ||
         memcmp(x.data(), y.data(), len) == 0;
}

inline bool operator!=(string_view x, string_view y) NOEXCEPT {
  return !(x == y);
}

inline bool operator<(string_view x, string_view y) NOEXCEPT {
  auto min_size = std::min(x.size(), y.size());
  const int r = min_size == 0 ? 0 : memcmp(x.data(), y.data(), min_size);
  return (r < 0) || (r == 0 && x.size() < y.size());
}

inline bool operator>(string_view x, string_view y) NOEXCEPT { return y < x; }

inline bool operator<=(string_view x, string_view y) NOEXCEPT {
  return !(y < x);
}

inline bool operator>=(string_view x, string_view y) NOEXCEPT {
  return !(x < y);
}

// IO Insertion Operator
std::ostream& operator<<(std::ostream& o, string_view piece);

//}  // inline namespace lts_2018_12_18
}  // namespace absl

#endif  // ABSL_HAVE_STD_STRING_VIEW

namespace absl {
//inline namespace lts_2018_12_18 {

// ClippedSubstr()
//
// Like `s.substr(pos, n)`, but clips `pos` to an upper bound of `s.size()`.
// Provided because std::string_view::substr throws if `pos > size()`
inline string_view ClippedSubstr(string_view s, size_t pos,
                                 size_t n = string_view::npos) {
  pos = std::min(pos, static_cast<size_t>(s.size()));
  return s.substr(pos, n);
}

// NullSafeStringView()
//
// Creates an `absl::string_view` from a pointer `p` even if it's null-valued.
// This function should be used where an `absl::string_view` can be created from
// a possibly-null pointer.
inline string_view NullSafeStringView(const char* p) {
  return p ? string_view(p) : string_view();
}

//}  // inline namespace lts_2018_12_18
}  // namespace absl

namespace ext {
using absl::string_view;
} // namespace ext

#endif  // ABSL_STRINGS_STRING_VIEW_H_
