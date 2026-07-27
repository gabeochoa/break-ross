#include "preload.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "log.h"
#include "rl.h"

#include "input_mapping.h"
#include "settings.h"
#include <afterhours/src/plugins/color.h>
#include <afterhours/src/plugins/files.h>
#include <afterhours/src/plugins/ui/theme.h>

using namespace afterhours;

static void load_gamepad_mappings() {
  std::ifstream ifs(
      files::get_resource_path("", "gamecontrollerdb.txt").string().c_str());
  if (!ifs.is_open()) {
    log_warn("failed to load game controller db");
    return;
  }
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  input::set_gamepad_mappings(buffer.str().c_str());
}

Preload::Preload() {}

Preload &Preload::init(const char *title) {
  files::init("Prime Pressure", "resources");

  int width = Settings::get().get_screen_width();
  int height = Settings::get().get_screen_height();

  raylib::InitWindow(width, height, title);
  raylib::SetWindowSize(width, height);
  raylib::SetWindowState(raylib::FLAG_WINDOW_RESIZABLE);

  raylib::TraceLogLevel logLevel = raylib::LOG_ERROR;
  raylib::SetTraceLogLevel(logLevel);
  raylib::SetTargetFPS(200);

  raylib::SetAudioStreamBufferSizeDefault(4096);
  raylib::InitAudioDevice();
  if (!raylib::IsAudioDeviceReady()) {
    log_warn("audio device not ready; continuing without audio");
  }
  raylib::SetMasterVolume(1.f);

  raylib::SetExitKey(0);

  load_gamepad_mappings();

  return *this;
}

Preload &Preload::make_singleton() {
  // init_ui_plugin creates the UI root entity and registers all UI singletons
  // (UIContext, FontManager, TextMeasureCache, AutoLayoutRoot component, etc.).
  auto &sophie = ui::init_ui_plugin<InputAction>();
  {
    input::add_singleton_components(sophie, get_mapping());
    window_manager::add_singleton_components(sophie, 200);
    // init_ui_plugin adds AutoLayoutRoot but doesn't register it as a singleton;
    // get_sophie() looks the entity up by this singleton.
    EntityHelper::registerSingleton<ui::AutoLayoutRoot>(sophie);

    ui::imm::ThemeDefaults::get()
        .set_theme_color(ui::Theme::Usage::Primary, colors::UI_GREEN)
        .set_theme_color(ui::Theme::Usage::Error, colors::UI_RED)
        .set_theme_color(ui::Theme::Usage::Font, colors::UI_WHITE)
        .set_theme_color(ui::Theme::Usage::Background, colors::UI_BLACK)
        .set_theme_color(ui::Theme::Usage::Secondary, raylib::YELLOW)
        .set_theme_color(ui::Theme::Usage::Accent, raylib::GREEN);
  }
  return *this;
}

Preload::~Preload() {
  if (raylib::IsAudioDeviceReady()) {
    raylib::CloseAudioDevice();
  }
  if (raylib::IsWindowReady()) {
    raylib::CloseWindow();
  }
}
