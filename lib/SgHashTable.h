//----------------------------------------------------------------------------
/** @file SgHashTable.h
    Hash table. */
//----------------------------------------------------------------------------

#ifndef SG_HASHTABLE_H
#define SG_HASHTABLE_H

#include "lib/SgHash.h"
#include "board/SgWrite.h"

template<class DATA>
struct SgHashEntry {
  SgHashEntry()
      : m_hash(),
        m_data() {}

  SgHashEntry(const SgHashCode &code, const DATA &data)
      : m_hash(code),
        m_data(data) {}

  SgHashCode m_hash;
  DATA m_data;
};


/** SgHashTable implements an array of DATA.
	The implementation probes the table in BLOCK_SIZE consecutive locations,
    as in the Fruit chess program.
	See http://arctrix.com/nas/chess/fruit/fruit_21_linux.zip, file trans.cpp
    (accessed on Oct 23, 2011).
    The new entry is always written, overwriting the least valuable among
    the BLOCK_SIZE entries.
    The table size is increased by BLOCK_SIZE - 1 entries to avoid
    an expensive modulo operation in the scan.
    A good value for BLOCK_SIZE is 4.
*/
template<class DATA, int BLOCK_SIZE = 1>
class SgHashTable {
 public:
  class Iterator {
   public:
    Iterator(const SgHashTable &array);
    const SgHashEntry<DATA> &operator*() const;
    const SgHashEntry<DATA> *operator->() const;
    void operator++();
    operator bool() const;

   private:
    void SkipInvalidEntries();
    const SgHashEntry<DATA> *m_end;
    const SgHashEntry<DATA> *m_current;
  };

  explicit SgHashTable(int maxHash);
  ~SgHashTable();
  void Age();
  void Clear();
  bool Lookup(const SgHashCode &code, DATA *data) const;
  int MaxHash() const;
  bool Store(const SgHashCode &code, const DATA &data);

  size_t NuCollisions() const {
    return m_nuCollisions;
  }

  size_t NuStores() const {
    return m_nuStores;
  }

  size_t NuLookups() const {
    return m_nuLookups;
  }

  size_t NuFound() const {
    return m_nuFound;
  }

 private:
  SgHashEntry<DATA> *m_entry;
  int m_maxHash;

  // @todo the following statistics can be made debug only
  // @todo pass a HashStatistics class to the HashTable when constructed

  mutable size_t m_nuCollisions;
  mutable size_t m_nuStores;
  mutable size_t m_nuLookups;
  mutable size_t m_nuFound;
  SgHashTable(const SgHashTable &) = delete;
  SgHashTable &operator=(const SgHashTable &) = delete;
};

template<class DATA, int BLOCK_SIZE>
SgHashTable<DATA, BLOCK_SIZE>::Iterator::Iterator(const SgHashTable &ht)
    : m_end(ht.m_entry + ht.m_maxHash + BLOCK_SIZE - 1),
      m_current(ht.m_entry) {
  SkipInvalidEntries();
}

template<class DATA, int BLOCK_SIZE>
const SgHashEntry<DATA> &SgHashTable<DATA, BLOCK_SIZE>::Iterator::operator*()
const {
  DBG_ASSERT(*this);
  DBG_ASSERT(m_current);
  return *m_current;
}

template<class DATA, int BLOCK_SIZE>
const SgHashEntry<DATA> *SgHashTable<DATA, BLOCK_SIZE>::Iterator::operator->()
const {
  DBG_ASSERT(*this);
  DBG_ASSERT(m_current);
  return m_current;
}

template<class DATA, int BLOCK_SIZE>
void SgHashTable<DATA, BLOCK_SIZE>::Iterator::operator++() {
  ++m_current;
  SkipInvalidEntries();
}

template<class DATA, int BLOCK_SIZE>
SgHashTable<DATA, BLOCK_SIZE>::Iterator::operator bool() const {
  return m_current < m_end;
}

template<class DATA, int BLOCK_SIZE>
void SgHashTable<DATA, BLOCK_SIZE>::Iterator::SkipInvalidEntries() {
  while (m_current < m_end && !m_current->m_data.IsValid())
    ++m_current;
}

template<class DATA, int BLOCK_SIZE>
SgHashTable<DATA, BLOCK_SIZE>::SgHashTable(int maxHash)
    : m_entry(0),
      m_maxHash(maxHash),
      m_nuCollisions(0),
      m_nuStores(0),
      m_nuLookups(0),
      m_nuFound(0) {
  m_entry = new SgHashEntry<DATA>[m_maxHash + BLOCK_SIZE - 1];
  Clear();
}

template<class DATA, int BLOCK_SIZE>
SgHashTable<DATA, BLOCK_SIZE>::~SgHashTable() {
  delete[] m_entry;
}

template<class DATA, int BLOCK_SIZE>
void SgHashTable<DATA, BLOCK_SIZE>::Age() {
  for (int i = m_maxHash + BLOCK_SIZE - 2; i >= 0; --i)
    m_entry[i].m_data.AgeData();
}

template<class DATA, int BLOCK_SIZE>
void SgHashTable<DATA, BLOCK_SIZE>::Clear() {
  for (int i = m_maxHash + BLOCK_SIZE - 2; i >= 0; --i) {
    m_entry[i].m_data.Invalidate();
  }
}

template<class DATA, int BLOCK_SIZE>
int SgHashTable<DATA, BLOCK_SIZE>::MaxHash() const {
  return m_maxHash;
}

template<class DATA, int BLOCK_SIZE>
bool SgHashTable<DATA, BLOCK_SIZE>::Store(const SgHashCode &code,
                                          const DATA &data) {
  ++m_nuStores;
  int h = code.Hash(m_maxHash);
  int best = -1;
  bool collision = true;
  for (int i = h; i < h + BLOCK_SIZE; i++) {
    if (!m_entry[i].m_data.IsValid() || m_entry[i].m_hash == code) {
      best = i;
      collision = false;
      break;
    } else if (best == -1
        || m_entry[best].m_data.IsBetterThan(m_entry[i].m_data)
        )
      best = i;
  }
  if (collision)
    ++m_nuCollisions;
  DBG_ASSERTRANGE(best, h, h + BLOCK_SIZE - 1);
  SgHashEntry<DATA> &entry = m_entry[best];
  entry.m_hash = code;
  entry.m_data = data;
  return true;
}

template<class DATA, int BLOCK_SIZE>
bool SgHashTable<DATA, BLOCK_SIZE>::Lookup(const SgHashCode &code,
                                           DATA *data) const {
  ++m_nuLookups;
  int h = code.Hash(m_maxHash);
  for (int i = h; i < h + BLOCK_SIZE; i++) {
    const SgHashEntry<DATA> &entry = m_entry[i];
    if (entry.m_data.IsValid()) {
      if (entry.m_hash == code) {
        *data = entry.m_data;
        ++m_nuFound;
        return true;
      }
    } else
      return false;
  }
  return false;
}

template<class DATA, int BLOCK_SIZE>
std::ostream &operator<<(std::ostream &out,
                         const SgHashTable<DATA, BLOCK_SIZE> &hash) {
  out << "HashTableStatistics:\n"
      << SgWriteLabel("Stores") << hash.NuStores() << '\n'
      << SgWriteLabel("LookupAttempt") << hash.NuLookups() << '\n'
      << SgWriteLabel("LookupSuccess") << hash.NuFound() << '\n'
      << SgWriteLabel("Collisions") << hash.NuCollisions() << '\n';
  return out;
}

#endif // SG_HASHTABLE_H
