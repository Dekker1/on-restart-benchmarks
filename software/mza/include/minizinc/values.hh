/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
 *  Main authors:
 *     Guido Tack <guido.tack@monash.edu>
 */

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __MINIZINC_VALUES_HH__
#define __MINIZINC_VALUES_HH__

#include <minizinc/exception.hh>
#include <minizinc/gc.hh>

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <string>
#include <vector>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace MiniZinc {
class IntVal;
}
namespace std {
MiniZinc::IntVal abs(const MiniZinc::IntVal& x);
}

namespace MiniZinc {

class FloatVal;

class IntVal {
  friend IntVal operator+(const IntVal& x, const IntVal& y);
  friend IntVal operator-(const IntVal& x, const IntVal& y);
  friend IntVal operator*(const IntVal& x, const IntVal& y);
  friend IntVal operator/(const IntVal& x, const IntVal& y);
  friend IntVal operator%(const IntVal& x, const IntVal& y);
  friend IntVal std::abs(const MiniZinc::IntVal& x);
  friend bool operator==(const IntVal& x, const IntVal& y);
  friend class FloatVal;

private:
  long long int _v;
  bool _infinity;
  IntVal(long long int v, bool infinity) : _v(v), _infinity(infinity) {}

  static long long int safePlus(long long int x, long long int y) {
    if (x < 0) {
      if (y < std::numeric_limits<long long int>::min() - x)
        throw ArithmeticError("integer overflow");
    } else {
      if (y > std::numeric_limits<long long int>::max() - x)
        throw ArithmeticError("integer overflow");
    }
    return x + y;
  }
  static long long int safeMinus(long long int x, long long int y) {
    if (x < 0) {
      if (y > x - std::numeric_limits<long long int>::min())
        throw ArithmeticError("integer overflow");
    } else {
      if (y < x - std::numeric_limits<long long int>::max())
        throw ArithmeticError("integer overflow");
    }
    return x - y;
  }
  static long long int safeMult(long long int x, long long int y) {
    if (y == 0) return 0;
    long long unsigned int x_abs = (x < 0 ? 0 - x : x);
    long long unsigned int y_abs = (y < 0 ? 0 - y : y);
    if (x_abs > std::numeric_limits<long long int>::max() / y_abs)
      throw ArithmeticError("integer overflow");
    return x * y;
  }
  static long long int safeDiv(long long int x, long long int y) {
    if (y == 0) throw ArithmeticError("integer division by zero");
    if (x == 0) return 0;
    if (x == std::numeric_limits<long long int>::min() && y == -1)
      throw ArithmeticError("integer overflow");
    return x / y;
  }
  static long long int safeMod(long long int x, long long int y) {
    if (y == 0) throw ArithmeticError("integer division by zero");
    if (y == -1) return 0;
    return x % y;
  }

public:
  IntVal(void) : _v(0), _infinity(false) {}
  IntVal(long long int v) : _v(v), _infinity(false) {}
  IntVal(const FloatVal& v);

  long long int toInt(void) const {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    return _v;
  }

  long long int toIntUnsafe(void) const;

  bool isFinite(void) const { return !_infinity; }
  bool isPlusInfinity(void) const { return _infinity && _v == 1; }
  bool isMinusInfinity(void) const { return _infinity && _v == -1; }

  IntVal& operator+=(const IntVal& x) {
    if (!(isFinite() && x.isFinite()))
      throw ArithmeticError("arithmetic operation on infinite value");
    _v = safePlus(_v, x._v);
    return *this;
  }
  IntVal& operator-=(const IntVal& x) {
    if (!(isFinite() && x.isFinite()))
      throw ArithmeticError("arithmetic operation on infinite value");
    _v = safeMinus(_v, x._v);
    return *this;
  }
  IntVal& operator*=(const IntVal& x) {
    if (!(isFinite() && x.isFinite()))
      throw ArithmeticError("arithmetic operation on infinite value");
    _v = safeMult(_v, x._v);
    return *this;
  }
  IntVal& operator/=(const IntVal& x) {
    if (!(isFinite() && x.isFinite()))
      throw ArithmeticError("arithmetic operation on infinite value");
    _v = safeDiv(_v, x._v);
    return *this;
  }
  IntVal operator-() const {
    IntVal r = *this;
    r._v = safeMinus(0, _v);
    return r;
  }
  IntVal& operator++() {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    _v = safePlus(_v, 1);
    return *this;
  }
  IntVal operator++(int) {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    IntVal ret = *this;
    _v = safePlus(_v, 1);
    return ret;
  }
  IntVal& operator--() {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    _v = safeMinus(_v, 1);
    return *this;
  }
  IntVal operator--(int) {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    IntVal ret = *this;
    _v = safeMinus(_v, 1);
    return ret;
  }
  IntVal pow(const IntVal& exponent) {
    if (!exponent.isFinite() || !isFinite())
      throw ArithmeticError("arithmetic operation on infinite value");
    if (exponent == 0) return 1;
    if (exponent == 1) return *this;
    IntVal result = 1;
    for (int i = 0; i < exponent.toInt(); i++) {
      result *= *this;
    }
    return result;
  }

  static const IntVal minint(void);
  static const IntVal maxint(void);
  static const IntVal infinity(void);

  /// Infinity-safe addition
  IntVal plus(int x) const {
    if (isFinite())
      return safePlus(_v, x);
    else
      return *this;
  }
  /// Infinity-safe subtraction
  IntVal minus(int x) const {
    if (isFinite())
      return safeMinus(_v, x);
    else
      return *this;
  }

  size_t hash(void) const {
    std::hash<long long int> longhash;
    return longhash(_v);
  }
};

inline long long int IntVal::toIntUnsafe(void) const { return _v; }

inline bool operator==(const IntVal& x, const IntVal& y) {
  return x._infinity == y._infinity && x._v == y._v;
}
inline bool operator<=(const IntVal& x, const IntVal& y) {
  return y.isPlusInfinity() || x.isMinusInfinity() ||
         (x.isFinite() && y.isFinite() && x.toInt() <= y.toInt());
}
inline bool operator<(const IntVal& x, const IntVal& y) {
  return (y.isPlusInfinity() && !x.isPlusInfinity()) ||
         (x.isMinusInfinity() && !y.isMinusInfinity()) ||
         (x.isFinite() && y.isFinite() && x.toInt() < y.toInt());
}
inline bool operator>=(const IntVal& x, const IntVal& y) { return y <= x; }
inline bool operator>(const IntVal& x, const IntVal& y) { return y < x; }
inline bool operator!=(const IntVal& x, const IntVal& y) { return !(x == y); }
inline IntVal operator+(const IntVal& x, const IntVal& y) {
  if (!(x.isFinite() && y.isFinite()))
    throw ArithmeticError("arithmetic operation on infinite value");
  return IntVal::safePlus(x._v, y._v);
}
inline IntVal operator-(const IntVal& x, const IntVal& y) {
  if (!(x.isFinite() && y.isFinite()))
    throw ArithmeticError("arithmetic operation on infinite value");
  return IntVal::safeMinus(x._v, y._v);
}
inline IntVal operator*(const IntVal& x, const IntVal& y) {
  if (!x.isFinite()) {
    if (y.isFinite() && (y._v == 1 || y._v == -1))
      return IntVal(IntVal::safeMult(x._v, y._v), !x.isFinite());
  } else if (!y.isFinite()) {
    if (x.isFinite() && (y._v == 1 || y._v == -1))
      return IntVal(IntVal::safeMult(x._v, y._v), true);
  } else {
    return IntVal::safeMult(x._v, y._v);
  }
  throw ArithmeticError("arithmetic operation on infinite value");
}
inline IntVal operator/(const IntVal& x, const IntVal& y) {
  if (y.isFinite() && (y._v == 1 || y._v == -1))
    return IntVal(IntVal::safeMult(x._v, y._v), !x.isFinite());
  if (!(x.isFinite() && y.isFinite()))
    throw ArithmeticError("arithmetic operation on infinite value");
  return IntVal::safeDiv(x._v, y._v);
}
inline IntVal operator%(const IntVal& x, const IntVal& y) {
  if (!(x.isFinite() && y.isFinite()))
    throw ArithmeticError("arithmetic operation on infinite value");
  return IntVal::safeMod(x._v, y._v);
}
template <class Char, class Traits>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const IntVal& s) {
  if (s.isMinusInfinity())
    return os << "-infinity";
  else if (s.isPlusInfinity())
    return os << "infinity";
  else
    return os << s.toInt();
}

}  // namespace MiniZinc

namespace std {
inline MiniZinc::IntVal abs(const MiniZinc::IntVal& x) {
  if (!x.isFinite()) return MiniZinc::IntVal::infinity();
  return x < 0 ? MiniZinc::IntVal::safeMinus(0, x._v) : x;
}

inline MiniZinc::IntVal min(const MiniZinc::IntVal& x, const MiniZinc::IntVal& y) {
  return x <= y ? x : y;
}
inline MiniZinc::IntVal max(const MiniZinc::IntVal& x, const MiniZinc::IntVal& y) {
  return x >= y ? x : y;
}

template <>
struct equal_to<MiniZinc::IntVal> {
public:
  bool operator()(const MiniZinc::IntVal& s0, const MiniZinc::IntVal& s1) const { return s0 == s1; }
};

inline MiniZinc::FloatVal abs(const MiniZinc::FloatVal&);
}  // namespace std

namespace std {
template <>
struct hash<MiniZinc::IntVal> {
public:
  size_t operator()(const MiniZinc::IntVal& s) const { return s.hash(); }
};
}  // namespace std

namespace MiniZinc {

class FloatVal {
  friend FloatVal operator+(const FloatVal& x, const FloatVal& y);
  friend FloatVal operator-(const FloatVal& x, const FloatVal& y);
  friend FloatVal operator*(const FloatVal& x, const FloatVal& y);
  friend FloatVal operator/(const FloatVal& x, const FloatVal& y);
  friend FloatVal std::abs(const MiniZinc::FloatVal& x);
  friend bool operator==(const FloatVal& x, const FloatVal& y);
  friend class IntVal;

private:
  double _v;
  bool _infinity;
  void checkOverflow(void) {
    if (!std::isfinite(_v)) throw ArithmeticError("overflow in floating point operation");
  }
  FloatVal(double v, bool infinity) : _v(v), _infinity(infinity) { checkOverflow(); }

public:
  FloatVal(void) : _v(0.0), _infinity(false) {}
  FloatVal(double v) : _v(v), _infinity(false) { checkOverflow(); }
  FloatVal(const IntVal& v) : _v(static_cast<double>(v._v)), _infinity(!v.isFinite()) {}

  double toDouble(void) const {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    return _v;
  }

  bool isFinite(void) const { return !_infinity; }
  bool isPlusInfinity(void) const { return _infinity && _v == 1.0; }
  bool isMinusInfinity(void) const { return _infinity && _v == -1.0; }

  FloatVal& operator+=(const FloatVal& x) {
    if (!(isFinite() && x.isFinite()))
      throw ArithmeticError("arithmetic operation on infinite value");
    _v += x._v;
    checkOverflow();
    return *this;
  }
  FloatVal& operator-=(const FloatVal& x) {
    if (!(isFinite() && x.isFinite()))
      throw ArithmeticError("arithmetic operation on infinite value");
    _v -= x._v;
    checkOverflow();
    return *this;
  }
  FloatVal& operator*=(const FloatVal& x) {
    if (!(isFinite() && x.isFinite()))
      throw ArithmeticError("arithmetic operation on infinite value");
    _v *= x._v;
    checkOverflow();
    return *this;
  }
  FloatVal& operator/=(const FloatVal& x) {
    if (!(isFinite() && x.isFinite()))
      throw ArithmeticError("arithmetic operation on infinite value");
    _v = _v / x._v;
    checkOverflow();
    return *this;
  }
  FloatVal operator-() const {
    FloatVal r = *this;
    r._v = -r._v;
    return r;
  }
  FloatVal& operator++() {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    _v = _v + 1;
    checkOverflow();
    return *this;
  }
  FloatVal operator++(int) {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    FloatVal ret = *this;
    _v = _v + 1;
    checkOverflow();
    return ret;
  }
  FloatVal& operator--() {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    _v = _v - 1;
    checkOverflow();
    return *this;
  }
  FloatVal operator--(int) {
    if (!isFinite()) throw ArithmeticError("arithmetic operation on infinite value");
    FloatVal ret = *this;
    _v = _v - 1;
    checkOverflow();
    return ret;
  }

  static const FloatVal infinity(void);

  /// Infinity-safe addition
  FloatVal plus(int x) {
    if (isFinite())
      return (*this) + x;
    else
      return *this;
  }
  /// Infinity-safe subtraction
  FloatVal minus(int x) {
    if (isFinite())
      return (*this) - x;
    else
      return *this;
  }

  size_t hash(void) const {
    std::hash<double> doublehash;
    return doublehash(_v);
  }
};

inline bool operator==(const FloatVal& x, const FloatVal& y) {
  return x._infinity == y._infinity && x._v == y._v;
}
inline bool operator<=(const FloatVal& x, const FloatVal& y) {
  return y.isPlusInfinity() || x.isMinusInfinity() ||
         (x.isFinite() && y.isFinite() && x.toDouble() <= y.toDouble());
}
inline bool operator<(const FloatVal& x, const FloatVal& y) {
  return (y.isPlusInfinity() && !x.isPlusInfinity()) ||
         (x.isMinusInfinity() && !y.isMinusInfinity()) ||
         (x.isFinite() && y.isFinite() && x.toDouble() < y.toDouble());
}
inline bool operator>=(const FloatVal& x, const FloatVal& y) { return y <= x; }
inline bool operator>(const FloatVal& x, const FloatVal& y) { return y < x; }
inline bool operator!=(const FloatVal& x, const FloatVal& y) { return !(x == y); }
inline FloatVal operator+(const FloatVal& x, const FloatVal& y) {
  if (!(x.isFinite() && y.isFinite()))
    throw ArithmeticError("arithmetic operation on infinite value");
  return x.toDouble() + y.toDouble();
}
inline FloatVal operator-(const FloatVal& x, const FloatVal& y) {
  if (!(x.isFinite() && y.isFinite()))
    throw ArithmeticError("arithmetic operation on infinite value");
  return x.toDouble() - y.toDouble();
}
inline FloatVal operator*(const FloatVal& x, const FloatVal& y) {
  if (!(x.isFinite() && y.isFinite()))
    throw ArithmeticError("arithmetic operation on infinite value");
  return x.toDouble() * y.toDouble();
}
inline FloatVal operator/(const FloatVal& x, const FloatVal& y) {
  if (!(x.isFinite() && y.isFinite()))
    throw ArithmeticError("arithmetic operation on infinite value");
  return x.toDouble() / y.toDouble();
}
template <class Char, class Traits>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const FloatVal& s) {
  if (s.isMinusInfinity())
    return os << "-infinity";
  else if (s.isPlusInfinity())
    return os << "infinity";
  else
    return os << s.toDouble();
}

inline IntVal::IntVal(const FloatVal& v)
    : _v(static_cast<long long int>(v._v)), _infinity(!v.isFinite()) {}

}  // namespace MiniZinc

namespace std {
inline MiniZinc::FloatVal abs(const MiniZinc::FloatVal& x) {
  if (!x.isFinite()) return MiniZinc::FloatVal::infinity();
  return x.toDouble() < 0 ? MiniZinc::FloatVal(-x.toDouble()) : x;
}

inline MiniZinc::FloatVal min(const MiniZinc::FloatVal& x, const MiniZinc::FloatVal& y) {
  return x <= y ? x : y;
}
inline MiniZinc::FloatVal max(const MiniZinc::FloatVal& x, const MiniZinc::FloatVal& y) {
  return x >= y ? x : y;
}

inline MiniZinc::FloatVal floor(const MiniZinc::FloatVal& x) {
  if (!x.isFinite()) return x;
  return floor(x.toDouble());
}
inline MiniZinc::FloatVal ceil(const MiniZinc::FloatVal& x) {
  if (!x.isFinite()) return x;
  return ceil(x.toDouble());
}

template <>
struct equal_to<MiniZinc::FloatVal> {
public:
  bool operator()(const MiniZinc::FloatVal& s0, const MiniZinc::FloatVal& s1) const {
    return s0 == s1;
  }
};
}  // namespace std

namespace std {
template <>
struct hash<MiniZinc::FloatVal> {
public:
  size_t operator()(const MiniZinc::FloatVal& s) const { return s.hash(); }
};
}  // namespace std

namespace MiniZinc {

typedef unsigned long long int UIntVal;

/// An integer set value
class IntSetVal : public ASTChunk {
public:
  /// Contiguous range
  struct Range {
    /// Range minimum
    IntVal min;
    /// Range maximum
    IntVal max;
    /// Construct range from \a m to \a n
    Range(IntVal m, IntVal n) : min(m), max(n) {}
    /// Default constructor
    Range(void) {}
  };

private:
  /// Return range at position \a i
  Range& get(int i) { return reinterpret_cast<Range*>(_data)[i]; }
  /// Return range at position \a i
  const Range& get(int i) const { return reinterpret_cast<const Range*>(_data)[i]; }
  /// Construct empty set
  IntSetVal(void) : ASTChunk(0) {}
  /// Construct set of single range
  IntSetVal(IntVal m, IntVal n);
  /// Construct set from \a s
  IntSetVal(const std::vector<Range>& s) : ASTChunk(sizeof(Range) * s.size()) {
    for (unsigned int i = static_cast<unsigned int>(s.size()); i--;) get(i) = s[i];
  }

  /// Disabled
  IntSetVal(const IntSetVal& r);
  /// Disabled
  IntSetVal& operator=(const IntSetVal& r);

public:
  /// Return number of ranges
  int size(void) const { return static_cast<int>(_size / sizeof(Range)); }
  /// Return minimum, or infinity if set is empty
  IntVal min(void) const { return size() == 0 ? IntVal::infinity() : get(0).min; }
  /// Return maximum, or minus infinity if set is empty
  IntVal max(void) const { return size() == 0 ? -IntVal::infinity() : get(size() - 1).max; }
  /// Return minimum of range \a i
  IntVal min(int i) const {
    assert(i < size());
    return get(i).min;
  }
  /// Return maximum of range \a i
  IntVal max(int i) const {
    assert(i < size());
    return get(i).max;
  }
  /// Return width of range \a i
  IntVal width(int i) const {
    assert(i < size());
    if (min(i).isFinite() && max(i).isFinite())
      return max(i) - min(i) + 1;
    else
      return IntVal::infinity();
  }
  /// Return cardinality
  IntVal card(void) const {
    IntVal c = 0;
    for (unsigned int i = size(); i--;) {
      if (width(i).isFinite())
        c += width(i);
      else
        return IntVal::infinity();
    }
    return c;
  }

  /// Allocate empty set from context
  static IntSetVal* a(void) {
    IntSetVal* r = static_cast<IntSetVal*>(ASTChunk::alloc(0));
    new (r) IntSetVal();
    return r;
  }

  /// Allocate set \f$\{m,n\}\f$ from context
  static IntSetVal* a(IntVal m, IntVal n) {
    if (m > n) {
      return a();
    } else {
      IntSetVal* r = static_cast<IntSetVal*>(ASTChunk::alloc(sizeof(Range)));
      new (r) IntSetVal(m, n);
      return r;
    }
  }

  /// Allocate set using iterator \a i
  template <class I>
  static IntSetVal* ai(I& i) {
    std::vector<Range> s;
    for (; i(); ++i) s.push_back(Range(i.min(), i.max()));
    IntSetVal* r = static_cast<IntSetVal*>(ASTChunk::alloc(sizeof(Range) * s.size()));
    new (r) IntSetVal(s);
    return r;
  }

  /// Allocate set from vector \a s0 (may contain duplicates)
  static IntSetVal* a(const std::vector<IntVal>& s0) {
    if (s0.size() == 0) return a();
    std::vector<IntVal> s = s0;
    std::sort(s.begin(), s.end());
    std::vector<Range> ranges;
    IntVal min = s[0];
    IntVal max = min;
    for (unsigned int i = 1; i < s.size(); i++) {
      if (s[i] > max + 1) {
        ranges.push_back(Range(min, max));
        min = s[i];
        max = min;
      } else {
        max = s[i];
      }
    }
    ranges.push_back(Range(min, max));
    IntSetVal* r = static_cast<IntSetVal*>(ASTChunk::alloc(sizeof(Range) * ranges.size()));
    new (r) IntSetVal(ranges);
    return r;
  }
  static IntSetVal* a(const std::vector<Range>& ranges) {
    IntSetVal* r = static_cast<IntSetVal*>(ASTChunk::alloc(sizeof(Range) * ranges.size()));
    new (r) IntSetVal(ranges);
    return r;
  }

  /// Check if set contains \a v
  bool contains(const IntVal& v) {
    for (int i = 0; i < size(); i++) {
      if (v < min(i)) return false;
      if (v <= max(i)) return true;
    }
    return false;
  }

  /// Check if it is equal to \a s
  bool equal(const IntSetVal* s) {
    if (size() != s->size()) return false;
    for (int i = 0; i < size(); i++)
      if (min(i) != s->min(i) || max(i) != s->max(i)) return false;
    return true;
  }

  /// Mark for garbage collection
  void mark(void) { _gc_mark = 1; }
};

/// Iterator over an IntSetVal
class IntSetRanges {
  /// The set value
  const IntSetVal* rs;
  /// The current range
  int n;

public:
  /// Constructor
  IntSetRanges(const IntSetVal* r) : rs(r), n(0) {}
  /// Check if iterator is still valid
  bool operator()(void) const { return n < rs->size(); }
  /// Move to next range
  void operator++(void) { ++n; }
  /// Return minimum of current range
  IntVal min(void) const { return rs->min(n); }
  /// Return maximum of current range
  IntVal max(void) const { return rs->max(n); }
  /// Return width of current range
  IntVal width(void) const { return rs->width(n); }
};

template <class Char, class Traits>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const IntSetVal& s) {
  if (s.size() == 0) {
    os << "1..0";
  } else if (s.size() == 1) {
    // Print the range
    IntSetRanges isr(&s);
    os << isr.min() << ".." << isr.max();
  } else {
    // Print each element of the set
    bool first = true;
    os << "{";
    for (IntSetRanges isr(&s); isr(); ++isr) {
      if (!first) os << ", ";
      first = false;
      for (IntVal v = isr.min(); v < isr.max(); ++v) {
        os << v;
      }
    }
    os << "}";
  }
  return os;
}

/// An integer set value
class FloatSetVal : public ASTChunk {
public:
  /// Contiguous range
  struct Range {
    /// Range minimum
    FloatVal min;
    /// Range maximum
    FloatVal max;
    /// Construct range from \a m to \a n
    Range(FloatVal m, FloatVal n) : min(m), max(n) {}
    /// Default constructor
    Range(void) {}
  };

private:
  /// Return range at position \a i
  Range& get(int i) { return reinterpret_cast<Range*>(_data)[i]; }
  /// Return range at position \a i
  const Range& get(int i) const { return reinterpret_cast<const Range*>(_data)[i]; }
  /// Construct empty set
  FloatSetVal(void) : ASTChunk(0) {}
  /// Construct set of single range
  FloatSetVal(FloatVal m, FloatVal n);
  /// Construct set from \a s
  FloatSetVal(const std::vector<Range>& s) : ASTChunk(sizeof(Range) * s.size()) {
    for (unsigned int i = static_cast<unsigned int>(s.size()); i--;) get(i) = s[i];
  }

  /// Disabled
  FloatSetVal(const FloatSetVal& r);
  /// Disabled
  FloatSetVal& operator=(const FloatSetVal& r);

public:
  /// Return number of ranges
  int size(void) const { return static_cast<int>(_size / sizeof(Range)); }
  /// Return minimum, or infinity if set is empty
  FloatVal min(void) const { return size() == 0 ? FloatVal::infinity() : get(0).min; }
  /// Return maximum, or minus infinity if set is empty
  FloatVal max(void) const { return size() == 0 ? -FloatVal::infinity() : get(size() - 1).max; }
  /// Return minimum of range \a i
  FloatVal min(int i) const {
    assert(i < size());
    return get(i).min;
  }
  /// Return maximum of range \a i
  FloatVal max(int i) const {
    assert(i < size());
    return get(i).max;
  }
  /// Return width of range \a i
  FloatVal width(int i) const {
    assert(i < size());
    if (min(i).isFinite() && max(i).isFinite() && min(i) == max(i))
      return 1;
    else
      return IntVal::infinity();
  }
  /// Return cardinality
  FloatVal card(void) const {
    FloatVal c = 0;
    for (unsigned int i = size(); i--;) {
      if (width(i).isFinite())
        c += width(i);
      else
        return FloatVal::infinity();
    }
    return c;
  }

  /// Allocate empty set from context
  static FloatSetVal* a(void) {
    FloatSetVal* r = static_cast<FloatSetVal*>(ASTChunk::alloc(0));
    new (r) FloatSetVal();
    return r;
  }

  /// Allocate set \f$\{m,n\}\f$ from context
  static FloatSetVal* a(FloatVal m, FloatVal n) {
    if (m > n) {
      return a();
    } else {
      FloatSetVal* r = static_cast<FloatSetVal*>(ASTChunk::alloc(sizeof(Range)));
      new (r) FloatSetVal(m, n);
      return r;
    }
  }

  /// Allocate set using iterator \a i
  template <class I>
  static FloatSetVal* ai(I& i) {
    std::vector<Range> s;
    for (; i(); ++i) s.push_back(Range(i.min(), i.max()));
    FloatSetVal* r = static_cast<FloatSetVal*>(ASTChunk::alloc(sizeof(Range) * s.size()));
    new (r) FloatSetVal(s);
    return r;
  }

  /// Allocate set from vector \a s0 (may contain duplicates)
  static FloatSetVal* a(const std::vector<FloatVal>& s0) {
    if (s0.size() == 0) return a();
    std::vector<FloatVal> s = s0;
    std::sort(s.begin(), s.end());
    std::vector<Range> ranges;
    FloatVal min = s[0];
    FloatVal max = min;
    for (unsigned int i = 1; i < s.size(); i++) {
      if (s[i] > max) {
        ranges.push_back(Range(min, max));
        min = s[i];
        max = min;
      } else {
        max = s[i];
      }
    }
    ranges.push_back(Range(min, max));
    FloatSetVal* r = static_cast<FloatSetVal*>(ASTChunk::alloc(sizeof(Range) * ranges.size()));
    new (r) FloatSetVal(ranges);
    return r;
  }
  static FloatSetVal* a(const std::vector<Range>& ranges) {
    FloatSetVal* r = static_cast<FloatSetVal*>(ASTChunk::alloc(sizeof(Range) * ranges.size()));
    new (r) FloatSetVal(ranges);
    return r;
  }

  /// Check if set contains \a v
  bool contains(const FloatVal& v) {
    for (int i = 0; i < size(); i++) {
      if (v < min(i)) return false;
      if (v <= max(i)) return true;
    }
    return false;
  }

  /// Check if it is equal to \a s
  bool equal(const FloatSetVal* s) {
    if (size() != s->size()) return false;
    for (int i = 0; i < size(); i++)
      if (min(i) != s->min(i) || max(i) != s->max(i)) return false;
    return true;
  }

  /// Mark for garbage collection
  void mark(void) { _gc_mark = 1; }
};

/// Iterator over an IntSetVal
class FloatSetRanges {
  /// The set value
  const FloatSetVal* rs;
  /// The current range
  int n;

public:
  /// Constructor
  FloatSetRanges(const FloatSetVal* r) : rs(r), n(0) {}
  /// Check if iterator is still valid
  bool operator()(void) const { return n < rs->size(); }
  /// Move to next range
  void operator++(void) { ++n; }
  /// Return minimum of current range
  FloatVal min(void) const { return rs->min(n); }
  /// Return maximum of current range
  FloatVal max(void) const { return rs->max(n); }
  /// Return width of current range
  FloatVal width(void) const { return rs->width(n); }
};

template <class Char, class Traits>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os,
                                             const FloatSetVal& s) {
  for (FloatSetRanges isr(&s); isr(); ++isr) os << isr.min() << ".." << isr.max() << " ";
  return os;
}
}  // namespace MiniZinc

#endif
