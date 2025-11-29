#ifdef BACKWARD
#include "backward/backward.hpp"
namespace backward {
backward::SignalHandling sh;
} // namespace backward
#endif

#include "argh.h"
#include "game.h"
#include "preload.h"
#include "settings.h"
#include "testing/test_macros.h"
#include "testing/tests/all_tests.h"
#include <iostream>

using namespace afterhours;

int main(int argc, char *argv[]) {
  argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

  if (cmdl["--help"]) {
    std::cout << "Break Ross \n\n";
    std::cout << "Usage: break_ross [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help                        Show this help message\n";
    std::cout
        << "  -w, --width <pixels>          Screen width (default: 1280)\n";
    std::cout
        << "  -h, --height <pixels>         Screen height (default: 720)\n";
    std::cout << "  --list-tests                 List all available tests\n";
    std::cout << "  --run-test <name>            Run a specific test\n";
    std::cout
        << "  --slow-test                  Run test in slow mode (visible)\n";
    return 0;
  }

  if (cmdl["--list-tests"]) {
    TestRegistry &registry = TestRegistry::get();
    std::cout << "Available tests:\n";
    for (const auto &[name, func] : registry.tests) {
      std::cout << "  - " << name << "\n";
    }
    return 0;
  }

  std::string test_name;
  if (cmdl({"--run-test"}) >> test_name) {
    bool slow_mode = cmdl["--slow-test"];

    int screenWidth, screenHeight;
    cmdl({"-w", "--width"}, 1280) >> screenWidth;
    cmdl({"-h", "--height"}, 720) >> screenHeight;

    Settings::get().load_save_file(screenWidth, screenHeight);

    Preload::get() //
        .init("Break Ross")
        .make_singleton();
    Settings::get().refresh_settings();

    run_test(test_name, slow_mode);

    Settings::get().write_save_file();

    return 0;
  }

  int screenWidth, screenHeight;
  cmdl({"-w", "--width"}, 1280) >> screenWidth;
  cmdl({"-h", "--height"}, 720) >> screenHeight;

  Settings::get().load_save_file(screenWidth, screenHeight);

  Preload::get() //
      .init("Break Ross")
      .make_singleton();
  Settings::get().refresh_settings();

  game();

  Settings::get().write_save_file();

  return 0;
}
