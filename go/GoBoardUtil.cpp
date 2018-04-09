

#include "platform/SgSystem.h"
#include "GoBoardUtil.h"

#include <iomanip>
#include <sstream>
#include <string>
#include "GoBoard.h"
#include "GoModBoard.h"
#include "GoMoveExecutor.h"
#include "platform/SgDebug.h"
#include "board/GoNbIterator.h"
#include "SgProp.h"

using SgPropUtil::PointToSgfString;

namespace {

void CfgDistanceCheck(const GoBoard& bd, GoPointArray<int>& array,
                      GoPointList& pointList, int d, GoPoint p) {
  if (!bd.IsBorder(p)) {
    if (bd.Occupied(p))
      p = bd.Anchor(p);
    if (array[p] == std::numeric_limits<int>::max()) {
      array[p] = d;
      pointList.PushBack(p);
    }
  }
}

void ScorePositionRecurse(const GoBoard& bd, GoPoint p,
                          const GoPointSet& deadStones, SgMarker& marker,
                          bool& isBlackAdjacent, bool& isWhiteAdjacent,
                          int& nuPoints, int& nuDeadWhite, int& nuDeadBlack) {
  if (bd.IsBorder(p))
    return;
  SgEmptyBlackWhite c = bd.GetColor(p);
  if (c != SG_EMPTY && !deadStones.Contains(p)) {
    if (c == SG_BLACK)
      isBlackAdjacent = true;
    else
      isWhiteAdjacent = true;
    return;
  }
  if (!marker.NewMark(p))
    return;
  ++nuPoints;
  if (c == SG_BLACK)
    ++nuDeadBlack;
  if (c == SG_WHITE)
    ++nuDeadWhite;
  ScorePositionRecurse(bd, p + GO_NORTH_SOUTH, deadStones, marker, isBlackAdjacent,
                       isWhiteAdjacent, nuPoints, nuDeadWhite, nuDeadBlack);
  ScorePositionRecurse(bd, p - GO_NORTH_SOUTH, deadStones, marker, isBlackAdjacent,
                       isWhiteAdjacent, nuPoints, nuDeadWhite, nuDeadBlack);
  ScorePositionRecurse(bd, p + GO_WEST_EAST, deadStones, marker, isBlackAdjacent,
                       isWhiteAdjacent, nuPoints, nuDeadWhite, nuDeadBlack);
  ScorePositionRecurse(bd, p - GO_WEST_EAST, deadStones, marker, isBlackAdjacent,
                       isWhiteAdjacent, nuPoints, nuDeadWhite, nuDeadBlack);
}

}

void GoBoardUtil::AddNeighborBlocksOfColor(const GoBoard& bd, GoPoint p,
                                           SgBlackWhite c,
                                           SgVector<GoPoint>& neighbors) {
  if (bd.IsColor(p - GO_NORTH_SOUTH, c))
    neighbors.Include(bd.Anchor(p - GO_NORTH_SOUTH));
  if (bd.IsColor(p - GO_WEST_EAST, c))
    neighbors.Include(bd.Anchor(p - GO_WEST_EAST));
  if (bd.IsColor(p + GO_WEST_EAST, c))
    neighbors.Include(bd.Anchor(p + GO_WEST_EAST));
  if (bd.IsColor(p + GO_NORTH_SOUTH, c))
    neighbors.Include(bd.Anchor(p + GO_NORTH_SOUTH));
}

void GoBoardUtil::AddWall(GoBoard& bd,
                          SgBlackWhite color,
                          GoPoint start,
                          int length,
                          int direction) {
  for (GoPoint p = start; length > 0; --length) {
    bd.Play(p, color);
    p += direction;
  }
}

GoPointList GoBoardUtil::AdjacentStones(const GoBoard& bd, GoPoint point) {
  DBG_ASSERT(bd.IsValidPoint(point));
  DBG_ASSERT(bd.Occupied(point));
  const SgBlackWhite other = SgOppBW(bd.GetStone(point));
  GoPointList result;
  SgMarker& mark = bd.m_userMarker;
  SgReserveMarker reserve(mark);
  SuppressUnused(reserve);
  mark.Clear();
  for (GoBoard::StoneIterator it(bd, point); it; ++it) {
    if (bd.NumNeighbors(*it, other) > 0) {
      GoPoint p = *it;
      if (bd.IsColor(p - GO_NORTH_SOUTH, other) && mark.NewMark(p - GO_NORTH_SOUTH))
        result.PushBack(p - GO_NORTH_SOUTH);
      if (bd.IsColor(p - GO_WEST_EAST, other) && mark.NewMark(p - GO_WEST_EAST))
        result.PushBack(p - GO_WEST_EAST);
      if (bd.IsColor(p + GO_WEST_EAST, other) && mark.NewMark(p + GO_WEST_EAST))
        result.PushBack(p + GO_WEST_EAST);
      if (bd.IsColor(p + GO_NORTH_SOUTH, other) && mark.NewMark(p + GO_NORTH_SOUTH))
        result.PushBack(p + GO_NORTH_SOUTH);
    }
  };
  return result;
}

void GoBoardUtil::AdjacentBlocks(const GoBoard& bd, GoPoint p, int maxLib,
                                 SgVector<GoPoint>* blocks) {
  DBG_ASSERT(blocks);
  GoPoint a[GO_MAXPOINT];
  int n = bd.AdjacentBlocks(p, maxLib, a, GO_MAXPOINT);
  blocks->SetTo(a, n);
}

void GoBoardUtil::BlocksAdjacentToPoints(const GoBoard& bd,
                                         const SgVector<GoPoint>& points,
                                         SgBlackWhite c,
                                         SgVector<GoPoint>* blocks) {
  SgMarker& mark = bd.m_userMarker;
  SgReserveMarker reserve(mark);
  SuppressUnused(reserve);
  mark.Clear();
  for (SgVectorIterator<GoPoint> it1(points); it1; ++it1)
    mark.Include(*it1);
  GoPoint a[GO_MAXPOINT];
  int n = 0;
  for (SgVectorIterator<GoPoint> it2(points); it2; ++it2) {
    GoPoint p = *it2;
    if (bd.NumNeighbors(p, c) > 0) {
      if (bd.IsColor(p - GO_NORTH_SOUTH, c)
          && mark.NewMark(bd.Anchor(p - GO_NORTH_SOUTH)))
        a[n++] = bd.Anchor(p - GO_NORTH_SOUTH);
      if (bd.IsColor(p - GO_WEST_EAST, c)
          && mark.NewMark(bd.Anchor(p - GO_WEST_EAST)))
        a[n++] = bd.Anchor(p - GO_WEST_EAST);
      if (bd.IsColor(p + GO_WEST_EAST, c)
          && mark.NewMark(bd.Anchor(p + GO_WEST_EAST)))
        a[n++] = bd.Anchor(p + GO_WEST_EAST);
      if (bd.IsColor(p + GO_NORTH_SOUTH, c)
          && mark.NewMark(bd.Anchor(p + GO_NORTH_SOUTH)))
        a[n++] = bd.Anchor(p + GO_NORTH_SOUTH);
    }
  }
  blocks->SetTo(a, n);
}

void GoBoardUtil::BlocksAdjacentToPoints(const GoBoard& bd,
                                         const GoPointSet& points,
                                         SgBlackWhite c,
                                         SgVector<GoPoint>* blocks) {
  DBG_ASSERT(blocks);
  SgMarker& mark = bd.m_userMarker;
  SgReserveMarker reserve(mark);
  SuppressUnused(reserve);
  mark.Clear();
  for (SgSetIterator it1(points); it1; ++it1)
    mark.Include(*it1);
  GoPoint a[GO_MAXPOINT];
  int n = 0;
  for (SgSetIterator it2(points); it2; ++it2) {
    GoPoint p = *it2;
    if (bd.NumNeighbors(p, c) > 0) {
      if (bd.IsColor(p - GO_NORTH_SOUTH, c)
          && mark.NewMark(bd.Anchor(p - GO_NORTH_SOUTH)))
        a[n++] = bd.Anchor(p - GO_NORTH_SOUTH);
      if (bd.IsColor(p - GO_WEST_EAST, c)
          && mark.NewMark(bd.Anchor(p - GO_WEST_EAST)))
        a[n++] = bd.Anchor(p - GO_WEST_EAST);
      if (bd.IsColor(p + GO_WEST_EAST, c)
          && mark.NewMark(bd.Anchor(p + GO_WEST_EAST)))
        a[n++] = bd.Anchor(p + GO_WEST_EAST);
      if (bd.IsColor(p + GO_NORTH_SOUTH, c)
          && mark.NewMark(bd.Anchor(p + GO_NORTH_SOUTH)))
        a[n++] = bd.Anchor(p + GO_NORTH_SOUTH);
    }
  }
  blocks->SetTo(a, n);
}

bool GoBoardUtil::BlockIsAdjacentTo(const GoBoard& bd, GoPoint block,
                                    const GoPointSet& walls) {
  for (GoBoard::StoneIterator it(bd, block); it; ++it) {
    if (walls.Contains((*it) + GO_NORTH_SOUTH)
        || walls.Contains((*it) - GO_NORTH_SOUTH)
        || walls.Contains((*it) + GO_WEST_EAST)
        || walls.Contains((*it) - GO_WEST_EAST)
        )
      return true;
  }
  return false;
}

GoPointArray<int> GoBoardUtil::CfgDistance(const GoBoard& bd, GoPoint p,
                                           int maxDist) {
  GoPointArray<int> array(std::numeric_limits<int>::max());
  GoPointList pointList;
  if (bd.Occupied(p))
    p = bd.Anchor(p);
  pointList.PushBack(p);
  int begin = 0;
  int end = 1;
  int d = 0;
  array[p] = d;
  while (begin != end && d < maxDist) {
    ++d;
    for (int i = begin; i != end; ++i) {
      p = pointList[i];
      if (bd.Occupied(p)) {
        for (GoBoard::StoneIterator it(bd, p); it; ++it) {
          CfgDistanceCheck(bd, array, pointList, d, *it + GO_NORTH_SOUTH);
          CfgDistanceCheck(bd, array, pointList, d, *it - GO_NORTH_SOUTH);
          CfgDistanceCheck(bd, array, pointList, d, *it + GO_WEST_EAST);
          CfgDistanceCheck(bd, array, pointList, d, *it - GO_WEST_EAST);
        }
      } else {
        CfgDistanceCheck(bd, array, pointList, d, p + GO_NORTH_SOUTH);
        CfgDistanceCheck(bd, array, pointList, d, p - GO_NORTH_SOUTH);
        CfgDistanceCheck(bd, array, pointList, d, p + GO_WEST_EAST);
        CfgDistanceCheck(bd, array, pointList, d, p - GO_WEST_EAST);
      }
    }
    begin = end;
    end = pointList.Length();
  }
  return array;
}

void GoBoardUtil::DumpBoard(const GoBoard& bd, std::ostream& out) {
  const int size = bd.Size();
  out << bd;
  if (bd.MoveNumber() == 0)
    return;
  out << "(;SZ[" << size << "]\n";
  const GoSetup& setup = bd.Setup();
  if (!setup.IsEmpty()) {
    for (SgBWIterator it; it; ++it) {
      SgBlackWhite c = *it;
      int stoneNumber = 0;
      out << (c == SG_BLACK ? "AB" : "AW");
      for (SgSetIterator it2(setup.m_stones[c]); it2; ++it2) {
        GoPoint p = *it2;
        ++stoneNumber;
        out << '[' << PointToSgfString(p, size, SG_PROPPOINTFMT_GO)
            << ']';
        if (stoneNumber % 10 == 0)
          out << '\n';
      }
      out << '\n';
    }
    out << "PL[" << (setup.m_player == SG_BLACK ? 'B' : 'W') << "]\n";
  }
  int moveNumber = 0;
  for (int i = 0; i < bd.MoveNumber(); ++i) {
    GoPlayerMove move = bd.Move(i);
    out << ';';
    out << (move.Color() == SG_BLACK ? "B" : "W");
    ++moveNumber;
    out << '[' << PointToSgfString(move.Point(), size, SG_PROPPOINTFMT_GO)
        << ']';
    if (moveNumber % 10 == 0)
      out << '\n';
  }
  out << ")\n";
}

void GoBoardUtil::ExpandToBlocks(const GoBoard& board, GoPointSet& pointSet) {
  DBG_ASSERT(pointSet.SubsetOf(board.Occupied()));
  int size = board.Size();
  for (SgBlackWhite color = SG_BLACK; color <= SG_WHITE; ++color) {
    GoPointSet set = pointSet & board.All(color);
    bool change(true);
    while (change) {
      change = false;
      GoPointSet next = set | (set.Border(size) & board.All(color));
      if (next != set) {
        change = true;
        set = next;
      }
    }
    pointSet |= set;
  }
}

void GoBoardUtil::DiagonalsOfColor(const GoBoard& bd, GoPoint p, int c,
                                   SgVector<GoPoint>* diagonals) {
  diagonals->Clear();
  if (bd.IsColor(p - GO_NORTH_SOUTH - GO_WEST_EAST, c))
    diagonals->PushBack(p - GO_NORTH_SOUTH - GO_WEST_EAST);
  if (bd.IsColor(p - GO_NORTH_SOUTH + GO_WEST_EAST, c))
    diagonals->PushBack(p - GO_NORTH_SOUTH + GO_WEST_EAST);
  if (bd.IsColor(p + GO_NORTH_SOUTH - GO_WEST_EAST, c))
    diagonals->PushBack(p + GO_NORTH_SOUTH - GO_WEST_EAST);
  if (bd.IsColor(p + GO_NORTH_SOUTH + GO_WEST_EAST, c))
    diagonals->PushBack(p + GO_NORTH_SOUTH + GO_WEST_EAST);
}

bool GoBoardUtil::EndOfGame(const GoBoard& bd) {
  SgBlackWhite toPlay = bd.ToPlay();
  GoPlayerMove passToPlay(toPlay, GO_PASS);
  GoPlayerMove passOpp(SgOppBW(toPlay), GO_PASS);
  int moveNumber = bd.MoveNumber();
  if (bd.Rules().TwoPassesEndGame()) {
    return moveNumber >= 2
        && bd.Move(moveNumber - 1) == passOpp
        && bd.Move(moveNumber - 2) == passToPlay;
  } else {
    return moveNumber >= 3
        && bd.Move(moveNumber - 1) == passOpp
        && bd.Move(moveNumber - 2) == passToPlay
        && bd.Move(moveNumber - 3) == passOpp;
  }
}

bool GoBoardUtil::GenerateIfLegal(const GoBoard& bd, GoPoint move,
                                  SgVector<GoPoint>* moves) {
  if (bd.IsLegal(move)) {
    if (moves)
      moves->Include(move);
    return true;
  }
  return false;
}

void GoBoardUtil::GetCoordString(GoMove p, std::string* s, int boardSize) {
  DBG_ASSERT(s);
  DBG_ASSERT(p != GO_NULLMOVE);
  if (p == GO_PASS)
    *s = "Pass";
  else if (p == GO_COUPONMOVE)
    *s = "Coupon";
  else {
    int col = GoPointUtil::Col(p);
    int row = GoPointUtil::Row(p);
    if (9 <= col)
      ++col;
    std::ostringstream o;
    o << static_cast<char>('A' + col - 1) << (boardSize + 1 - row);
    *s = o.str();
  }
}

bool GoBoardUtil::HasAdjacentBlocks(const GoBoard& bd, GoPoint p,
                                    int maxLib) {
  DBG_ASSERT(bd.Occupied(p));
  const SgBlackWhite other = SgOppBW(bd.GetStone(p));
  for (GoBoard::StoneIterator stone(bd, p); stone; ++stone)
    for (GoNbIterator nb(bd, *stone); nb; ++nb)
      if (bd.IsColor(*nb, other) && bd.AtMostNumLibs(*nb, maxLib))
        return true;
  return false;
}

bool GoBoardUtil::PointHasAdjacentBlock(const GoBoard& bd, GoPoint p,
                                        SgBlackWhite blockColor, int maxLib) {
  DBG_ASSERT(bd.GetColor(p) != blockColor);
  for (GoNbIterator it(bd, p); it; ++it)
    if (bd.IsColor(*it, blockColor)
        && bd.AtMostNumLibs(*it, maxLib)
        )
      return true;
  return false;
}

bool GoBoardUtil::IsHandicapPoint(GoGrid size, GoGrid col, GoGrid row) {
  GoGrid line1;
  GoGrid line3;
  if (size < 9)
    return false;
  if (size <= 11) {
    line1 = 3;
    line3 = size - 2;
  } else {
    line1 = 4;
    line3 = size - 3;
  }
  if (size > 11 && size % 2 != 0) {
    GoGrid line2 = size / 2 + 1;
    return (row == line1 || row == line2 || row == line3)
        && (col == line1 || col == line2 || col == line3);
  } else
    return (row == line1 || row == line3)
        && (col == line1 || col == line3);
}

bool GoBoardUtil::IsSimpleEyeOfBlock(const GoBoard& bd, GoPoint lib,
                                     GoPoint blockAnchor,
                                     const SgVector<GoPoint>& eyes) {
  SgBlackWhite color = bd.GetStone(blockAnchor);
  if (bd.IsColor(lib - GO_NORTH_SOUTH, color)
      && bd.Anchor(lib - GO_NORTH_SOUTH) != blockAnchor)
    return false;
  if (bd.IsColor(lib + GO_NORTH_SOUTH, color)
      && bd.Anchor(lib + GO_NORTH_SOUTH) != blockAnchor)
    return false;
  if (bd.IsColor(lib - GO_WEST_EAST, color)
      && bd.Anchor(lib - GO_WEST_EAST) != blockAnchor)
    return false;
  if (bd.IsColor(lib + GO_WEST_EAST, color)
      && bd.Anchor(lib + GO_WEST_EAST) != blockAnchor)
    return false;
  int nuForFalse = (bd.Line(lib) == 1) ? 1 : 2;
  for (SgNb4DiagIterator it(lib); it; ++it) {
    GoPoint nb(*it);
    if (!bd.IsBorder(nb) && !bd.IsColor(nb, color)
        && !eyes.Contains(nb))
      if (--nuForFalse <= 0)
        return false;
  }
  return true;
}

bool GoBoardUtil::IsSnapback(const GoBoard& constBd, GoPoint p) {
  DBG_ASSERT(constBd.IsValidPoint(p));
  DBG_ASSERT(constBd.Occupied(p));

  bool snapback = false;
  if (constBd.IsSingleStone(p) && constBd.InAtari(p)) {
    const GoPoint lib = constBd.TheLiberty(p);
    GoModBoard mbd(constBd);
    GoBoard& bd = mbd.Board();
    const bool isLegal =
        GoBoardUtil::PlayIfLegal(bd, lib, SgOppBW(bd.GetStone(p)));
    if (isLegal
        && bd.InAtari(lib)
        && !bd.IsSingleStone(lib)
        )
      snapback = true;
    if (isLegal)
      bd.Undo();
  }
  return snapback;
}

bool GoBoardUtil::ManySecondaryLibs(const GoBoard& bd, GoPoint block) {
  const int LIBERTY_LIMIT = 9;
  static SgMarker m;
  m.Clear();
  int nu = 0;
  for (GoBoard::LibertyIterator it(bd, block); it; ++it) {
    GoPoint p(*it);
    if (m.NewMark(p) && ++nu >= LIBERTY_LIMIT)
      return true;
    for (GoNbIterator itn(bd, p); itn; ++itn)
      if (bd.IsEmpty(*itn)
          && m.NewMark(*itn)
          && ++nu >= LIBERTY_LIMIT
          )
        return true;
  }
  DBG_ASSERT(nu < LIBERTY_LIMIT);
  return false;
}

GoArrayList<GoPoint, 4> GoBoardUtil::NeighborsOfColor(const GoBoard& bd,
                                                      GoPoint p, int c) {
  GoArrayList<GoPoint, 4> result;
  if (bd.IsColor(p - GO_NORTH_SOUTH, c))
    result.PushBack(p - GO_NORTH_SOUTH);
  if (bd.IsColor(p - GO_WEST_EAST, c))
    result.PushBack(p - GO_WEST_EAST);
  if (bd.IsColor(p + GO_WEST_EAST, c))
    result.PushBack(p + GO_WEST_EAST);
  if (bd.IsColor(p + GO_NORTH_SOUTH, c))
    result.PushBack(p + GO_NORTH_SOUTH);
  return result;
}

void GoBoardUtil::NeighborsOfColor(const GoBoard& bd, GoPoint p, int c,
                                   SgVector<GoPoint>* neighbors) {
  neighbors->Clear();
  if (bd.IsColor(p - GO_NORTH_SOUTH, c))
    neighbors->PushBack(p - GO_NORTH_SOUTH);
  if (bd.IsColor(p - GO_WEST_EAST, c))
    neighbors->PushBack(p - GO_WEST_EAST);
  if (bd.IsColor(p + GO_WEST_EAST, c))
    neighbors->PushBack(p + GO_WEST_EAST);
  if (bd.IsColor(p + GO_NORTH_SOUTH, c))
    neighbors->PushBack(p + GO_NORTH_SOUTH);
}

bool GoBoardUtil::TrompTaylorPassWins(const GoBoard& bd, SgBlackWhite toPlay) {
  if (toPlay != bd.ToPlay())
    return false;
  if (!bd.Rules().CaptureDead() || bd.Rules().JapaneseScoring())
    return false;
  if (bd.GetLastMove() != GO_PASS)
    return false;
  float komi = bd.Rules().Komi().ToFloat();
  float score = GoBoardUtil::TrompTaylorScore(bd, komi);
  if ((score > 0 && toPlay == SG_BLACK) || (score < 0 && toPlay == SG_WHITE))
    return true;
  return false;
}

bool GoBoardUtil::PlayIfLegal(GoBoard& bd, GoPoint p, SgBlackWhite player) {
  if (p != GO_PASS && p != GO_COUPONMOVE) {
    if (!bd.IsEmpty(p))
      return false;
    if (!bd.Rules().AllowSuicide() && bd.IsSuicide(p, player))
      return false;
  }
  bd.Play(p, player);
  if (bd.LastMoveInfo(GO_MOVEFLAG_ILLEGAL)) {
    bd.Undo();
    return false;
  }
  return true;
}

void GoBoardUtil::ReduceToAnchors(const GoBoard& bd,
                                  SgVector<GoPoint>* stones) {
  DBG_ASSERT(stones);
  SgVector<GoPoint> result;
  for (SgVectorIterator<GoPoint> stone(*stones); stone; ++stone)
    if (bd.Occupied(*stone))
      result.Insert(bd.Anchor(*stone));
  stones->SwapWith(&result);
}

void GoBoardUtil::ReduceToAnchors(const GoBoard& bd,
                                  const SgVector<GoPoint>& stones,
                                  GoArrayList<GoPoint, GO_MAXPOINT>& anchors) {
  anchors.Clear();
  for (SgVectorIterator<GoPoint> it(stones); it; ++it)
    if (bd.Occupied(*it))
      anchors.Include(bd.Anchor(*it));
}

void GoBoardUtil::RegionCode(const GoBoard& bd,
                             const SgVector<GoPoint>& region,
                             SgHashCode* c) {
  BOOST_STATIC_ASSERT(SG_BLACK < 2);
  BOOST_STATIC_ASSERT(SG_WHITE < 2);

  c->Clear();
  for (SgVectorIterator<GoPoint> it(region); it; ++it) {
    GoPoint p = *it;
    if (bd.Occupied(p))
      SgHashUtil::XorZobrist(*c, p + bd.GetStone(p) * GO_MAXPOINT);
  }
}

bool GoBoardUtil::RemainingChineseHandicap(const GoBoard& bd) {
  const GoRules& rules = bd.Rules();
  return (!rules.JapaneseHandicap()
      && rules.Handicap() > bd.TotalNumStones(SG_BLACK));
}

float GoBoardUtil::ScoreSimpleEndPosition(const GoBoard& bd, float komi,
                                          bool noCheck) {
  int score = 0;
  for (GoBoard::Iterator it(bd); it; ++it)
    score += ScorePoint(bd, *it, noCheck);
  return float(score) - komi;
}

void GoBoardUtil::SharedLiberties(const GoBoard& bd,
                                  GoPoint block1,
                                  GoPoint block2,
                                  SgVector<GoPoint>* sharedLibs) {
  DBG_ASSERT(sharedLibs);
  DBG_ASSERT(bd.Occupied(block1));
  DBG_ASSERT(bd.Occupied(block2));
  block1 = bd.Anchor(block1);
  block2 = bd.Anchor(block2);
  sharedLibs->Clear();
  for (GoBoard::LibertyIterator libIter(bd, block1); libIter; ++libIter) {
    GoPoint lib = *libIter;
    if (bd.IsLibertyOfBlock(lib, block2))
      sharedLibs->PushBack(lib);
  }
}

void GoBoardUtil::SharedLibertyBlocks(const GoBoard& bd, GoPoint anchor,
                                      int maxLib, SgVector<GoPoint>* blocks) {
  DBG_ASSERT(blocks);
  SgMarker& mark = bd.m_userMarker;
  SgReserveMarker reserve(mark);
  SuppressUnused(reserve);
  mark.Clear();
  for (GoBoard::StoneIterator it1(bd, anchor); it1; ++it1)
    mark.Include(*it1);
  for (SgVectorIterator<GoPoint> blit(*blocks); blit; ++blit) {
    GoPoint a = *blit;
    for (GoBoard::StoneIterator it(bd, a); it; ++it)
      mark.Include(*it);
  }
  SgBlackWhite c = bd.GetStone(anchor);
  for (GoBoard::LibertyIterator it2(bd, anchor); it2; ++it2) {
    GoPoint p = *it2;
    if (bd.NumNeighbors(p, c) > 0) {
      if (bd.IsColor(p - GO_NORTH_SOUTH, c) && mark.NewMark(bd.Anchor(p - GO_NORTH_SOUTH))
          && bd.AtMostNumLibs(p - GO_NORTH_SOUTH, maxLib))
        blocks->PushBack(bd.Anchor(p - GO_NORTH_SOUTH));
      if (bd.IsColor(p - GO_WEST_EAST, c) && mark.NewMark(bd.Anchor(p - GO_WEST_EAST))
          && bd.AtMostNumLibs(p - GO_WEST_EAST, maxLib))
        blocks->PushBack(bd.Anchor(p - GO_WEST_EAST));
      if (bd.IsColor(p + GO_WEST_EAST, c) && mark.NewMark(bd.Anchor(p + GO_WEST_EAST))
          && bd.AtMostNumLibs(p + GO_WEST_EAST, maxLib))
        blocks->PushBack(bd.Anchor(p + GO_WEST_EAST));
      if (bd.IsColor(p + GO_NORTH_SOUTH, c) && mark.NewMark(bd.Anchor(p + GO_NORTH_SOUTH))
          && bd.AtMostNumLibs(p + GO_NORTH_SOUTH, maxLib))
        blocks->PushBack(bd.Anchor(p + GO_NORTH_SOUTH));
    }
  }
}

void GoBoardUtil::UndoAll(GoBoard& bd) {
  while (bd.CanUndo())
    bd.Undo();
}

bool GoBoardUtil::AtLeastTwoSharedLibs(const GoBoard& bd, GoPoint block1,
                                       GoPoint block2) {
  DBG_ASSERT(bd.Occupied(block1));
  DBG_ASSERT(bd.Occupied(block2));
  block2 = bd.Anchor(block2);
  bool fHasOneShared = false;
  for (GoBoard::LibertyIterator libIter(bd, block1); libIter; ++libIter) {
    if (bd.IsLibertyOfBlock(*libIter, block2)) {
      if (fHasOneShared)
        return true;
      fHasOneShared = true;
    }
  }
  return false;
}

void GoBoardUtil::TestForChain(GoBoard& bd, GoPoint block, GoPoint block2,
                               GoPoint lib, SgVector<GoPoint>* extended) {
  if (AtLeastTwoSharedLibs(bd, block, block2))
    extended->PushBack(block);
  else {
    GoRestoreToPlay r(bd);
    bd.SetToPlay(SgOppBW(bd.GetStone(block)));
    if (MoveNotLegalOrAtari(bd, lib))
      extended->PushBack(block);
  }
}

bool GoBoardUtil::HasStonesOfBothColors(const GoBoard& bd,
                                        const SgVector<GoPoint>& stones) {
  SgBWArray<bool> has(false);
  for (SgVectorIterator<GoPoint> it(stones); it; ++it) {
    if (bd.Occupied(*it)) {
      SgBlackWhite color(bd.GetStone(*it));
      has[color] = true;
      if (has[SgOppBW(color)])
        return true;
    }
  }
  return false;
}

bool GoBoardUtil::MoveNotLegalOrAtari(const GoBoard& bd, GoPoint move) {
  GoModBoard modBoard(bd);
  GoMoveExecutor execute(modBoard.Board(), move);
  return (!execute.IsLegal() || bd.InAtari(move));
}

bool GoBoardUtil::MoveLegalAndNotAtari(const GoBoard& bd, GoPoint move) {
  GoModBoard modBoard(bd);
  GoMoveExecutor execute(modBoard.Board(), move);
  return (execute.IsLegal() && !bd.InAtari(move));
}

bool GoBoardUtil::ScorePosition(const GoBoard& bd,
                                const GoPointSet& deadStones, float& score) {
  SgMarker marker;
  int stones = 0;
  int territory = 0;
  int dead = 0;
  for (GoBoard::Iterator it(bd); it; ++it) {
    GoPoint p = *it;
    if (bd.Occupied(p) && !deadStones.Contains(p)) {
      if (bd.GetColor(p) == SG_BLACK)
        ++stones;
      else
        --stones;
      continue;
    }
    if (marker.Contains(p))
      continue;
    int nuPoints = 0;
    int nuDeadWhite = 0;
    int nuDeadBlack = 0;
    bool isBlackAdjacent = false;
    bool isWhiteAdjacent = false;
    ScorePositionRecurse(bd, p, deadStones, marker, isBlackAdjacent,
                         isWhiteAdjacent, nuPoints, nuDeadWhite,
                         nuDeadBlack);
    if ((nuDeadWhite > 0 && nuDeadBlack > 0)
        || (isBlackAdjacent && nuDeadBlack > 0)
        || (isWhiteAdjacent && nuDeadWhite > 0))
      return false;
    dead += nuDeadBlack;
    dead -= nuDeadWhite;
    if (isBlackAdjacent && !isWhiteAdjacent)
      territory += nuPoints;
    else if (isWhiteAdjacent && !isBlackAdjacent)
      territory -= nuPoints;
  }
  int prisoners = bd.NumPrisoners(SG_BLACK) - bd.NumPrisoners(SG_WHITE);
  float komi = bd.Rules().Komi().ToFloat();
  if (bd.Rules().JapaneseScoring())
    score = float(territory - dead - prisoners) - komi;
  else
    score = float(territory + stones) - komi;
  return true;
}

int GoBoardUtil::Stones(const GoBoard& bd, GoPoint p, GoPoint stones[]) {
  DBG_ASSERT(bd.IsValidPoint(p));
  DBG_ASSERT(bd.Occupied(p));
  if (bd.IsSingleStone(p)) {
    stones[0] = p;
    return 1;
  } else {
    int nm = 0;
    for (GoBoard::StoneIterator it(bd, bd.Anchor(p)); it; ++it)
      stones[nm++] = p;
    return nm;
  }
}

bool GoBoardUtil::TwoPasses(const GoBoard& bd) {
  SgBlackWhite toPlay = bd.ToPlay();
  GoPlayerMove passToPlay(toPlay, GO_PASS);
  GoPlayerMove passOpp(SgOppBW(toPlay), GO_PASS);
  int moveNumber = bd.MoveNumber();
  return (moveNumber >= 2
      && bd.Move(moveNumber - 1) == passOpp
      && bd.Move(moveNumber - 2) == passToPlay
  );
}

GoPointSet GoBoardUtil::Lines(const GoBoard& bd, GoGrid from, GoGrid to) {
  DBG_ASSERT(from >= 1);
  DBG_ASSERT(from <= to);
  DBG_ASSERT(to <= (bd.Size() + 1) / 2);
  GoPointSet lines;
  for (GoGrid i = from; i <= to; ++i)
    lines |= bd.LineSet(i);
  return lines;
}

GoRect GoBoardUtil::GetDirtyRegion(const GoBoard& bd, GoMove move,
                                   SgBlackWhite colour, bool checklibs,
                                   bool premove) {
  GoRect dirty;
  if (move == GO_PASS)
    return dirty;

  SgBlackWhite opp = SgOppBW(colour);
  dirty.Include(move);

  GoPointSet blocks;
  if (checklibs) {
    for (GoNbIterator inb(bd, move); inb; ++inb)
      if (bd.Occupied(*inb))
        for (GoBoard::StoneIterator istone(bd, *inb); istone;
             ++istone)
          dirty.Include(*istone);
  }
  if (premove) {
    for (GoNbIterator inb(bd, move); inb; ++inb) {
      if (bd.IsColor(*inb, opp) && bd.NumLiberties(*inb) == 1) {
        for (GoBoard::StoneIterator icap(bd, *inb); icap; ++icap) {
          dirty.Include(*icap);
          if (checklibs) {
            for (GoNbIterator inb2(bd, *icap); inb2; ++inb2)
              if (bd.IsColor(*inb2, colour))
                blocks.Include(bd.Anchor(*inb2));
          }
        }
      }
    }
  }
  if (!premove && bd.CapturingMove()) {
    for (GoPointList::Iterator icaptures(bd.CapturedStones()); icaptures;
         ++icaptures) {
      dirty.Include(*icaptures);
      if (checklibs) {
        for (GoNbIterator inb(bd, *icaptures); inb; ++inb)
          if (bd.IsColor(*inb, colour))
            blocks.Include(bd.Anchor(*inb));
      }
    }
  }
  if (checklibs) {
    for (SgSetIterator iblocks(blocks); iblocks; ++iblocks)
      for (GoBoard::StoneIterator istone(bd, *iblocks); istone;
           ++istone)
        dirty.Include(*istone);
  }

  return dirty;
}

int GoBoardUtil::Approx2Libs(const GoBoard& board, GoPoint block,
                             GoPoint p, SgBlackWhite color) {
  int libs2 = 0;
  for (GoNbIterator inb(board, p); inb; ++inb) {
    GoPoint nb = *inb;
    if (board.IsEmpty(nb))
      libs2++;
    else if (board.IsColor(nb, color)
        && board.Anchor(nb) != board.Anchor(block)
        )
      libs2 += board.NumLiberties(nb);
  }

  return libs2;
}

std::ostream& operator<<(std::ostream& out, const GoBoardWrite::WriteMap& w) {
  w.Points().Write(out, w.Board().Size());
  return out;
}
