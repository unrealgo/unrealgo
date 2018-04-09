
#ifndef SG_MARKER_H
#define SG_MARKER_H

#include "lib/Array.h"
#include "board/GoPoint.h"
#include "board/GoPointSet.h"


class SgMarker {
 public:
  SgMarker();
  void Include(GoPoint p);
  bool Contains(GoPoint p) const;

  bool NewMark(GoPoint p);
  void Clear();
  void GetPoints(GoPointSet *points) const;

#ifndef NDEBUG

  bool markerInUse() const;

#endif

 private:
  void Init();

  int m_thisMark;

  GoArray<int, GO_MAXPOINT> m_mark;

#ifndef NDEBUG

  bool m_markerInUse;

  friend class SgReserveMarker;

#endif


  SgMarker(const SgMarker &);

  SgMarker &operator=(const SgMarker &);
};

inline SgMarker::SgMarker()
#ifndef NDEBUG
: m_markerInUse(false)
#endif
{
  Init();
}

inline void SgMarker::Clear() {
  if (++m_thisMark == 0)
    Init();
}

inline bool SgMarker::Contains(GoPoint p) const {
  return m_mark[p] == m_thisMark;
}

inline void SgMarker::GetPoints(GoPointSet *points) const {
  points->Clear();
  for (GoPoint p = 0; p < GO_MAXPOINT; ++p)
    if (Contains(p))
      points->Include(p);
}

inline void SgMarker::Include(GoPoint p) {
  DBG_ASSERT_BOARDRANGE(p);
  m_mark[p] = m_thisMark;
}

inline void SgMarker::Init() {
  m_thisMark = 1;
  m_mark.Fill(0);
}

inline bool SgMarker::NewMark(GoPoint p) {
  if (Contains(p))
    return false;
  Include(p);
  return true;
}

#ifndef NDEBUG

inline bool SgMarker::markerInUse() const {
  return m_markerInUse;
}

#endif


class SgReserveMarker {
 public:

  SgReserveMarker(SgMarker &marker);
  ~SgReserveMarker();

 private:
#ifndef NDEBUG
  SgMarker &m_marker;
#endif
};

#ifndef NDEBUG

inline SgReserveMarker::SgReserveMarker(SgMarker &marker)
        : m_marker(marker) {
  DBG_ASSERT(!marker.m_markerInUse);
  m_marker.m_markerInUse = true;
}

inline SgReserveMarker::~SgReserveMarker() {
  m_marker.m_markerInUse = false;
}

#else

inline SgReserveMarker::SgReserveMarker(SgMarker &marker) {
  SuppressUnused(marker);
}

inline SgReserveMarker::~SgReserveMarker() {}

#endif

#endif
