#include "Run.hpp"

#include "../BuildConfig.hpp"
#include "../Logger.hpp"
#include "../Manifest.hpp"
#include "Build.hpp"
#include "Global.hpp"

#include <cstdlib>
#include <iostream>
#include <span>

int runMain(std::span<const StringRef> args) {
  // Parse args
  bool isDebug = true;
  String runArgs;
  for (usize i = 0; i < args.size(); ++i) {
    StringRef arg = args[i];
    HANDLE_GLOBAL_OPTS({{"run"}})

    else if (arg == "-d" || arg == "--debug") {
      isDebug = true;
    }
    else if (arg == "-r" || arg == "--release") {
      isDebug = false;
    }
    else {
      runArgs += " " + String(arg);
    }
  }

  String outDir;
  if (buildImpl(isDebug, outDir) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  const String projectName = getPackageName();
  const String command = outDir + "/" + projectName + runArgs;
  Logger::info("Running", command);
  const int status = std::system(command.c_str());
  const int exitCode = status >> 8;
  return exitCode;
}

void runHelp() noexcept {
  std::cout << runDesc << '\n';
  std::cout << '\n';
  printUsage("run", "[OPTIONS] [args]...");
  std::cout << '\n';
  printHeader("Options:");
  printGlobalOpts();
  printOption("--debug", "-d", "Build with debug information [default]");
  printOption("--release", "-r", "Build with optimizations");
  std::cout << '\n';
  printHeader("Arguments:");
  std::cout << "  [args]...\tArguments passed to the program" << '\n';
}