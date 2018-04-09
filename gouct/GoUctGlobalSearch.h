#ifndef GOUCT_GLOBALSEARCH_H
#define GOUCT_GLOBALSEARCH_H

#include <cstdlib>
#include <limits>
#include <boost/scoped_ptr.hpp>
#include <boost/version.hpp>
#include <GoUtil.h>
#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoEyeUtil.h"
#include "GoRegionBoard.h"
#include "GoSafetySolver.h"
#include "GoUctDefaultMoveFilter.h"
#include "GoUctSearch.h"
#include "GoUctUtil.h"
#include "board/GoPoint.h"
#include "GoUctPlayoutPolicy.h"

const bool GOUCT_USE_SAFETY_SOLVER = false;
struct GoUctGlobalSearchStateParam {
  bool m_mercyRule;
  bool m_territoryStatistics;
  UctValueType m_lengthModification;
  UctValueType m_scoreModification;
  bool m_useTreeFilter;
  bool m_useDefaultPriorKnowledge;
  float m_defaultPriorWeight;
  float m_additiveKnowledgeScale;
  GoUctGlobalSearchStateParam();
  ~GoUctGlobalSearchStateParam();
};
struct GoUctGlobalSearchAllParam {
  GoUctGlobalSearchAllParam(
      const GoUctGlobalSearchStateParam &searchStateParam,
      const GoUctPlayoutPolicyParam &policyParam,
      const GoUctDefaultMoveFilterParam &moveFilterParam,
      const GoBWSet &safe,
      const GoPointArray<bool> &allSafe);
  const GoUctGlobalSearchStateParam &m_searchStateParam;
  const GoUctPlayoutPolicyParam &m_policyParam;
  const GoUctDefaultMoveFilterParam &m_moveFilterParam;
  const GoBWSet &m_safe;
  const GoPointArray<bool> &m_allSafe;
};
template<class POLICY>
class GoUctGlobalSearchState
    : public GoUctState {
 public:

  GoPointArray<UctStatistics> m_territoryStatistics;
  GoUctGlobalSearchState(unsigned int threadId, const GoBoard &bd,
                         POLICY *policy,
                         const GoUctGlobalSearchAllParam &param);
  ~GoUctGlobalSearchState();
  UctValueType Evaluate();
  UctValueType FinalScore();
  bool WinTheGame();
  bool TrompTaylorPassWins();
  void CollectFeatures(char feature[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures);
  void CollectFeatures(char feature[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures, int maxSteps);
  bool GenerateAllMoves(UctValueType count, std::vector<UctMoveInfo> &moves,
                        UctProvenType &provenType);
  void GenerateLegalMoves(std::vector<UctMoveInfo> &moves);
  GoMove GeneratePlayoutMove(bool &skipRaveUpdate);
  void ExecutePlayout(GoMove move);
  void GameStart();
  void EndPlayout();
  void StartPlayout();
  void StartPlayouts();
  void StartSearch();
  void Clear();
  POLICY *Policy();
  void SetPolicy(POLICY *policy);
  void ClearTerritoryStatistics();

 private:
  const GoUctGlobalSearchAllParam m_param;
  bool m_swapMoves;
  bool m_mercyRuleTriggered;
  int m_passMovesPlayoutPhase;
  int m_mercyRuleThreshold;
  int m_stoneDiff;
  int m_initialMoveNumber;
  GoPointList m_area;
  UctValueType m_mercyRuleResult;
  UctValueType m_invMaxScore;
  SgRandom m_random;
  boost::scoped_ptr<POLICY> m_policy;
  GoUctDefaultMoveFilter m_treeFilter;
  GoUctGlobalSearchState(const GoUctGlobalSearchState &search);
  GoUctGlobalSearchState &operator=(const GoUctGlobalSearchState &search);
  void ApplyFilter(std::vector<UctMoveInfo> &moves);
  bool CheckMercyRule();
  template<class BOARD>
  UctValueType EvaluateBoard(const BOARD &bd, float komi);
  float GetKomi() const;
  const GoBWSet &Safe() const;
  bool AllSafe(GoPoint p) const;
};

template<class POLICY>
GoUctGlobalSearchState<POLICY>::GoUctGlobalSearchState(unsigned int threadId,
                                                       const GoBoard &bd, POLICY *policy,
                                                       const GoUctGlobalSearchAllParam &param)
    : GoUctState(threadId, bd),
      m_param(param),
      m_swapMoves(true),
      m_policy(policy),
      m_treeFilter(Board(), m_param.m_moveFilterParam) {
  ClearTerritoryStatistics();
}

template<class POLICY>
GoUctGlobalSearchState<POLICY>::~GoUctGlobalSearchState() {
}

template<class POLICY>
bool GoUctGlobalSearchState<POLICY>::CheckMercyRule() {
  DBG_ASSERT(m_param.m_searchStateParam.m_mercyRule);
  DBG_ASSERT(IsInPlayout());
  if (m_stoneDiff >= m_mercyRuleThreshold) {
    m_mercyRuleTriggered = true;
    m_mercyRuleResult = (UctBoard().ToPlay() == SG_BLACK ? 1 : 0);
  } else if (m_stoneDiff <= -m_mercyRuleThreshold) {
    m_mercyRuleTriggered = true;
    m_mercyRuleResult = (UctBoard().ToPlay() == SG_WHITE ? 1 : 0);
  } else
    DBG_ASSERT(!m_mercyRuleTriggered);
  return m_mercyRuleTriggered;
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::ApplyFilter(std::vector<UctMoveInfo> &moves) {
  std::vector<GoPoint> filtered = m_treeFilter.Get();
  if (filtered.size() > 0) {
    std::vector<UctMoveInfo> filteredMoves;
    for (std::vector<UctMoveInfo>::const_iterator it = moves.begin();
         it != moves.end(); ++it)
      if (find(filtered.begin(), filtered.end(), it->uct_move)
          == filtered.end())
        filteredMoves.push_back(*it);
    moves = filteredMoves;
  }
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::ClearTerritoryStatistics() {
  for (GoPointArray<UctStatistics>::NonConstIterator
           it(m_territoryStatistics); it; ++it)
    (*it).Clear();
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::EndPlayout() {
  GoUctState::EndPlayout();
  m_policy->EndPlayout();
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::CollectFeatures(char features[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures) {
  const GoBoard &bd = Board();
  std::stack<GoMove> sequence;
  SgBlackWhite currentPlayer = SgOppBW(bd.ToPlay());
  for (int i = 0; i < numFeatures; i++) {
    for (GoBoard::Iterator it(bd); it; ++it) {
      GoPoint p = *it;
      if (bd.GetColor(p) == currentPlayer)
        features[i][GoPointUtil::Row(p) - 1][GoPointUtil::Col(p) - 1] = 1;
      else
        features[i][GoPointUtil::Row(p) - 1][GoPointUtil::Col(p) - 1] = 0;
    }
    GoMove lastMove = bd.GetLastMove();
    if (lastMove != GO_NULLMOVE) {
      sequence.push(lastMove);
      TakeBackInTree(1);
    }
  }
  memset(features[numFeatures - 1], 1 - currentPlayer, (size_t) GO_MAX_ONBOARD);

  while (!sequence.empty()) {
    GoMove move = sequence.top();
    Execute(move);
    sequence.pop();
  }
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::CollectFeatures(char features[][GO_MAX_SIZE][GO_MAX_SIZE], int numFeatures,
                                                     int maxSteps) {
  const GoBoard &bd = Board();
  std::stack<GoMove> sequence;
  SgBlackWhite currentPlayer = SgOppBW(bd.ToPlay());
  for (int i = 0; i < numFeatures; i++) {
    if (i < maxSteps) {
      for (GoBoard::Iterator it(bd); it; ++it) {
        GoPoint p = *it;
        if (bd.GetColor(p) == currentPlayer)
          features[i][GoPointUtil::Row(p) - 1][GoPointUtil::Col(p) - 1] = 1;
        else
          features[i][GoPointUtil::Row(p) - 1][GoPointUtil::Col(p) - 1] = 0;
      }
      GoMove lastMove = bd.GetLastMove();
      if (lastMove != GO_NULLMOVE) {
        sequence.push(lastMove);
        TakeBackInTree(1);
      }
    } else
      memset(features[i], 0, (size_t) GO_MAX_ONBOARD);
  }
  memset(features[numFeatures - 1], 1 - currentPlayer, (size_t) GO_MAX_ONBOARD);

  while (!sequence.empty()) {
    GoMove move = sequence.top();
    Execute(move);
    sequence.pop();
  }
}

template<class POLICY>
UctValueType GoUctGlobalSearchState<POLICY>::Evaluate() {
  float komi = GetKomi();
  if (IsInPlayout())
    return EvaluateBoard(UctBoard(), komi);
  else
    return EvaluateBoard(Board(), komi);
}

template<class POLICY>
UctValueType GoUctGlobalSearchState<POLICY>::FinalScore() {
  float komi = GetKomi();
  return GoBoardUtil::TrompTaylorScore(Board(), komi);
}

template<class POLICY>
bool GoUctGlobalSearchState<POLICY>::WinTheGame() {
  float komi = GetKomi();
  const GoBoard &bd = Board();
  float score = GoBoardUtil::TrompTaylorScore(bd, komi);
  std::ostringstream stream;
  stream << "\n" << bd;
  std::cout << stream.str();
  std::cout << GoUtil::ScoreToString(score) << std::endl;
  if (bd.ToPlay() != SG_BLACK)
    score *= -1;
  return score > 0;
}

template<class POLICY>
bool GoUctGlobalSearchState<POLICY>::TrompTaylorPassWins() {
  const GoBoard &bd = Board();
  return GoBoardUtil::TrompTaylorPassWins(bd, bd.ToPlay());
}

template<class POLICY>
template<class BOARD>
UctValueType GoUctGlobalSearchState<POLICY>::EvaluateBoard(const BOARD &bd, float komi) {
  UctValueType score;
  GoPointArray<SgEmptyBlackWhite> scoreBoard;
  GoPointArray<SgEmptyBlackWhite> *scoreBoardPtr;
  const GoUctGlobalSearchStateParam &param = m_param.m_searchStateParam;
  if (param.m_territoryStatistics)
    scoreBoardPtr = &scoreBoard;
  else
    scoreBoardPtr = 0;
  if (param.m_mercyRule && m_mercyRuleTriggered)
    return m_mercyRuleResult;
  else if (m_passMovesPlayoutPhase < 2)
    score = UctValueType(GoBoardUtil::TrompTaylorScore(bd, komi, scoreBoardPtr));
  else {
    score = UctValueType(GoBoardUtil::ScoreSimpleEndPosition(bd, komi, Safe(), false, scoreBoardPtr));
  }
  if (param.m_territoryStatistics) {
    for (typename BOARD::Iterator it(bd); it; ++it)
      switch (scoreBoard[*it]) {
        case SG_BLACK:m_territoryStatistics[*it].Add(1);
          break;
        case SG_WHITE:m_territoryStatistics[*it].Add(0);
          break;
        case SG_EMPTY:m_territoryStatistics[*it].Add(0.5);
          break;
      }
  }
  if (bd.ToPlay() != SG_BLACK)
    score *= -1;
  UctValueType lengthMod = UctValueType(GameLength()) * param.m_lengthModification;
  if (lengthMod > 0.5)
    lengthMod = 0.5;
  if (score > std::numeric_limits<UctValueType>::epsilon())
    return (1 - param.m_scoreModification)
        + param.m_scoreModification * score * m_invMaxScore
        - lengthMod;
  else if (score < -std::numeric_limits<UctValueType>::epsilon())
    return param.m_scoreModification
        + param.m_scoreModification * score * m_invMaxScore
        + lengthMod;
  else
    return 0.5;
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::ExecutePlayout(GoMove move) {
  GoUctState::ExecutePlayout(move);
  const GoUctBoard &bd = UctBoard();
  if (bd.ToPlay() == SG_BLACK)
    m_stoneDiff -= bd.NuCapturedStones();
  else
    m_stoneDiff += bd.NuCapturedStones();
  m_policy->OnPlay();
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::GameStart() {
  GoUctState::GameStart();
  m_passMovesPlayoutPhase = 0;
  m_mercyRuleTriggered = false;
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::GenerateLegalMoves(std::vector<UctMoveInfo> &moves) {
  const GoBoard &bd = Board();
  DBG_ASSERT(!bd.Rules().AllowSuicide());

  if (GoBoardUtil::TwoPasses(bd))
    if (bd.Rules().CaptureDead()
        || bd.MoveNumber() - m_initialMoveNumber >= 2)
      return;

  SgBlackWhite toPlay = bd.ToPlay();
  for (GoBoard::Iterator it(bd); it; ++it) {
    GoPoint p = *it;
    if (bd.IsEmpty(p)
        && !GoEyeUtil::IsSimpleEye(bd, p, toPlay)
        && !AllSafe(p)
        && bd.IsLegal(p, toPlay)
        )
      moves.push_back(UctMoveInfo(p));
  }
  if (m_swapMoves && moves.size() > 1)
    std::swap(moves[0], moves[m_random.SmallInt(moves.size())]);
  moves.push_back(UctMoveInfo(GO_PASS));
}

inline float invsqrt(float value) {
  return 1 / sqrtf(value);
}

template<class POLICY>
bool GoUctGlobalSearchState<POLICY>::
GenerateAllMoves(UctValueType count,
                 std::vector<UctMoveInfo> &moves,
                 UctProvenType &provenType) {
  const GoUctGlobalSearchStateParam &param = m_param.m_searchStateParam;
  provenType = PROVEN_NONE;
  moves.clear();
  GenerateLegalMoves(moves);
  if (!moves.empty() && count == 0) {
    if (param.m_useTreeFilter)
      ApplyFilter(moves);
#ifdef USE_KNOWLEDGE
    if (  feParam.m_priorKnowledgeType != PRIOR_NONE
       || feParam.m_useAsAdditivePredictor
       )
    {
        DBG_ASSERT(m_featureKnowledge);
        m_featureKnowledge->Compute(feParam);
        if (feParam.m_priorKnowledgeType != PRIOR_NONE)
            m_featureKnowledge->SetPriorKnowledge(moves);
    }
    ApplyAdditivePredictors(moves);
#endif
  }
  return false;
}

template<class POLICY>
GoMove GoUctGlobalSearchState<POLICY>::GeneratePlayoutMove(bool &skipRaveUpdate) {
  DBG_ASSERT(IsInPlayout());
  if (m_param.m_searchStateParam.m_mercyRule && CheckMercyRule())
    return GO_NULLMOVE;
  GoPoint move = m_policy->GenerateMove();
  DBG_ASSERT(move != GO_NULLMOVE);
#ifndef NDEBUG
  if (move == GO_PASS) {
    const GoUctBoard &bd = UctBoard();
    for (GoUctBoard::Iterator it(bd); it; ++it)
      DBG_ASSERT(bd.Occupied(*it)
                || Safe().OneContains(*it)
                || GoBoardUtil::SelfAtari(bd, *it)
                || !GoUctUtil::GeneratePoint(bd, *it, bd.ToPlay())
      );
  } else
    DBG_ASSERT(!Safe().OneContains(move));
#endif
  if (move == GO_PASS) {
    skipRaveUpdate = true;
    if (m_passMovesPlayoutPhase < 2)
      ++m_passMovesPlayoutPhase;
    else
      return GO_NULLMOVE;
  } else
    m_passMovesPlayoutPhase = 0;
  return move;
}

template<class POLICY>
float GoUctGlobalSearchState<POLICY>::GetKomi() const {
  const GoRules &rules = Board().Rules();
  float komi = rules.Komi().ToFloat();
  if (rules.ExtraHandicapKomi())
    komi += float(rules.Handicap());
  return komi;
}

template<class POLICY>
inline POLICY *GoUctGlobalSearchState<POLICY>::Policy() {
  return m_policy.get();
}

template<class POLICY>
const GoBWSet &GoUctGlobalSearchState<POLICY>::Safe() const {
  return m_param.m_safe;
}

template<class POLICY>
bool GoUctGlobalSearchState<POLICY>::AllSafe(GoPoint p) const {
  return m_param.m_allSafe[p];
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::SetPolicy(POLICY *policy) {
  m_policy.reset(policy);
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::StartPlayout() {
  GoUctState::StartPlayout();
  m_passMovesPlayoutPhase = 0;
  m_mercyRuleTriggered = false;
  const GoBoard &bd = Board();
  m_stoneDiff = bd.All(SG_BLACK).Size() - bd.All(SG_WHITE).Size();
  m_policy->StartPlayout();
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::StartPlayouts() {
  GoUctState::StartPlayouts();
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::StartSearch() {
  GoUctState::StartSearch();
  const GoBoard &bd = Board();
  const int size = bd.Size();
  const float maxScore = float(size * size) + std::abs(GetKomi());
  m_invMaxScore = UctValueType(1 / maxScore);
  m_initialMoveNumber = bd.MoveNumber();
  m_mercyRuleThreshold = static_cast<int>(0.3 * size * size);
  ClearTerritoryStatistics();
}

template<class POLICY>
void GoUctGlobalSearchState<POLICY>::Clear() {
  GoUctState::Clear();
}

template<class POLICY, class FACTORY>
class GoUctGlobalSearchStateFactory
    : public UctThreadStateFactory {
 public:

  GoUctGlobalSearchStateFactory(GoBoard &bd,
                                FACTORY &playoutPolicyFactory,
                                const GoUctPlayoutPolicyParam &policyParam,
                                const GoUctDefaultMoveFilterParam &treeFilterParam,
                                const GoBWSet &safe,
                                const GoPointArray<bool> &allSafe);
  UctThreadState *Create(unsigned int threadId,
                         const UctSearch &search);

 private:
  GoBoard &m_bd;
  FACTORY &m_playoutPolicyFactory;

#if USE_KNOWLEDGE
  GoUctKnowledgeFactory m_knowledgeFactory;
#endif

  const GoUctPlayoutPolicyParam &m_policyParam;
  const GoUctDefaultMoveFilterParam &m_treeFilterParam;
  const GoBWSet &m_safe;
  const GoPointArray<bool> &m_allSafe;
};

template<class POLICY, class FACTORY>
GoUctGlobalSearchStateFactory<POLICY, FACTORY>
::GoUctGlobalSearchStateFactory(GoBoard &bd,
                                FACTORY &playoutPolicyFactory,
                                const GoUctPlayoutPolicyParam &policyParam,
                                const GoUctDefaultMoveFilterParam &treeFilterParam,
                                const GoBWSet &safe,
                                const GoPointArray<bool> &allSafe)
    : m_bd(bd),
      m_playoutPolicyFactory(playoutPolicyFactory),
#if USE_KNOWLEDGE
    m_knowledgeFactory(policyParam),
#endif
      m_policyParam(policyParam),
      m_treeFilterParam(treeFilterParam),
      m_safe(safe),
      m_allSafe(allSafe) {}

template<class POLICY, class FACTORY>
class GoUctGlobalSearch
    : public GoUctSearch {
 public:
  GoUctGlobalSearchStateParam m_param;
  GoUctGlobalSearch(GoBoard &bd,
                    FACTORY *playoutPolicyFactory,
                    const GoUctPlayoutPolicyParam &policyParam,
                    const GoUctDefaultMoveFilterParam &treeFilterParam);
  UctValueType UnknownEval() const;
  void OnStartSearch();
  void DisplayGfx();
  void SetDefaultParameters(int boardSize);
  bool GlobalSearchLiveGfx() const;
  void SetGlobalSearchLiveGfx(bool enable);

 private:
  GoBWSet m_safe;
  GoPointArray<bool> m_allSafe;
  boost::scoped_ptr<FACTORY> m_playoutPolicyFactory;
  GoRegionBoard m_regions;
  bool m_globalSearchLiveGfx;
};

template<class POLICY, class FACTORY>
GoUctGlobalSearch<POLICY, FACTORY>::GoUctGlobalSearch(GoBoard &bd,
                                                      FACTORY *playoutFactory,
                                                      const GoUctPlayoutPolicyParam &policyParam,
                                                      const GoUctDefaultMoveFilterParam &rootFilterParam)
    : GoUctSearch(bd, 0),
      m_playoutPolicyFactory(playoutFactory),
      m_regions(bd),
      m_globalSearchLiveGfx(GOUCT_LIVEGFX_NONE) {
  UctThreadStateFactory *stateFactory =
      new GoUctGlobalSearchStateFactory<POLICY, FACTORY>(bd,
                                                         *playoutFactory,
                                                         policyParam,
                                                         rootFilterParam,
                                                         m_safe, m_allSafe);
  SetThreadStateFactory(stateFactory);
  SetDefaultParameters(bd.Size());
  if (LockFree()) {
    unsigned int nuThreads = boost::thread::hardware_concurrency();
#ifdef LIMIT_THREADS_TO_4
    if (nuThreads > 4)
        nuThreads = 4;
#endif
    if (nuThreads > MAX_BATCHES)
      nuThreads = MAX_BATCHES;
    SetThreadsNumberOnly(nuThreads);
  }
}

template<class POLICY, class FACTORY>
inline bool GoUctGlobalSearch<POLICY, FACTORY>::GlobalSearchLiveGfx() const {
  return m_globalSearchLiveGfx;
}

template<class POLICY, class FACTORY>
void GoUctGlobalSearch<POLICY, FACTORY>::DisplayGfx() {
  GoUctSearch::DisplayGfx();
  if (m_globalSearchLiveGfx) {
    const GoUctGlobalSearchState<POLICY> &state =
        dynamic_cast<GoUctGlobalSearchState<POLICY> &>(ThreadState(0));
    SgDebug() << "gogui-gfx:\n";
    GoUctUtil::GfxBestMove(*this, ToPlay(), SgDebug());
    GoUctUtil::GfxTerritoryStatistics(state.m_territoryStatistics,
                                      Board(), SgDebug());
    GoUctUtil::GfxStatus(*this, SgDebug());
    SgDebug() << '\n';
  }
}

template<class POLICY, class FACTORY>
void GoUctGlobalSearch<POLICY, FACTORY>::OnStartSearch() {
  GoUctSearch::OnStartSearch();
  m_safe.Clear();
  m_allSafe.Fill(false);
  if (GOUCT_USE_SAFETY_SOLVER) {
    const GoBoard &bd = Board();
    GoSafetySolver solver(bd, &m_regions);
    solver.FindSafePoints(&m_safe);
    for (GoBoard::Iterator it(bd); it; ++it)
      m_allSafe[*it] = m_safe.OneContains(*it);
  }
  if (m_globalSearchLiveGfx && !m_param.m_territoryStatistics)
    SgWarning() <<
                "GoUctGlobalSearch: "
                    "live graphics need territory statistics enabled\n";
}

template<class POLICY, class FACTORY>
void GoUctGlobalSearch<POLICY, FACTORY>::SetDefaultParameters(int boardSize) {
  SetFirstPlayUrgency(1);
  SetMoveSelect(SG_UCTMOVESELECT_COUNT);
  SetRave(true);
  SetExpandThreshold(std::numeric_limits<UctValueType>::is_integer ?
                     UctValueType(1) :
                     std::numeric_limits<UctValueType>::epsilon());
  SetVirtualLoss(true);
  SetBiasTermConstant(0.0);
  SetExpandThreshold(3);
  if (boardSize < 15) {
    SetRaveWeightInitial(1.0);
    SetRaveWeightFinal(5000);
    m_param.m_lengthModification = 0;
  } else {
    SetRaveWeightInitial(0.9f);
    SetRaveWeightFinal(5000);
    m_param.m_lengthModification = 0.00028f;
  }
}

template<class POLICY, class FACTORY>
inline void GoUctGlobalSearch<POLICY, FACTORY>::SetGlobalSearchLiveGfx(
    bool enable) {
  m_globalSearchLiveGfx = enable;
}

template<class POLICY, class FACTORY>
UctValueType GoUctGlobalSearch<POLICY, FACTORY>::UnknownEval() const {
  return UctValueType(0.5);
}

template<class POLICY, class FACTORY>
UctThreadState *GoUctGlobalSearchStateFactory<POLICY, FACTORY>::Create(
    unsigned int threadId, const UctSearch &search) {
  const GoUctGlobalSearch<POLICY, FACTORY> &globalSearch =
      dynamic_cast<const GoUctGlobalSearch<POLICY, FACTORY> &>(search);
  const GoBoard &bd = globalSearch.Board();
  GoUctGlobalSearchState<POLICY> *state =
      new GoUctGlobalSearchState<POLICY>(threadId, bd, 0,
                                         GoUctGlobalSearchAllParam(
                                             globalSearch.m_param,
                                             m_policyParam,
                                             m_treeFilterParam,
                                             m_safe, m_allSafe));
  POLICY *policy = m_playoutPolicyFactory.Create(state->UctBoard());
  state->SetPolicy(policy);
#ifdef USE_KNOWLEDGE
  GoAdditiveKnowledge* knowledge =
    m_knowledgeFactory.Create(state->Board());
  state->SetAdditiveKnowledge(knowledge);
  GoUctFeatureKnowledge* featureKnowledge =
      dynamic_cast<GoUctFeatureKnowledge*>(knowledge);
  if (! featureKnowledge)
      featureKnowledge =
      dynamic_cast<GoUctFeatureKnowledge*>(
          m_knowledgeFactory.CreateByType(state->Board(),
          KNOWLEDGE_FEATURES));
DBG_ASSERT(featureKnowledge);
  state->SetFeatureKnowledge(featureKnowledge);
#endif
  return state;
}

#endif
