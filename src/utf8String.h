#ifndef LEDIT_UTF8_STRING
#define LEDIT_UTF8_STRING
#ifdef __linux__
#include <cstdint>
#else
#include <cstddef>
#endif
#include <string>
#include <vector>
class Utf8String {
public:
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = char32_t;
    using difference_type = char32_t;
    using pointer = char32_t *;
    using reference = char32_t &;
    explicit iterator() { str = nullptr; }
    explicit iterator(Utf8String *_str) : str(_str) {
      end = _str->length() == 0;
    }
    explicit iterator(Utf8String *_str, bool _end, size_t _index)
        : str(_str), end(_end), index(_index) {}
    iterator &operator++() {
      tryIncrement();
      return *this;
    }
    iterator operator++(int) {
      iterator retval = *this;
      ++(*this);
      return retval;
    }
    iterator &operator--() {
      tryDecrement();
      return *this;
    }
    friend iterator operator-(iterator it, size_t diff) {
      iterator n(it.str, it.end, it.index);
      for (size_t i = 0; i < diff; i++)
        n.tryDecrement();
      return n;
    }

    iterator operator--(int) {
      iterator retval = *this;
      --(*this);
      return retval;
    }
    bool operator==(iterator other) const { return equal(other); }
    bool operator!=(iterator other) const { return !equal(other); }
    value_type operator*() const { return str->getCharacterAt(index); }

  private:
    Utf8String *str;
    size_t index = 0;
    bool end = false;
    bool equal(iterator other) const {
      if (other.str != str)
        return false;
      if (end) {
        if (other.end)
          return true;
        return false;
      } else if (other.end) {
        return false;
      }
      return index == other.index;
    }
    void tryIncrement() {
      if (end || !str)
        return;
      if (++index == str->length()) {
        end = true;
      }
    }
    void tryDecrement() {
      if (!str)
        return;
      if (str->length() == 0)
        return;
      if (index == 0)
        return;
      end = false;
      index--;
    }
  };
  using const_iterator = iterator;

  Utf8String();
  Utf8String(std::string base);
  Utf8String(Utf8String &other);
  Utf8String(const Utf8String &other);
  Utf8String(const size_t len, const char32_t *ptr);
  Utf8String(const size_t len, const char32_t in);
  Utf8String(const char32_t input[]);

  Utf8String &operator+=(const char32_t input[]);
  Utf8String &operator=(const char32_t input[]);

  friend Utf8String operator+(const Utf8String &lhs, const char32_t rhs[]);
  friend Utf8String operator+(const Utf8String &lhs, const char32_t rhs);
  friend Utf8String operator+(const char32_t lhs[], const Utf8String &rhs);
  friend Utf8String operator+(const Utf8String lhs, const Utf8String &rhs);

  Utf8String &operator+=(const Utf8String &other);
  Utf8String &operator+=(const char32_t &other);

  bool operator==(const Utf8String &other) const;
  bool operator!=(const Utf8String &other) const;
  bool operator==(const char32_t input[]);
  bool operator!=(const char32_t input[]);

  size_t find(char32_t search, size_t start) const;
  size_t find(Utf8String search, size_t start);
  size_t find(char32_t search) const;
  size_t find(const Utf8String &other) const;

  iterator begin();

  iterator end();

  char32_t operator[](int i) const;

  size_t length() const;
  size_t size() const;
  std::string getStr() const;
  const std::string &getStrRef() const;
  std::vector<char32_t> getCodePoints() const;
  std::vector<char32_t> getCodePointsRange(size_t off = 0,
                                           size_t len = 0) const;

  char32_t getCharacterAt(size_t index) const;
  Utf8String substr(size_t start = 0) const;
  Utf8String substr(size_t start, size_t len) const;
  void erase(size_t start = 0, size_t len = 0);

  void insert(size_t index, const Utf8String &other);
  void append(const Utf8String &other);
  void append(char32_t cp);
  void append(std::vector<char32_t> &cps);
  void appendAt(const Utf8String &other, size_t start);
  void appendAt(std::vector<char32_t> &cps, size_t start);
  void appendAt(char32_t cp, size_t start);
 
  bool endsWith(const Utf8String &other) const;
  void set(size_t idx, char32_t cc);
  uint32_t getIdx();
  void setIdx(uint32_t v);
private:
  std::string unicodeToUtf8(std::vector<char32_t> &in);
  std::string unicodeToUtf8(std::u32string &in);
  std::pair<size_t, size_t>
  calculateByteLength(size_t character_start = 0) const;

  std::pair<size_t, size_t> calculateByteLength(size_t character_start,
                                                size_t length) const;
  size_t calculateCharacterLength(std::string &u);

  void setState();
  std::vector<char32_t> toCodePoints(const std::string &u, size_t off = 0,
                                     size_t len = 0) const;
  std::string base;
  size_t character_length = 0;
  uint32_t idx = 0;
};

#endif