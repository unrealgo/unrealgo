

#include "platform/SgSystem.h"
#include "GoSafetyCommands.h"

#include "GoBensonSolver.h"
#include "GoBoard.h"
#include "GoGtpCommandUtil.h"
#include "GoModBoard.h"
#include "GoSafetySolver.h"
#include "GoSafetyUtil.h"
#include "board/GoPointSet.h"

using GoGtpCommandUtil::BlackWhiteArg;

GoSafetyCommands::GoSafetyCommands(const GoBoard& bd)
    : m_bd(bd) {}

void GoSafetyCommands::AddGoGuiAnalyzeCommands(GtpCommand& cmd) {
  cmd <<
      "plist/Go Safe Dame Static/go_safe_dame_static\n"
          "gfx/Go Safe Benson/go_safe_gfx benson\n"
          "gfx/Go Safe Static/go_safe_gfx static\n"
          "gfx/Go Safe Winner/go_safe_winner\n";
}

void GoSafetyCommands::CmdDameStatic(GtpCommand& cmd) {
  cmd.CheckArgNone();
  GoRegionBoard regionAttachment(m_bd);
  GoSafetySolver solver(m_bd, &regionAttachment);
  GoBWSet safe;
  solver.FindSafePoints(&safe);
  GoPointSet dame =
      GoSafetyUtil::FindDamePoints(m_bd, m_bd.AllEmpty(), safe);
  cmd << PointSetWriter(dame, "", false);
}

void GoSafetyCommands::CmdGfx(GtpCommand& cmd) {
  std::string type = cmd.Arg();
  int totalRegions = 0;
  GoBWSet safe = GetSafe(totalRegions, type);
  GoPointSet dame;
  GoPointSet unsurroundable;
  GoSafetyUtil::FindDameAndUnsurroundablePoints(m_bd, m_bd.AllEmpty(), safe,
                                                &dame, &unsurroundable);
  cmd << "BLACK";
  for (SgSetIterator it(safe[SG_BLACK]); it; ++it)
    cmd << ' ' << GoWritePoint(*it);
  cmd << '\n';
  cmd << "WHITE";
  for (SgSetIterator it(safe[SG_WHITE]); it; ++it)
    cmd << ' ' << GoWritePoint(*it);
  cmd << '\n';
  cmd << "COLOR #980098";
  for (SgSetIterator it(dame); it; ++it)
    cmd << ' ' << GoWritePoint(*it);
  cmd << '\n';
  cmd << "CIRCLE";
  for (SgSetIterator it(unsurroundable); it; ++it)
    cmd << ' ' << GoWritePoint(*it);
  cmd << '\n';
  GoPointSet blackAndWhite = safe[SG_WHITE] & safe[SG_BLACK];
  if (blackAndWhite.Size() > 0) {
    cmd << "COLOR red ";
    for (SgSetIterator it(blackAndWhite); it; ++it)
      cmd << ' ' << GoWritePoint(*it);
    cmd << '\n';
  }
  int nuBlack = safe[SG_BLACK].Size();
  int nuWhite = safe[SG_WHITE].Size();
  int nuPoints = m_bd.AllPoints().Size();
  cmd << "TEXT Solver: " << cmd.Arg(0)
      << "  B: " << nuBlack << " (" << (100 * nuBlack / nuPoints) << " %)"
      << "  W: " << nuWhite << " (" << (100 * nuWhite / nuPoints) << " %)"
      << "  Both: " << (nuBlack + nuWhite)
      << " (" << (100 * (nuBlack + nuWhite) / nuPoints) << " %)"
      << "  Regions: " << totalRegions;
}

void GoSafetyCommands::CmdSafe(GtpCommand& cmd) {
  cmd.CheckNuArgLessEqual(2);
  std::string type = cmd.Arg(0);
  int totalRegions = 0;
  GoBWSet safe = GetSafe(totalRegions, type);
  GoPointSet set =
      (cmd.NuArg() == 2 ? safe[BlackWhiteArg(cmd, 1)] : safe.Both());
  cmd << set.Size();
  for (SgSetIterator it(set); it; ++it)
    cmd << ' ' << GoWritePoint(*it);
}

void GoSafetyCommands::CmdWinner(GtpCommand& cmd) {
  cmd.CheckArgNone();
  const SgEmptyBlackWhite winner = GoSafetyUtil::GetWinner(m_bd);
  if (winner == SG_BLACK)
    cmd << "black";
  else if (winner == SG_WHITE)
    cmd << "white";
  else
    cmd << "unknown";
}

GoBWSet GoSafetyCommands::GetSafe(int& totalRegions, const std::string& type) {
  GoRegionBoard regionAttachment(m_bd);
  GoBWSet safe;
  if (type == "benson") {
    GoBensonSolver solver(m_bd, &regionAttachment);
    solver.FindSafePoints(&safe);
  } else if (type == "static") {
    GoSafetySolver solver(m_bd, &regionAttachment);
    solver.FindSafePoints(&safe);
  } else
    throw GtpFailure() << "invalid safety solver: " << type;
  GoPointSet proved = safe.Both();
  for (SgBWIterator cit; cit; ++cit) {
    SgBlackWhite c = *cit;
    for (SgVectorIteratorOf<GoRegion> it(regionAttachment.AllRegions(c));
         it; ++it)
      if ((*it)->Points().SubsetOf(proved))
        ++totalRegions;
  }
  return safe;
}

void GoSafetyCommands::Register(GtpEngine& e) {
  Register(e, "go_safe", &GoSafetyCommands::CmdSafe);
  Register(e, "go_safe_dame_static", &GoSafetyCommands::CmdDameStatic);
  Register(e, "go_safe_gfx", &GoSafetyCommands::CmdGfx);
  Register(e, "go_safe_winner", &GoSafetyCommands::CmdWinner);
}

void GoSafetyCommands::Register(GtpEngine& engine,
                                const std::string& command,
                                GtpCallback<GoSafetyCommands>::Method method) {
  engine.Register(command,
                  new GtpCallback<GoSafetyCommands>(this, method));
}
