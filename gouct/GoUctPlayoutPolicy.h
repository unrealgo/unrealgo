
#ifndef GOUCT_PLAYOUTPOLICY_H
#define GOUCT_PLAYOUTPOLICY_H

#include <iostream>
#include <boost/array.hpp>
#include "GoBoardUtil.h"
#include "GoEyeUtil.h"
#include "GoUctPureRandomGenerator.h"
#include "GoUctGammaMoveGenerator.h"

class GoUctPlayoutPolicyParam {
 public:
  bool m_statisticsEnabled;
  bool m_useNakadeHeuristic;
  bool m_usePatternsInPlayout;
  bool m_usePatternsInPriorKnowledge;
  int m_fillboardTries;
  float m_patternGammaThreshold;
  GoUctPlayoutPolicyParam();
};
enum GoUctPlayoutPolicyType {
  GOUCT_FILLBOARD,
  GOUCT_NAKADE,
  GOUCT_ATARI_CAPTURE,
  GOUCT_ATARI_DEFEND,
  GOUCT_LOWLIB,
  GOUCT_PATTERN,
  GOUCT_GAMMA_PATTERN,
  GOUCT_REPLACE_CAPTURE,
  GOUCT_CAPTURE,
  GOUCT_RANDOM,
  GOUCT_SELFATARI_CORRECTION,
  GOUCT_CLUMP_CORRECTION,
  GOUCT_PASS,
  _GOUCT_NU_DEFAULT_PLAYOUT_TYPE
};
const char *GoUctPlayoutPolicyTypeStr(GoUctPlayoutPolicyType type);
struct GoUctPlayoutPolicyStat {
  std::size_t m_nuMoves;
  UctStatistics m_nonRandLen;
  UctStatistics m_moveListLen;
  boost::array<std::size_t, _GOUCT_NU_DEFAULT_PLAYOUT_TYPE> m_nuMoveType;
  void Clear();
  void Write(std::ostream &out) const;
};
template<class BOARD>
class GoUctPlayoutPolicy {
 public:
  GoUctPlayoutPolicy(const BOARD &bd, const GoUctPlayoutPolicyParam &param);
  GoPoint GenerateMove();
  void EndPlayout();
  void StartPlayout();
  void OnPlay();
  GoUctPlayoutPolicyType MoveType() const;
  const GoUctPlayoutPolicyStat &Statistics(SgBlackWhite color) const;
  void ClearStatistics();
  GoPointList AllRandomMoves() const;
  GoPointList GetEquivalentBestMoves() const;
  const GoUctPlayoutPolicyParam &Param() const;
  const GoUctGammaMoveGenerator<BOARD> &GammaGenerator() const;
  typedef bool Corrector(const BOARD &, GoPoint &);
  bool CorrectMove(typename GoUctPlayoutPolicy<BOARD>::Corrector &
  corrFunction,
                   GoPoint &move,
                   GoUctPlayoutPolicyType moveType);
  GoPointList GetPolicyMoves(GoUctPlayoutPolicyType type);

 private:
  class CaptureGenerator {
   public:
    CaptureGenerator(const BOARD &bd);
    void StartPlayout();
    void OnPlay();
    void Generate(GoPointList &moves);

   private:
    const BOARD &m_bd;
    std::vector<GoPoint> m_candidates;
  };
  static const bool SECOND_LAST_MOVE_PATTERNS = true;
  static const bool USE_CLUMP_CORRECTION = false;
  static const bool DEBUG_CORRECT_MOVE = false;
  const BOARD &m_bd;
  const GoUctPlayoutPolicyParam &m_param;
  bool m_checked;
  GoUctPlayoutPolicyType m_moveType;
  std::size_t m_nonRandLen;
  GoPoint m_lastMove;
  GoPointList m_moves;
  SgRandom m_random;
  GoUctGammaMoveGenerator<BOARD> m_gammaGenerator;
  CaptureGenerator m_captureGenerator;
  GoUctPureRandomGenerator<BOARD> m_pureRandomGenerator;
  SgBWArray<GoUctPlayoutPolicyStat> m_statistics;
  bool GenerateAtariCaptureMove();
  bool GenerateAtariDefenseMove();
  bool GenerateLowLibMove(GoPoint lastMove);
  bool GenerateNakadeMove();
  void GenerateNakadeMove(GoPoint p);
  /** Generate pattern move around last two moves */
  bool GeneratePatternMove();
  void GeneratePatternMove(GoPoint p);
  void GeneratePatternMove2(GoPoint p, GoPoint lastMove);
  void GeneratePureRandom();
  bool GeneratePoint(GoPoint p) const;
  /** Does playing on a liberty increase number of liberties for block?
      If yes, add to m_moves.
      Disabled if both liberties are simple chain libs, e.g. bamboo. */
  void PlayGoodLiberties(GoPoint block);
  /** see GoUctUtil::SelectRandom */
  GoPoint SelectRandom();
  /** Add statistics for most recently generated move. */
  void UpdateStatistics();
};

template<class BOARD>
inline const GoUctPlayoutPolicyParam &GoUctPlayoutPolicy<BOARD>::Param() const {
  return m_param;
}

template<class BOARD>
GoUctPlayoutPolicy<BOARD>::CaptureGenerator::CaptureGenerator(const BOARD &bd)
    : m_bd(bd) {
  m_candidates.reserve(GO_MAX_NUM_MOVES);
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::CaptureGenerator::StartPlayout() {
  m_candidates.clear();
  for (typename BOARD::Iterator it(m_bd); it; ++it) {
    const GoPoint p = *it;
    if (m_bd.Occupied(p) && m_bd.Anchor(p) == p && m_bd.InAtari(p))
      m_candidates.push_back(p);
  }
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::CaptureGenerator::OnPlay() {
  GoPoint lastMove = m_bd.GetLastMove();
  if (lastMove == GO_NULLMOVE || lastMove == GO_PASS)
    return;
  if (m_bd.OccupiedInAtari(lastMove))
    m_candidates.push_back(m_bd.Anchor(lastMove));
  if (m_bd.NumNeighbors(lastMove, m_bd.ToPlay()) == 0)
    return;
  if (m_bd.OccupiedInAtari(lastMove + GO_NORTH_SOUTH))
    m_candidates.push_back(m_bd.Anchor(lastMove + GO_NORTH_SOUTH));
  if (m_bd.OccupiedInAtari(lastMove - GO_NORTH_SOUTH))
    m_candidates.push_back(m_bd.Anchor(lastMove - GO_NORTH_SOUTH));
  if (m_bd.OccupiedInAtari(lastMove + GO_WEST_EAST))
    m_candidates.push_back(m_bd.Anchor(lastMove + GO_WEST_EAST));
  if (m_bd.OccupiedInAtari(lastMove - GO_WEST_EAST))
    m_candidates.push_back(m_bd.Anchor(lastMove - GO_WEST_EAST));
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::CaptureGenerator::Generate(GoPointList &moves) {
  DBG_ASSERT(moves.IsEmpty());
  const SgBlackWhite opp = m_bd.Opponent();
  // For efficiency reasons, this function does not check, if the same
  // move is generated multiple times (and will therefore played with
  // higher probabilty, if there are also other capture moves), because in
  // nearly all cases, there is zero or one global capture move on the
  // board. Most captures are done immediately by the atari heuristic
  for (size_t i = 0; i < m_candidates.size(); ++i) {
    const GoPoint p = m_candidates[i];
    if (!m_bd.OccupiedInAtari(p)) {
      m_candidates[i] = m_candidates[m_candidates.size() - 1];
      m_candidates.pop_back();
      --i;
      continue;
    }
    if (m_bd.GetColor(p) == opp)
      moves.PushBack(m_bd.TheLiberty(p));
  }
}

template<class BOARD>
GoUctPlayoutPolicy<BOARD>::GoUctPlayoutPolicy(const BOARD &bd,
                                              const GoUctPlayoutPolicyParam &param)
    : m_bd(bd),
      m_param(param),
      m_checked(false),
      m_gammaGenerator(bd, param.m_patternGammaThreshold, m_random),
      m_captureGenerator(bd),
      m_pureRandomGenerator(bd, m_random) {
  ClearStatistics();
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::ClearStatistics() {
  m_statistics[SG_BLACK].Clear();
  m_statistics[SG_WHITE].Clear();
}

template<class BOARD>
bool GoUctPlayoutPolicy<BOARD>::CorrectMove(
    typename GoUctPlayoutPolicy<BOARD>::Corrector &corrFunction,
    GoPoint &mv, GoUctPlayoutPolicyType moveType) {
#if DEBUG
  const GoPoint oldMv = mv;
#endif
  if (!corrFunction(m_bd, mv))
    return false;

  m_moves.SetTo(mv);
  m_moveType = moveType;

#if DEBUG
  if (DEBUG_CORRECT_MOVE)
      SgDebug() << m_bd
                << "Replace " << GoWriteMove(oldMv, m_bd.ToPlay())
                << " by " << GoWriteMove(mv, m_bd.ToPlay()) << '\n';
#endif
  return true;
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::EndPlayout() {}

template<class BOARD>
bool GoUctPlayoutPolicy<BOARD>::GenerateAtariCaptureMove() {
  DBG_ASSERT(!SgIsSpecialMove(m_lastMove));
  if (m_bd.InAtari(m_lastMove)) {
    GoMove mv = m_bd.TheLiberty(m_lastMove);
    m_moves.PushBack(mv);
    return true;
  }
  return false;
}

template<class BOARD>
bool GoUctPlayoutPolicy<BOARD>::GenerateAtariDefenseMove() {
  return GoBoardUtil::AtariDefenseMoves(m_bd, m_lastMove, m_moves);
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::PlayGoodLiberties(GoPoint block) {
  GoPoint ignoreOther;
  if (!GoBoardUtil::IsSimpleChain(m_bd, block, ignoreOther))
    for (typename BOARD::LibertyIterator it(m_bd, block); it; ++it)
      if (GoBoardUtil::GainsLiberties(m_bd, block, *it)
          && !GoBoardUtil::SelfAtari(m_bd, *it)
          )
        m_moves.PushBack(*it);
}

template<class BOARD>
bool GoUctPlayoutPolicy<BOARD>::GenerateLowLibMove(GoPoint lastMove) {
  DBG_ASSERT(!SgIsSpecialMove(lastMove));
  DBG_ASSERT(!m_bd.IsEmpty(lastMove));
  DBG_ASSERT(m_moves.IsEmpty());

  const SgBlackWhite toPlay = m_bd.ToPlay();

  // take liberty of last move
  if (m_bd.NumLiberties(lastMove) == 2) {
    const GoPoint anchor = m_bd.Anchor(lastMove);
    PlayGoodLiberties(anchor);
  }

  if (m_bd.NumNeighbors(lastMove, toPlay) != 0) {
    // play liberties of neighbor blocks
    GoArrayList<GoPoint, 4> ourLowLibBlocks;
    for (GoNb4Iterator<BOARD> it(m_bd, lastMove); it; ++it) {
      if (m_bd.GetColor(*it) == toPlay
          && m_bd.NumLiberties(*it) == 2) {
        const GoPoint anchor = m_bd.Anchor(*it);
        if (!ourLowLibBlocks.Contains(anchor)) {
          ourLowLibBlocks.PushBack(anchor);
          PlayGoodLiberties(anchor);
        }
      }
    }
  }

  return !m_moves.IsEmpty();
}

template<class BOARD>
SG_ATTR_FLATTEN GoPoint GoUctPlayoutPolicy<BOARD>::GenerateMove() {
  m_moves.Clear();
  m_checked = false;
  GoPoint mv = GO_NULLMOVE;

  if (m_param.m_fillboardTries > 0) {
    m_moveType = GOUCT_FILLBOARD;
    mv = m_pureRandomGenerator.
        GenerateFillboardMove(m_param.m_fillboardTries);
  }

  m_lastMove = m_bd.GetLastMove();

  if (mv == GO_NULLMOVE
      && !SgIsSpecialMove(m_lastMove) // skip if Pass or Null
      && !m_bd.IsEmpty(m_lastMove) // skip if move was suicide
      ) {
    if (m_param.m_useNakadeHeuristic && GenerateNakadeMove()) {
      m_moveType = GOUCT_NAKADE;
      mv = SelectRandom();
    }
    if (mv == GO_NULLMOVE && GenerateAtariCaptureMove()) {
      m_moveType = GOUCT_ATARI_CAPTURE;
      mv = SelectRandom();
    }
    if (mv == GO_NULLMOVE && GenerateAtariDefenseMove()) {
      m_moveType = GOUCT_ATARI_DEFEND;
      mv = SelectRandom();
    }
    if (mv == GO_NULLMOVE && GenerateLowLibMove(m_lastMove)) {
      m_moveType = GOUCT_LOWLIB;
      mv = SelectRandom();
    }
    if (mv == GO_NULLMOVE) {
      if (m_param.m_usePatternsInPlayout) {
        m_moveType = GOUCT_GAMMA_PATTERN;
        mv = m_gammaGenerator.GenerateBiasedPatternMove();
      } else if (GeneratePatternMove()) {
        m_moveType = GOUCT_PATTERN;
        mv = SelectRandom();
      }
    }
  }

  if (mv != GO_NULLMOVE)
    CorrectMove(GoUctUtil::DoFalseEyeToCaptureCorrection, mv,
                GOUCT_REPLACE_CAPTURE);
  if (mv == GO_NULLMOVE) {
    m_moveType = GOUCT_CAPTURE;
    m_captureGenerator.Generate(m_moves);
    mv = SelectRandom();
  }
  if (mv == GO_NULLMOVE) {
    m_moveType = GOUCT_RANDOM;
    mv = m_pureRandomGenerator.Generate();
  }
  if (mv == GO_NULLMOVE) {
    m_moveType = GOUCT_PASS;
    mv = GO_PASS;
    m_checked = true;
  } else {
    DBG_ASSERT(m_bd.IsLegal(mv));
    m_checked = CorrectMove(GoUctUtil::DoSelfAtariCorrection, mv,
                            GOUCT_SELFATARI_CORRECTION);
    if (USE_CLUMP_CORRECTION && !m_checked)
      CorrectMove(GoUctUtil::DoClumpCorrection, mv,
                  GOUCT_CLUMP_CORRECTION);
  }
  DBG_ASSERT(m_bd.IsLegal(mv));
  DBG_ASSERT(mv == GO_PASS || !m_bd.IsSuicide(mv));

  if (m_param.m_statisticsEnabled)
    UpdateStatistics();

  return mv;
}

/** Nakade heuristic.
    If there is a region of three empty points adjacent to last move, play in
    the center of the region. */
template<class BOARD>
bool GoUctPlayoutPolicy<BOARD>::GenerateNakadeMove() {
  DBG_ASSERT(m_moves.IsEmpty());
  DBG_ASSERT(!SgIsSpecialMove(m_lastMove));
  GenerateNakadeMove(m_lastMove + GO_NORTH_SOUTH);
  GenerateNakadeMove(m_lastMove - GO_NORTH_SOUTH);
  GenerateNakadeMove(m_lastMove + GO_WEST_EAST);
  GenerateNakadeMove(m_lastMove - GO_WEST_EAST);
  // Ignore duplicates in move list, happens rarely
  return !m_moves.IsEmpty();
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::GenerateNakadeMove(GoPoint p) {
  SgBlackWhite toPlay = m_bd.ToPlay();
  if (m_bd.IsEmpty(p) && m_bd.NumNeighbors(p, toPlay) == 0) {
    int numEmptyNeighbors = m_bd.NumEmptyNeighbors(p);
    if (numEmptyNeighbors == 2) {
      int n = 0;
      for (GoNb4Iterator<BOARD> it(m_bd, p); it; ++it)
        if (m_bd.IsEmpty(*it)) {
          if (m_bd.NumEmptyNeighbors(*it) != 1
              || m_bd.NumNeighbors(*it, toPlay) > 0
              )
            return;
          if (++n > 2)
            break;
        }
      m_moves.PushBack(p);
    } else if (numEmptyNeighbors == 1) {
      for (GoNb4Iterator<BOARD> it(m_bd, p); it; ++it)
        if (m_bd.IsEmpty(*it)) {
          if (m_bd.NumEmptyNeighbors(*it) != 2
              || m_bd.NumNeighbors(*it, toPlay) > 0
              )
            return;
          for (GoNb4Iterator<BOARD> it2(m_bd, *it); it2; ++it2)
            if (m_bd.IsEmpty(*it2) && *it2 != p) {
              if (m_bd.NumEmptyNeighbors(*it2) == 1
                  && m_bd.NumNeighbors(*it2, toPlay) == 0
                  )
                m_moves.PushBack(*it);
              break;
            }
          break;
        }

    }
  }
}

template<class BOARD>
bool GoUctPlayoutPolicy<BOARD>::GeneratePatternMove() {
  DBG_ASSERT(m_moves.IsEmpty());
  DBG_ASSERT(!SgIsSpecialMove(m_lastMove));
  GeneratePatternMove(m_lastMove + GO_NORTH_SOUTH - GO_WEST_EAST);
  GeneratePatternMove(m_lastMove + GO_NORTH_SOUTH);
  GeneratePatternMove(m_lastMove + GO_NORTH_SOUTH + GO_WEST_EAST);
  GeneratePatternMove(m_lastMove - GO_WEST_EAST);
  GeneratePatternMove(m_lastMove + GO_WEST_EAST);
  GeneratePatternMove(m_lastMove - GO_NORTH_SOUTH - GO_WEST_EAST);
  GeneratePatternMove(m_lastMove - GO_NORTH_SOUTH);
  GeneratePatternMove(m_lastMove - GO_NORTH_SOUTH + GO_WEST_EAST);
  if (SECOND_LAST_MOVE_PATTERNS) {
    const GoPoint lastMove2 = m_bd.Get2ndLastMove();
    if (!SgIsSpecialMove(lastMove2)) {
      GeneratePatternMove2(lastMove2 + GO_NORTH_SOUTH - GO_WEST_EAST, m_lastMove);
      GeneratePatternMove2(lastMove2 + GO_NORTH_SOUTH, m_lastMove);
      GeneratePatternMove2(lastMove2 + GO_NORTH_SOUTH + GO_WEST_EAST, m_lastMove);
      GeneratePatternMove2(lastMove2 - GO_WEST_EAST, m_lastMove);
      GeneratePatternMove2(lastMove2 + GO_WEST_EAST, m_lastMove);
      GeneratePatternMove2(lastMove2 - GO_NORTH_SOUTH - GO_WEST_EAST, m_lastMove);
      GeneratePatternMove2(lastMove2 - GO_NORTH_SOUTH, m_lastMove);
      GeneratePatternMove2(lastMove2 - GO_NORTH_SOUTH + GO_WEST_EAST, m_lastMove);
    }
  }
  return !m_moves.IsEmpty();
}

template<class BOARD>
inline void GoUctPlayoutPolicy<BOARD>::GeneratePatternMove(GoPoint p) {
  if (m_bd.IsEmpty(p)
      && !GoBoardUtil::SelfAtari(m_bd, p))
    m_moves.PushBack(p);
}

template<class BOARD>
inline void GoUctPlayoutPolicy<BOARD>::GeneratePatternMove2(GoPoint p,
                                                            GoPoint lastMove) {
  if (m_bd.IsEmpty(p)
      && !GoPointUtil::In8Neighborhood(lastMove, p)
      && !GoBoardUtil::SelfAtari(m_bd, p))
    m_moves.PushBack(p);
}

template<class BOARD>
inline bool GoUctPlayoutPolicy<BOARD>::GeneratePoint(GoPoint p) const {
  return GoUctUtil::GeneratePoint(m_bd, p, m_bd.ToPlay());
}

template<class BOARD>
GoPointList GoUctPlayoutPolicy<BOARD>::AllRandomMoves() const {
  GoPointList result;
  for (typename BOARD::Iterator it(m_bd); it; ++it)
    if (m_bd.IsEmpty(*it) && GeneratePoint(*it))
      result.PushBack(*it);
  return result;
}

template<class BOARD>
GoPointList GoUctPlayoutPolicy<BOARD>::GetEquivalentBestMoves() const {
  if (m_moveType == GOUCT_RANDOM)
    return AllRandomMoves();

  // Move in move_info are not checked yet, if legal etc.
  GoPointList result;
  for (GoPointList::Iterator it(m_moves); it; ++it)
    if (m_checked || GeneratePoint(*it))
      result.PushBack(*it);
  return result;
}

template<class BOARD>
GoPointList GoUctPlayoutPolicy<BOARD>::
GetPolicyMoves(GoUctPlayoutPolicyType type) {
  StartPlayout();
  m_moves.Clear();
  m_lastMove = m_bd.GetLastMove();
  bool hasLastMove = !SgIsSpecialMove(m_lastMove)
      && !m_bd.IsEmpty(m_lastMove); // skip if move was suicide
  switch (type) {
    case GOUCT_FILLBOARD:DBG_ASSERT(false); // changes the move list.
      break;

    case GOUCT_NAKADE:
      if (hasLastMove)
        GenerateNakadeMove();
      // TODO? needed? move_info.MakeUnique();
      break;

    case GOUCT_ATARI_CAPTURE:
      if (hasLastMove)
        GenerateAtariCaptureMove();
      break;

    case GOUCT_ATARI_DEFEND:
      if (hasLastMove)
        GenerateAtariDefenseMove();
      break;

    case GOUCT_LOWLIB:
      if (hasLastMove)
        GenerateLowLibMove(m_lastMove);
      break;

    case GOUCT_PATTERN:
      if (hasLastMove)
        GeneratePatternMove();
      break;

    case GOUCT_CAPTURE:m_captureGenerator.Generate(m_moves);
      break;

    case GOUCT_RANDOM:m_moves = AllRandomMoves();
      break;

    default:DBG_ASSERT(false); // not implemented
  }
  EndPlayout();
  return m_moves;
}

template<class BOARD>
GoUctPlayoutPolicyType GoUctPlayoutPolicy<BOARD>::MoveType() const {
  return m_moveType;
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::OnPlay() {
  m_captureGenerator.OnPlay();
  m_pureRandomGenerator.OnPlay();
}

template<class BOARD>
const GoUctGammaMoveGenerator<BOARD> &
GoUctPlayoutPolicy<BOARD>::GammaGenerator() const {
  return m_gammaGenerator;
}

template<class BOARD>
inline GoPoint GoUctPlayoutPolicy<BOARD>::SelectRandom() {
  return GoUctUtil::SelectRandom(m_bd, m_bd.ToPlay(), m_moves, m_random);
}

template<class BOARD>
const GoUctPlayoutPolicyStat &
GoUctPlayoutPolicy<BOARD>::Statistics(SgBlackWhite color) const {
  return m_statistics[color];
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::StartPlayout() {
  m_captureGenerator.StartPlayout();
  m_pureRandomGenerator.Start();
  m_nonRandLen = 0;
}

template<class BOARD>
void GoUctPlayoutPolicy<BOARD>::UpdateStatistics() {
  GoUctPlayoutPolicyStat &statistics = m_statistics[m_bd.ToPlay()];
  ++statistics.m_nuMoves;
  ++statistics.m_nuMoveType[m_moveType];
  if (m_moveType == GOUCT_RANDOM) {
    if (m_nonRandLen > 0) {
      statistics.m_nonRandLen.Add(float(m_nonRandLen));
      m_nonRandLen = 0;
    }
  } else {
    ++m_nonRandLen;
    statistics.m_moveListLen.Add(float(GetEquivalentBestMoves().Length()));
  }
}

template<class BOARD>
class GoUctPlayoutPolicyFactory {
 public:
  explicit GoUctPlayoutPolicyFactory(const GoUctPlayoutPolicyParam &param);
  GoUctPlayoutPolicy<BOARD> *Create(const BOARD &bd);

 private:
  const GoUctPlayoutPolicyParam &m_param;
};

template<class BOARD>
GoUctPlayoutPolicyFactory<BOARD>
::GoUctPlayoutPolicyFactory(const GoUctPlayoutPolicyParam &param)
    : m_param(param) {}

template<class BOARD>
GoUctPlayoutPolicy<BOARD> *
GoUctPlayoutPolicyFactory<BOARD>::Create(const BOARD &bd) {
  return new GoUctPlayoutPolicy<BOARD>(bd, m_param);
}

#endif // GOUCT_PLAYOUTPOLICY_H
