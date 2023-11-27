#ifndef LEDIT_UTF8_STRING
#define LEDIT_UTF8_STRING
#include <cstddef>
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
  Utf8String(std::string base) {
    this->base = base;
    setState();
  }

  Utf8String() {
    this->base = "";
    setState();
  }

  Utf8String(Utf8String &other) {
    this->base = other.base;
    setState();
  }
  Utf8String(const Utf8String &other) {
    this->base = other.base;
    setState();
  }

  Utf8String(const size_t len, const char32_t *ptr) {
    std::vector<char32_t> buff(ptr, ptr + len);
    base = unicodeToUtf8(buff);
    character_length = len;
  }

  Utf8String(const size_t len, const char32_t in) {
    std::vector<char32_t> buff;
    buff.push_back(in);
    base = unicodeToUtf8(buff);
    character_length = len;
  }

  Utf8String(const char32_t input[]) {
    std::u32string str(input);
    this->character_length = str.length();
    this->base = unicodeToUtf8(str);
  }

  Utf8String &operator+=(const char32_t input[]) {
    std::u32string str(input);
    this->character_length += str.length();
    this->base += unicodeToUtf8(str);
    return *this;
  }

  Utf8String &operator=(const char32_t input[]) {
    std::u32string str(input);
    this->character_length = str.length();
    this->base = unicodeToUtf8(str);
    return *this;
  }

  friend Utf8String operator+(const Utf8String &lhs, const char32_t rhs[]) {
    Utf8String n;
    n.append(lhs);
    n += rhs;
    return n;
  }

  friend Utf8String operator+(const Utf8String &lhs, const char32_t rhs) {
    Utf8String n;
    n.append(lhs);
    n.append(rhs);
    return n;
  }

  friend Utf8String operator+(const char32_t lhs[], const Utf8String &rhs) {
    Utf8String n;
    n += lhs;
    n.append(rhs);
    return n;
  }

  friend Utf8String operator+(const Utf8String lhs, const Utf8String &rhs) {
    Utf8String n;
    n.append(lhs);
    n.append(rhs);
    return n;
  }

  Utf8String &operator+=(const Utf8String &other) {
    append(other);
    return *this;
  }

  Utf8String &operator+=(const char32_t &other) {
    append(other);
    return *this;
  }

  bool operator==(const Utf8String &other) const {
    return base == other.getStr();
  }

  bool operator==(const char32_t input[]) {
    std::u32string str(input);
    std::string n = unicodeToUtf8(str);
    return n == base;
  }

  bool operator!=(const char32_t input[]) {
    std::u32string str(input);
    std::string n = unicodeToUtf8(str);
    return n != base;
  }

  size_t find(char32_t search, size_t start) {
    for (size_t i = start; i < this->character_length; i++) {
      if (getCharacterAt(i) == search)
        return i;
    }
    return std::string::npos;
  }
  size_t find(Utf8String search, size_t start) {
    auto cps = getCodePointsRange(start);
    auto str = unicodeToUtf8(cps);
    return str.find(search.getStr());
  }
  size_t find(char32_t search) const {
    for (size_t i = 0; i < this->character_length; i++) {
      if (getCharacterAt(i) == search)
        return i;
    }
    return std::string::npos;
  }

  size_t find(Utf8String &other) { return base.find(other.getStr()); }

  iterator begin() { return iterator(this); }

  iterator end() { return iterator(this, true, character_length); }

  char32_t operator[](int i) { return getCharacterAt(i); }

  size_t length() const { return this->character_length; }
  size_t size() const { return this->character_length; }
  std::string getStr() const { return this->base; }
  std::vector<char32_t> getCodePoints() {
    return this->toCodePoints(this->base);
  }
  std::vector<char32_t> getCodePointsRange(size_t off = 0, size_t len = 0) {
    return this->toCodePoints(this->base, off, len);
  }

  char32_t getCharacterAt(size_t index) const {
    auto p = calculateByteLength(index, 1);
    std::string sub = this->base.substr(p.first, p.second);
    std::vector<char32_t> entries = toCodePoints(sub);
    return entries[0];
  }
    Utf8String substr(size_t start = 0) const {
    auto p = calculateByteLength(start);
    return Utf8String(this->base.substr(p.first, p.second));
  }
  Utf8String substr(size_t start, size_t len) const {
    auto p = calculateByteLength(start, len);
    return Utf8String(this->base.substr(p.first, p.second));
  }
  void erase(size_t start = 0, size_t len = 0) {
    auto p = calculateByteLength(start, len);
    this->base.erase(p.first, p.second);
    setState();
  }

  void insert(size_t index, Utf8String &other) { appendAt(other, index); }
  void append(const Utf8String &other) {
    this->base += other.base;
    this->character_length += other.character_length;
  }
  void append(char32_t cp) {
    std::vector<char32_t> cps = {cp};
    this->append(cps);
  }
  void append(std::vector<char32_t> &cps) {
    std::string value = unicodeToUtf8(cps);
    character_length += cps.size();
    this->base += value;
  }
  void appendAt(Utf8String &other, size_t start) {
    auto p = this->calculateByteLength(start);
    this->base.insert(p.first, other.base);
    this->character_length += other.character_length;
  }
  void appendAt(std::vector<char32_t> &cps, size_t start) {
    auto p = this->calculateByteLength(start);
    std::string value = unicodeToUtf8(cps);
    character_length += cps.size();
    this->base.insert(p.first, value);
  }
  void appendAt(char32_t cp, size_t start) {
    std::vector<char32_t> cps = {cp};
    this->appendAt(cps, start);
  }

  bool endsWith(const Utf8String &other) const {
    if (other.length() > character_length)
      return false;
    const auto sub = substr(character_length - other.length());
    return sub == other;
  }

private:
  std::string unicodeToUtf8(std::vector<char32_t> &in) {
    std::string out = "";
    for (char32_t cp : in) {
      if (cp <= 0x7F) {
        out += (char)cp;
        continue;
      }
      if (cp <= 0x07FF) {
        out += (char)(((cp >> 6) & 0x1F) | 0xC0);
        out += (char)(((cp >> 0) & 0x3F) | 0x80);
        continue;
      }
      if (cp <= 0xFFFF) {
        out += (char)(((cp >> 12) & 0x0F) | 0xE0);
        out += (char)(((cp >> 6) & 0x3F) | 0x80);
        out += (char)(((cp >> 0) & 0x3F) | 0x80);
        continue;
      }
      if (cp <= 0x10FFFF) {
        // 4-byte unicode
        out += (char)(((cp >> 18) & 0x07) | 0xF0);
        out += (char)(((cp >> 12) & 0x3F) | 0x80);
        out += (char)(((cp >> 6) & 0x3F) | 0x80);
        out += (char)(((cp >> 0) & 0x3F) | 0x80);
        continue;
      }
    }
    return out;
  }
  std::string unicodeToUtf8(std::u32string &in) {
    std::string out = "";
    for (uint32_t cp : in) {
      if (cp <= 0x7F) {
        out += (char)cp;
        continue;
      }
      if (cp <= 0x07FF) {
        out += (char)(((cp >> 6) & 0x1F) | 0xC0);
        out += (char)(((cp >> 0) & 0x3F) | 0x80);
        continue;
      }
      if (cp <= 0xFFFF) {
        out += (char)(((cp >> 12) & 0x0F) | 0xE0);
        out += (char)(((cp >> 6) & 0x3F) | 0x80);
        out += (char)(((cp >> 0) & 0x3F) | 0x80);
        continue;
      }
      if (cp <= 0x10FFFF) {
        // 4-byte unicode
        out += (char)(((cp >> 18) & 0x07) | 0xF0);
        out += (char)(((cp >> 12) & 0x3F) | 0x80);
        out += (char)(((cp >> 6) & 0x3F) | 0x80);
        out += (char)(((cp >> 0) & 0x3F) | 0x80);
        continue;
      }
    }
    return out;
  }
  std::pair<size_t, size_t>
  calculateByteLength(size_t character_start = 0) const {
    return calculateByteLength(character_start, character_length);
  }
  std::pair<size_t, size_t> calculateByteLength(size_t character_start,
                                                size_t length) const {
    if(character_start == 0 && length == character_length){
        return std::pair(0, base.length());
    }
    const std::string &u = this->base;
    int l = u.length();
    size_t offset = 0;
    size_t character_pos = 0;
    size_t byte_offset = 0;
    size_t byte_len = 0;
    bool started_recording = character_start == 0;
    while (l > 0) {
      if (character_start > 0 && character_pos == character_start) {
        byte_offset = offset;
        started_recording = true;
      }
      if (character_pos - length == character_start)
        break;
      uint8_t u0 = u[offset];
      if (u0 >= 0 && u0 <= 127) {
        l -= 1;
        offset++;
        character_pos++;
        if (started_recording)
          byte_len += 1;
        continue;
      }
      if (l < 2)
        break;
      uint8_t u1 = u[offset + 1];
      if (u0 >= 192 && u0 <= 223) {
        l -= 2;
        offset += 2;
        character_pos++;
        if (started_recording)
          byte_len += 2;
        continue;
      }
      if ((uint8_t)u[offset] == 0xed && (u[offset + 1] & 0xa0) == 0xa0)
        break;
      if (l < 3)
        break;
      uint8_t u2 = u[offset + 2];
      if (u0 >= 224 && u0 <= 239) {
        l -= 3;
        offset += 3;
        character_pos++;
        if (started_recording)
          byte_len += 3;
        continue;
      }
      if (l < 4)
        break;
      uint8_t u3 = u[3];
      if (u0 >= 240 && u0 <= 247) {
        l -= 4;
        offset += 4;
        character_pos++;
        if (started_recording)
          byte_len += 4;
      }
    }
    if (character_start > 0 && character_pos == character_start &&
        byte_offset == 0) {
      byte_offset = offset;
      started_recording = true;
    }
    return std::pair(byte_offset, byte_len);
  }
  size_t calculateCharacterLength(std::string &u) {
    int l = u.length();
    size_t offset = 0;
    size_t character_len = 0;
    while (l > 0) {
      uint8_t u0 = u[offset];
      if (u0 >= 0 && u0 <= 127) {
        l -= 1;
        offset++;
        character_len += 1;
        continue;
      }
      if (l < 2)
        break;
      uint8_t u1 = u[offset + 1];
      if (u0 >= 192 && u0 <= 223) {
        l -= 2;
        offset += 2;
        character_len++;
        continue;
      }
      if ((uint8_t)u[offset] == 0xed && (u[offset + 1] & 0xa0) == 0xa0)
        break;
      if (l < 3)
        break;
      uint8_t u2 = u[offset + 2];
      if (u0 >= 224 && u0 <= 239) {
        l -= 3;
        offset += 3;
        character_len++;
        continue;
      }
      if (l < 4)
        break;
      uint8_t u3 = u[3];
      if (u0 >= 240 && u0 <= 247) {
        l -= 4;
        offset += 4;
        character_len++;
      }
    }
    return character_len;
  }

  void setState() {
    this->character_length = calculateCharacterLength(this->base);
  }
  std::vector<char32_t> toCodePoints(std::string &u, size_t off = 0,
                                     size_t len = 0) const {
    std::vector<char32_t> points;
    size_t character_pos = 0;
    int l = u.length();
    int ll = l;
    while (l > 0) {
      if (len != 0 && points.size() == len)
        break;
      size_t offset = ll - l;
      uint8_t u0 = u[offset];
      if (u0 >= 0 && u0 <= 127) {
        if (character_pos >= off)
          points.push_back((int32_t)u0);
        l -= 1;
        character_pos++;
        continue;
      }
      if (l < 2)
        break;
      uint8_t u1 = u[offset + 1];
      if (u0 >= 192 && u0 <= 223) {
        if (character_pos >= off)
          points.push_back((u0 - 192) * 64 + (u1 - 128));
        l -= 2;
        character_pos++;
        continue;
      }
      if ((uint8_t)u[offset] == 0xed && (u[offset + 1] & 0xa0) == 0xa0)
        break;
      if (l < 3)
        break;
      uint8_t u2 = u[offset + 2];
      if (u0 >= 224 && u0 <= 239) {
        if (character_pos >= off)
          points.push_back((u0 - 224) * 4096 + (u1 - 128) * 64 + (u2 - 128));
        l -= 3;

        character_pos++;
        continue;
      }
      if (l < 4)
        break;
      uint8_t u3 = u[offset + 3];
      if (u0 >= 240 && u0 <= 247) {
        if (character_pos >= off)
          points.push_back((u0 - 240) * 262144 + (u1 - 128) * 4096 +
                           (u2 - 128) * 64 + (u3 - 128));
        l -= 4;
        character_pos++;
      }
    }
    return points;
  }
  std::string base;
  size_t character_length = 0;
};

#endif