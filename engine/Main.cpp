#include <iostream>
#include <boost/foreach.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include "MainEngine.h"
#include "GoInit.h"
#include "SgInit.h"
#include "platform/SgPlatform.h"
#include "Version.h"

using boost::filesystem::path;
using std::ostream;
using std::string;
using std::vector;
namespace po = boost::program_options;

namespace {
path GetProgramDir(const char* programPath) {
  if (programPath == nullptr)
    return "";
  return path(programPath).branch_path();
}

path GetTopSourceDir() {
#ifdef ABS_TOP_SRCDIR
  return path(ABS_TOP_SRCDIR);
#else
  return "";
#endif
}

void Help(po::options_description& desc, ostream& out) {
  out << "Usage: " APP_NAME " [options] [input files]\n" << desc << "\n";
  exit(0);
}

struct CommandLineOptions {
  CommandLineOptions();
  bool m_allowHandicap;
  bool m_quiet;
  int m_fixedBoardSize;
  int m_maxGames;
  string m_config;
  const char* m_programPath;
  int m_srand;
  vector<string> m_inputFiles;
};

CommandLineOptions::CommandLineOptions() : m_allowHandicap(false),
                                           m_quiet(false),
                                           m_fixedBoardSize(0),
                                           m_maxGames(-1),
                                           m_config(),
                                           m_programPath(nullptr),
                                           m_srand(0) {}

void ParseOptions(int argc, char** argv, struct CommandLineOptions& options) {
  po::options_description normalOptions("Options");
  normalOptions.add_options()
      ("config",
       po::value<std::string>(&options.m_config)->default_value(""),
       "execute GTP commands from file before starting main command loop")

      ("help", "Displays this help and exit")

      ("maxgames",
       po::value<int>(&options.m_maxGames)->default_value(-1),
       "make clear_board fail after n invocations")

      ("nobook", "don't automatically load opening book")

      ("nohandicap", "don't support handicap commands")

      ("quiet", "don't print debug messages")

      ("srand",
       po::value<int>(&options.m_srand)->default_value(0),
       "set random seed (-1:none, 0:time(0))")

      ("size",
       po::value<int>(&options.m_fixedBoardSize)->default_value(0),
       "initial (and fixed) board size");

  po::options_description hiddenOptions;
  hiddenOptions.add_options()
      ("input-file", po::value<vector<string> >(&options.m_inputFiles),
       "input file");
  po::options_description allOptions;
  allOptions.add(normalOptions).add(hiddenOptions);
  po::positional_options_description positionalOptions;
  positionalOptions.add("input-file", -1);
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, (const char* const*)argv).options(allOptions).
        positional(positionalOptions).run(), vm);
    po::notify(vm);
  }
  catch (...) {
    Help(normalOptions, std::cerr);
  }
  if (vm.count("help"))
    Help(normalOptions, std::cout);
  if (vm.count("nohandicap"))
    options.m_allowHandicap = false;
  if (vm.count("quiet"))
    options.m_quiet = true;
}

void PrintStartupMessage() {
  SgDebug() << UnrealGo::License("");
}
}

int main(int argc, char** argv) {
  struct CommandLineOptions options;
  options.m_allowHandicap = true;
  options.m_quiet = false;

  if (argc > 0 && argv != nullptr) {
    options.m_programPath = argv[0];
    SgPlatform::SetProgramDir(GetProgramDir(argv[0]));
    SgPlatform::SetTopSourceDir(GetTopSourceDir());
    try {
      ParseOptions(argc, argv, options);
    }
    catch (const SgException& e) {
      SgDebug() << e.what() << "\n";
      return 1;
    }
  }

  if (options.m_quiet)
    SgDebugToNull();

  try {
    SgInit();
    GoInit();

    PrintStartupMessage();

    SgRandom::SetSeed(options.m_srand);
    MainEngine engine(options.m_fixedBoardSize,
                      options.m_programPath,
                      !options.m_allowHandicap);
    GoGtpAssertionHandler assertionHandler(engine);

    if (options.m_maxGames >= 0)
      engine.SetMaxClearBoard(options.m_maxGames);
    if (!options.m_config.empty())
      engine.ExecuteFile(options.m_config);
    if (!options.m_inputFiles.empty()) {
      for (auto& file : options.m_inputFiles) {
        std::ifstream fin(file.c_str());
        if (!fin)
          throw SgException(boost::format("Error file '%1%'") % file);
        GtpInputStream in(fin);
        GtpOutputStream out(std::cout);
        engine.MainLoop(in, out);
      }
    } else {
      GtpInputStream in(std::cin);
      GtpOutputStream out(std::cout);
      engine.MainLoop(in, out);
    }
  }
  catch (const GtpFailure& e) {
    SgDebug() << e.Response() << '\n';
    return 1;
  }
  catch (const std::exception& e) {
    SgDebug() << e.what() << '\n';
    return 1;
  }
  return 0;
}