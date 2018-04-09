//----------------------------------------------------------------------------
/** @file SgHash.h
    Hash codes and Zobrist tables.

    See A.L. Zobrist "A New Hashing Method with Application for Game Playing",
    Techn. Rep. #88, Univ. of Wisconsin, Madison, WI 53706, April 1970.
    (Reprinted in ICCA Journal, Spring 1990?.) */
//----------------------------------------------------------------------------

#ifndef SG_HASH_H
#define SG_HASH_H

#include <algorithm>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "lib/Array.h"
#include "platform/SgException.h"
#include "lib/SgRandom.h"


template<int N>
class SgHash {
 public:
  SgHash() {}
  SgHash(unsigned int key);

  ~SgHash() {}

  void Clear();
  bool operator<(const SgHash &code) const;
  bool operator==(const SgHash &code) const;
  bool operator!=(const SgHash &code) const;
  bool IsZero() const;
  void Xor(const SgHash &code);
  unsigned int Hash(int max) const;
  unsigned int Code1() const;
  unsigned int Code2() const;
  static SgHash Random(SgRandom &random);
  void RollLeft(int n);
  void RollRight(int n);
  std::string ToString() const;
  void FromString(const std::string &str);
  static int Size();

 private:
  unsigned int Mix32(int key) const;
  unsigned int GetWord() const;
  std::bitset<N> m_code;
};
typedef SgHash<64> SgHashCode;

template<int N>
SgHash<N>::SgHash(unsigned int key)
    : m_code(Mix32(key)) {
  for (int i = 1; i < (N / 32); ++i) {
    unsigned int mix = Mix32(GetWord());
    m_code <<= 32;
    m_code |= mix;
  }
}

template<int N>
bool SgHash<N>::operator<(const SgHash &code) const {
  for (int i = N - 1; i >= 0; --i) {
    bool c1 = m_code[i];
    bool c2 = code.m_code[i];
    if (!c1 && c2)
      return true;
    if (c1 && !c2)
      return false;
  }
  return false;
}

template<int N>
bool SgHash<N>::operator==(const SgHash &code) const {
  return code.m_code == m_code;
}

template<int N>
bool SgHash<N>::operator!=(const SgHash &code) const {
  return code.m_code != m_code;
}

template<int N>
void SgHash<N>::Clear() {
  m_code.reset();
}

template<int N>
unsigned int SgHash<N>::Code1() const {
  return GetWord();
}

template<int N>
unsigned int SgHash<N>::Code2() const {
  return (m_code >> 32).to_ulong();
}

template<int N>
void SgHash<N>::FromString(const std::string &str) {
  Clear();
  for (std::string::const_iterator i_str = str.begin();
       i_str != str.end(); ++i_str) {
    m_code <<= 4;
    char c = *i_str;
    if (c >= '0' && c <= '9')
      m_code |= std::bitset<N>(c - '0');
    else if (c >= 'A' && c <= 'F')
      m_code |= std::bitset<N>(10 + c - 'A');
    else if (c >= 'a' && c <= 'f')
      m_code |= std::bitset<N>(10 + c - 'a');
    else throw SgException("Bad hex in hash string");
  }
}

template<int N>
unsigned int SgHash<N>::GetWord() const {
  static const std::bitset<N> mask(0xffffffffUL);
  return static_cast<unsigned int>((m_code & mask).to_ulong());
}

template<int N>
unsigned int SgHash<N>::Hash(int max) const {
  return GetWord() % max;
}

template<int N>
bool SgHash<N>::IsZero() const {
  return m_code.none();
}

template<int N>
unsigned int SgHash<N>::Mix32(int key) const {
  key += ~(key << 15);
  key ^= (key >> 10);
  key += (key << 3);
  key ^= (key >> 6);
  key += ~(key << 11);
  key ^= (key >> 16);
  return key;
}

template<int N>
SgHash<N> SgHash<N>::Random(SgRandom &random) {
  SgHash hashcode;
  hashcode.m_code = random.Int();
  for (int i = 1; i < (N / 32); ++i) {
    hashcode.m_code <<= 32;
    hashcode.m_code |= random.Int();
  }

  return hashcode;
}

template<int N>
void SgHash<N>::RollLeft(int n) {
  m_code = (m_code << n) ^ (m_code >> (N - n));
}

template<int N>
void SgHash<N>::RollRight(int n) {
  m_code = (m_code >> n) ^ (m_code << (N - n));
}

template<int N>
int SgHash<N>::Size() {
  return N;
}

template<int N>
std::string SgHash<N>::ToString() const {
  std::ostringstream buffer;
  buffer.fill('0');
  std::bitset<N> mask(0xff);
  for (int i = (N + 7) / 8 - 1; i >= 0; --i) {
    std::bitset<N> b = ((m_code >> (i * 8)) & mask);
    buffer << std::hex << std::setw(2) << b.to_ulong();
  }
  return buffer.str();
}

template<int N>
void SgHash<N>::Xor(const SgHash &code) {
  m_code ^= code.m_code;
}

template<int N>
std::ostream &operator<<(std::ostream &out, const SgHash<N> &hash) {
  out << hash.ToString();
  return out;
}

template<int N>
std::istream &operator>>(std::istream &in, const SgHash<N> &hash) {
  std::string str;
  in >> str;
  hash.FromString(str);
  return in;
}

template<int N>
class SgHashZobrist {
 public:
  static const int MAX_HASH_INDEX = 1500;
  SgHashZobrist(SgRandom &random);

  const SgHash<N> &Get(int index) const {
    return m_hash[index];
  }

  static const SgHashZobrist &GetTable();

 private:
  static SgRandom s_random;
  static SgHashZobrist s_globalTable;
  GoArray<SgHash<N>, MAX_HASH_INDEX> m_hash;
};

typedef SgHashZobrist<64> SgHashZobristTable;

template<int N>
SgRandom SgHashZobrist<N>::s_random;
template<int N>
SgHashZobrist<N> SgHashZobrist<N>::s_globalTable(s_random);

template<int N>
SgHashZobrist<N>::SgHashZobrist(SgRandom &random) {
  for (int i = 0; i < MAX_HASH_INDEX; ++i)
    m_hash[i] = SgHash<N>::Random(random);
}

template<int N>
const SgHashZobrist<N> &SgHashZobrist<N>::GetTable() {
  return s_globalTable;
}


namespace SgHashUtil {
template<int N>
inline SgHash<N> GetZobrist(int index) {
  return SgHashZobrist<N>::GetTable().Get(index);
}

template<int N>
inline void XorZobrist(SgHash<N> &hash, int index) {
  hash.Xor(GetZobrist<N>(index));
}

template<int N>
inline void XorInteger(SgHash<N> &hash, int index) {
  hash.Xor(SgHash<N>(index));
}
}

#endif // SG_HASH_H
