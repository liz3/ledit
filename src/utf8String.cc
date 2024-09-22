#include "utf8String.h"

Utf8String::Utf8String(std::string base) {
  this->base = base;
  setState();
}
Utf8String::Utf8String() {
  this->base = "";
  setState();
}
Utf8String::Utf8String(Utf8String &other) {
  this->base = other.base;
  this->idx = other.idx;
  setState();
}
Utf8String::Utf8String(const Utf8String &other) {
  this->base = other.base;
  this->idx = other.idx;
  setState();
}
Utf8String::Utf8String(const size_t len, const char32_t *ptr) {
  std::vector<char32_t> buff(ptr, ptr + len);
  base = unicodeToUtf8(buff);
  character_length = len;
}
Utf8String::Utf8String(const size_t len, const char32_t in) {
  std::vector<char32_t> buff;
  buff.push_back(in);
  base = unicodeToUtf8(buff);
  character_length = len;
}
Utf8String::Utf8String(const char32_t input[]) {
  std::u32string str(input);
  this->character_length = str.length();
  this->base = unicodeToUtf8(str);
}
Utf8String &Utf8String::operator+=(const char32_t input[]) {
  std::u32string str(input);
  this->character_length += str.length();
  this->base += unicodeToUtf8(str);
  return *this;
}
Utf8String &Utf8String::operator=(const char32_t input[]) {
  std::u32string str(input);
  this->character_length = str.length();
  this->base = unicodeToUtf8(str);
  return *this;
}
Utf8String operator+(const Utf8String &lhs,
                                        const char32_t rhs[]) {
  Utf8String n;
  n.append(lhs);
  n += rhs;
  return n;
}
Utf8String operator+(const Utf8String &lhs,
                                        const char32_t rhs) {
  Utf8String n;
  n.append(lhs);
  n.append(rhs);
  return n;
}
Utf8String operator+(const char32_t lhs[],
                                        const Utf8String &rhs) {
  Utf8String n;
  n += lhs;
  n.append(rhs);
  return n;
}

Utf8String operator+(const Utf8String lhs,
                                        const Utf8String &rhs) {
  Utf8String n;
  n.append(lhs);
  n.append(rhs);
  return n;
}

Utf8String &Utf8String::operator+=(const Utf8String &other) {
  append(other);
  return *this;
}

Utf8String &Utf8String::operator+=(const char32_t &other) {
  append(other);
  return *this;
}

bool Utf8String::operator==(const Utf8String &other) const {
  return this->getStrRef() == other.getStrRef();
}

bool Utf8String::operator!=(const Utf8String &other) const {
  return this->getStrRef() != other.getStrRef();
}

bool Utf8String::operator==(const char32_t input[]) {
  std::u32string str(input);
  std::string n = unicodeToUtf8(str);
  return n == base;
}

bool Utf8String::operator!=(const char32_t input[]) {
  std::u32string str(input);
  std::string n = unicodeToUtf8(str);
  return n != base;
}

size_t Utf8String::find(char32_t search, size_t start) const {
  const auto chars = getCodePointsRange(start);
  for (size_t i = start; i < this->character_length; i++) {
    if (chars[i] == search)
      return i;
  }
  return std::string::npos;
}
size_t Utf8String::find(Utf8String search, size_t start) {
  auto cps = getCodePointsRange(start);
  auto str = unicodeToUtf8(cps);
  return str.find(search.getStr());
}
size_t Utf8String::find(char32_t search) const {
  auto chars = getCodePoints();
  for (size_t i = 0; i < this->character_length; i++) {
    if (chars[i] == search)
      return i;
  }
  return std::string::npos;
}
size_t Utf8String::find(const Utf8String &other) const {
  if (other.length() == 1)
    return find(other[0]);
  if (character_length < other.length())
    return std::string::npos;
  for (size_t i = 0; i < character_length; i++) {
    if (this->substr(i, other.length()) == other)
      return i;
  }
  return std::string::npos;
}
Utf8String::iterator Utf8String::begin() { return iterator(this); }

Utf8String::iterator Utf8String::end() {
  return iterator(this, true, character_length);
}

char32_t Utf8String::operator[](int i) const { return getCharacterAt(i); }

size_t Utf8String::length() const { return this->character_length; }

size_t Utf8String::size() const { return this->character_length; }

std::string Utf8String::getStr() const { return this->base; }

const std::string &Utf8String::getStrRef() const { return this->base; }

std::vector<char32_t> Utf8String::getCodePoints() const {
  return this->toCodePoints(this->base);
}

std::vector<char32_t> Utf8String::getCodePointsRange(size_t off,
                                                     size_t len) const {
  return this->toCodePoints(this->base, off, len);
}

char32_t Utf8String::getCharacterAt(size_t index) const {
  if (character_length == 0)
    return 0;
  auto p = calculateByteLength(index, 1);
  if (!p.second)
    return 0;
  std::string sub = this->base.substr(p.first, p.second);
  std::vector<char32_t> entries = toCodePoints(sub);
  return entries[0];
}
Utf8String Utf8String::substr(size_t start) const {
  auto p = calculateByteLength(start);
  return Utf8String(this->base.substr(p.first, p.second));
}
Utf8String Utf8String::substr(size_t start, size_t len) const {
  auto p = calculateByteLength(start, len);
  return Utf8String(this->base.substr(p.first, p.second));
}

void Utf8String::erase(size_t start, size_t len) {
  auto p = calculateByteLength(start, len);
  this->base.erase(p.first, p.second);
  setState();
}
void Utf8String::insert(size_t index, const Utf8String &other) {
  appendAt(other, index);
}

void Utf8String::append(const Utf8String &other) {
  this->base += other.base;
  this->character_length += other.character_length;
}

void Utf8String::append(char32_t cp) {
  std::vector<char32_t> cps = {cp};
  this->append(cps);
}
void Utf8String::append(std::vector<char32_t> &cps) {
  std::string value = unicodeToUtf8(cps);
  character_length += cps.size();
  this->base += value;
}
void Utf8String::appendAt(const Utf8String &other, size_t start) {
  auto p = this->calculateByteLength(start);
  this->base.insert(p.first, other.base);
  this->character_length += other.character_length;
}
void Utf8String::appendAt(std::vector<char32_t> &cps, size_t start) {
  auto p = this->calculateByteLength(start);
  std::string value = unicodeToUtf8(cps);
  character_length += cps.size();
  this->base.insert(p.first, value);
}

void Utf8String::appendAt(char32_t cp, size_t start) {
  std::vector<char32_t> cps = {cp};
  this->appendAt(cps, start);
}
bool Utf8String::endsWith(const Utf8String &other) const {
  if (other.length() > character_length)
    return false;
  if (other.length() == character_length)
    return other.getStrRef() == getStrRef();
  const auto &ref = other.getStrRef();
  if (ref.length() > base.length())
    return false;
  return ref == base.substr(base.length() - ref.length());
}
void Utf8String::set(size_t idx, char32_t cc) {
  Utf8String temp;
  temp += cc;
  auto p = calculateByteLength(idx, 1);
  this->base.erase(p.first, p.second);
  this->base.insert(p.first, temp.getStrRef());
}
uint32_t Utf8String::getIdx() { return idx; }
void Utf8String::setIdx(uint32_t v) { idx = v; }

  std::string Utf8String::unicodeToUtf8(std::vector<char32_t> &in) {
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
  std::string Utf8String::unicodeToUtf8(std::u32string &in) {
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
  Utf8String::calculateByteLength(size_t character_start) const {
    return calculateByteLength(character_start, character_length);
  }
  std::pair<size_t, size_t> Utf8String::calculateByteLength(size_t character_start,
                                                size_t length) const {
    if (character_start == 0 && length == character_length) {
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
        continue;
      }
      break;
    }
    if (character_start > 0 && character_pos == character_start &&
        byte_offset == 0) {
      byte_offset = offset;
      started_recording = true;
    }
    return std::pair(byte_offset, byte_len);
  }
   size_t Utf8String::calculateCharacterLength(std::string &u) {
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
        continue;
      }
      return character_len + l;
    }
    return character_len;
  }

    void Utf8String::setState() {
    this->character_length = calculateCharacterLength(this->base);
  }
  std::vector<char32_t> Utf8String::toCodePoints(const std::string &u, size_t off,
                                     size_t len) const {
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
        continue;
      }
      break;
    }
    return points;
  }
