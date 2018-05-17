
#include "platform/SgSystem.h"
#include "GoUctUtil.h"

#include <iomanip>
#include <iostream>
#include <boost/io/ios_state.hpp>
#include <boost/format.hpp>
#include "board/GoBWSet.h"
#include "board/GoPointSet.h"
#include "SgProp.h"
#include "UctSearch.h"

using boost::format;
using GoPointUtil::ToString;
using GoPointUtil::Pt;
using SgPropUtil::PointToSgfString;
using std::fixed;
using std::ostream;
using std::setprecision;
using std::vector;

namespace {
bool IsRectEmpty(const GoBoard &bd, int left, int right, int top, int bottom) {
  for (SgRectIterator it(GoRect(left, right, top, bottom)); it; ++it)
    if (!bd.IsEmpty(*it))
      return false;
  return true;
}

void SaveNode(std::ostream &out, const UctSearchTree &tree, const UctNode &node,
              SgBlackWhite toPlay, int boardSize, int maxDepth, int depth) {
  if (!node.HasMean())
    return;

  out << "C[MoveCount " << node.MoveCount()
      << "\nPosCount " << node.PosCount()
      << "\nMean " << fixed << setprecision(2) << node.Mean()
      << "\n" << node.ProvenType();
  if (!node.HasChildren()) {
    out << "]\n";
    return;
  }
  out << "]\nLB";
  for (UctChildNodeIterator it(tree, node); it; ++it) {
    const UctNode &child = *it;
    if (!child.HasMean())
      continue;
    out << "["
        << PointToSgfString(child.Move(), boardSize, SG_PROPPOINTFMT_GO)
        << ':' << child.MoveCount() << ']';
  }
  out << '\n';
  if (maxDepth >= 0 && depth >= maxDepth)
    return;
  for (UctChildNodeIterator it(tree, node); it; ++it) {
    const UctNode &child = *it;
    if (!child.HasMean())
      continue;
    GoPoint move = child.Move();
    out << "(;" << (toPlay == SG_BLACK ? 'B' : 'W') << '['
        << PointToSgfString(move, boardSize, SG_PROPPOINTFMT_GO) << ']';
    SaveNode(out, tree, child, SgOppBW(toPlay), boardSize, maxDepth,
             depth + 1);
    out << ")\n";
  }
}

} // namespace

void GoUctUtil::ClearStatistics(GoPointArray<UctStatistics> &stats) {
  for (GoPointArray<UctStatistics>::NonConstIterator
           it(stats); it; ++it)
    (*it).Clear();
}

GoPoint GoUctUtil::GenForcedOpeningMove(const GoBoard &bd) {
  int sz = bd.Size();
  if (sz < 13 || bd.TotalNumStones(SG_BLACK) > 5
      || bd.TotalNumStones(SG_WHITE) > 5)
    return GO_NULLMOVE;
  GoArrayList<GoPoint, 4> moves;
  if (IsRectEmpty(bd, 1, 5, 1, 5))
    moves.PushBack(Pt(4, 4));
  if (IsRectEmpty(bd, 1, 5, sz - 4, sz))
    moves.PushBack(Pt(4, sz - 3));
  if (IsRectEmpty(bd, sz - 4, sz, 1, 5))
    moves.PushBack(Pt(sz - 3, 4));
  if (IsRectEmpty(bd, sz - 4, sz, sz - 4, sz))
    moves.PushBack(Pt(sz - 3, sz - 3));
  if (moves.IsEmpty())
    return GO_NULLMOVE;
  return moves[0];
}

void GoUctUtil::GfxBestMove(const UctSearch &search, SgBlackWhite toPlay,
                            ostream &out) {
  const UctSearchTree &tree = search.Tree();
  const UctNode &root = tree.Root();
  out << "VAR";
  const UctNode *bestValueChild = search.FindBestChild(root);
  if (bestValueChild != 0) {
    GoPoint move = bestValueChild->Move();
    out << ' ' << (toPlay == SG_BLACK ? 'B' : 'W') << ' '
        << GoWritePoint(move);
  }
  out << '\n';
}

void GoUctUtil::GfxCounts(const UctSearchTree &tree, ostream &out) {
  const UctNode &root = tree.Root();
  out << "LABEL";
  if (root.HasChildren())
    for (UctChildNodeIterator it(tree, root); it; ++it) {
      const UctNode &child = *it;
      if (child.HasMean())
        out << (format(" %s %.0f")
            % ToString(child.Move()) % child.MoveCount());
    }
  out << '\n';
}

void GoUctUtil::GfxMoveValues(const UctSearch &search, SgBlackWhite toPlay,
                              ostream &out) {
  const UctSearchTree &tree = search.Tree();
  const UctNode &root = tree.Root();
  out << "INFLUENCE";
  if (root.HasChildren())
    for (UctChildNodeIterator it(tree, root); it; ++it) {
      const UctNode &child = *it;
      if (!child.HasMean())
        continue;
      UctValueType value = UctSearch::InverseEval(child.Mean());
      // Scale to [-1,+1], black positive
      UctValueType influence = value * 2 - 1;
      if (toPlay == SG_WHITE)
        influence *= -1;
      GoPoint move = child.Move();
      out << ' ' << GoWritePoint(move) << ' ' << fixed
          << setprecision(2) << influence;
    }
  out << '\n';
}

void GoUctUtil::GfxSequence(const UctSearch &search, SgBlackWhite toPlay,
                            ostream &out) {
  vector<GoMove> sequence;
  search.FindBestSequence(sequence);
  out << "VAR";
  for (GoMove move : sequence) {
    out << (toPlay == SG_BLACK ? " B " : " W ")
        << GoWritePoint(move);
    toPlay = SgOppBW(toPlay);
  }
  out << '\n';
}

void GoUctUtil::GfxStatus(const UctSearch &search, ostream &out) {
  const UctSearchTree &tree = search.Tree();
  const UctNode &root = tree.Root();
  const UctSearchStat &stat = search.Statistics();
  UctValueType abortPercent = stat.search_aborted.Mean() * UctValueType(100);
  out << (format(
      "TEXT N=%.0f V=%.2f Len=%.0f Tree=%.1f/%.1f Abrt=%.0f%% Gm/s=%.0f\n")
      % root.MoveCount() % root.Mean() % stat.game_length.Mean()
      % stat.moves_in_tree.Mean() % stat.moves_in_tree.Max()
      % abortPercent % stat.searches_per_second);
}

void GoUctUtil::GfxTerritoryStatistics(
    const GoPointArray<UctStatistics> &territoryStatistics,
    const GoBoard &bd, std::ostream &out) {
  boost::io::ios_all_saver saver(out);
  out << fixed << setprecision(3) << "INFLUENCE";
  for (GoBoard::Iterator it(bd); it; ++it)
    if (territoryStatistics[*it].Count() > 0)
      // Scale to [-1,+1], black positive
      out << ' ' << GoWritePoint(*it) << ' '
          << territoryStatistics[*it].Mean() * 2 - 1;
  out << '\n';
}

void GoUctUtil::SaveTree(const UctSearchTree &tree, int boardSize,
                         const GoBWSet &stones, SgBlackWhite toPlay,
                         ostream &out, int maxDepth) {
  out << "(;FF[4]GM[1]SZ[" << boardSize << "]\n";
  for (SgBWIterator itColor; itColor; ++itColor) {
    const GoPointSet &stonesColor = stones[*itColor];
    if (stonesColor.Size() == 0)
      continue;
    out << ((*itColor) == SG_BLACK ? "AB" : "AW");
    for (SgSetIterator it(stonesColor); it; ++it)
      out << '[' << PointToSgfString(*it, boardSize, SG_PROPPOINTFMT_GO)
          << ']';
    out << '\n';
  }
  out << "PL[" << (toPlay == SG_BLACK ? "B" : "W") << "]\n";
  if (tree.Root().HasMean())
    SaveNode(out, tree, tree.Root(), toPlay, boardSize, maxDepth, 0);
  out << ")\n";
}

namespace {
bool IsMeanLess(const UctNode *lhs, const UctNode *rhs) {
  return (lhs->Mean() < rhs->Mean());
}
} // namespace

std::string GoUctUtil::ChildrenStatistics(const UctSearch &search,
                                          bool bSort, const UctNode &node) {
  std::ostringstream out;
  vector<const UctNode *> vec;
  const UctSearchTree &tree = search.Tree();
  for (UctChildNodeIterator it(tree, node); it; ++it) {
    const UctNode &child = *it;
    vec.push_back(&child);
  }
  if (bSort)
    sort(vec.begin(), vec.end(), IsMeanLess);
  for (auto it = vec.begin(); it != vec.end();
       ++it) {
    const UctNode &child = **it;
    out << search.MoveString(child.Move()) << " -" << " value="
        << child.Mean() << " count=" << child.MoveCount() << '\n';
  }
  return out.str();
}