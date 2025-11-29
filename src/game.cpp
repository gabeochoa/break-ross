#include "game.h"

#include "components.h"
#include "game_constants.h"
#include "game_setup.h"
#include "input_mapping.h"
#include "log.h"
#include "preload.h"
#include "render_backend.h"
#include "settings.h"
#include "systems/BallPhysics.h"
#include "systems/HandleCameraControls.h"
#include "systems/HandleCollisions.h"
#include "systems/HandleShopInput.h"
#include "systems/LoopDetection.h"
#include "systems/MazeTraversal.h"
#include "systems/RebuildPhotoReveal.h"
#include "systems/RenderBall.h"
#include "systems/RenderBrick.h"
#include "systems/RenderFPS.h"
#include "systems/RenderFogOfWar.h"
#include "systems/RenderGameUI.h"
#include "systems/RenderLetterboxBars.h"
#include "systems/RenderPhotoReveal.h"
#include "systems/RenderRenderTexture.h"
#include "systems/RenderRoads.h"
#include "systems/RenderSquare.h"
#include "systems/RenderSystemHelpers.h"
#include "systems/RevealFogOfWar.h"
#include "systems/SpawnNewBalls.h"
#include "systems/TestSystem.h"
#include "systems/UpdateBallUpgrades.h"
#include "testing/test_app.h"
#include "testing/test_input.h"
#include "testing/test_macros.h"
#include <afterhours/src/plugins/camera.h>
#include <afterhours/src/plugins/files.h>

#include <afterhours/src/plugins/animation.h>
#include <chrono>
#include <iostream>
#include <thread>

bool running = true;
raylib::RenderTexture2D mainRT;
raylib::RenderTexture2D screenRT;
raylib::Font uiFont;

void game() {
  mainRT =
      raylib::LoadRenderTexture(static_cast<int>(game_constants::WORLD_WIDTH),
                                static_cast<int>(game_constants::WORLD_HEIGHT));
  screenRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                       Settings::get().get_screen_height());
  uiFont = raylib::LoadFont(
      afterhours::files::get_resource_path("fonts", "Gaegu-Bold.ttf")
          .string()
          .c_str());

  afterhours::SystemManager systems;

  {
    afterhours::window_manager::enforce_singletons(systems);
    afterhours::ui::enforce_singletons<InputAction>(systems);
    afterhours::input::enforce_singletons(systems);
    afterhours::camera::enforce_singletons(systems);
  }

  TestSystem *test_system_ptr = nullptr;

  {
    afterhours::input::register_update_systems(systems);
    afterhours::window_manager::register_update_systems(systems);

    systems.register_fixed_update_system(std::make_unique<BallPhysics>());
    systems.register_fixed_update_system(std::make_unique<MazeTraversal>());
    systems.register_fixed_update_system(std::make_unique<LoopDetection>());
    systems.register_fixed_update_system(std::make_unique<HandleCollisions>());
    systems.register_fixed_update_system(
        std::make_unique<RebuildPhotoReveal>());

    systems.register_update_system(std::make_unique<HandleCameraControls>());
    systems.register_update_system(std::make_unique<HandleShopInput>());
    systems.register_update_system(std::make_unique<SpawnNewBalls>());
    systems.register_update_system(std::make_unique<UpdateBallUpgrades>());
    systems.register_update_system(std::make_unique<RevealFogOfWar>());

    auto test_system = std::make_unique<TestSystem>();
    test_system_ptr = test_system.get();
    systems.register_update_system(std::move(test_system));
  }

  {
    systems.register_render_system(std::make_unique<BeginWorldRender>());
    afterhours::camera::register_begin_camera(systems);
    systems.register_render_system(std::make_unique<RenderPhotoReveal>());
    systems.register_render_system(std::make_unique<RenderRoads>());
    systems.register_render_system(std::make_unique<RenderBrick>());
    systems.register_render_system(std::make_unique<RenderBall>());
    systems.register_render_system(std::make_unique<RenderSquare>());
    systems.register_render_system(std::make_unique<RenderFogOfWar>());
    afterhours::camera::register_end_camera(systems);
    systems.register_render_system(std::make_unique<EndWorldRender>());
    systems.register_render_system(
        std::make_unique<BeginPostProcessingRender>());
    systems.register_render_system(std::make_unique<RenderRenderTexture>());
    systems.register_render_system(std::make_unique<RenderLetterboxBars>());
    systems.register_render_system(std::make_unique<RenderGameUI>());
    systems.register_render_system(std::make_unique<RenderFPS>());
    afterhours::ui::register_render_systems<InputAction>(
        systems, InputAction::ToggleUILayoutDebug);
    systems.register_render_system(std::make_unique<EndDrawing>());
  }

  setup_game();

  while (running && !raylib::WindowShouldClose()) {
    if (raylib::IsKeyPressed(raylib::KEY_ESCAPE)) {
      running = false;
    }
    float dt = raylib::GetFrameTime();
    systems.run(dt);

    if (test_system_ptr && test_system_ptr->is_complete()) {
      std::string error = test_system_ptr->get_error();
      if (!error.empty()) {
        std::cout << "Test '" << test_system_ptr->get_test_name()
                  << "' failed: " << error << std::endl;
        running = false;
      } else {
        std::cout << "Test '" << test_system_ptr->get_test_name() << "' passed!"
                  << std::endl;
        running = false;
      }
    }
  }
}

void run_test(const std::string &test_name, bool slow_mode) {
  TestRegistry &registry = TestRegistry::get();
  auto it = registry.tests.find(test_name);
  if (it == registry.tests.end()) {
    std::cout << "Test '" << test_name << "' not found" << std::endl;
    return;
  }

  test_input::slow_test_mode = slow_mode;

  mainRT =
      raylib::LoadRenderTexture(static_cast<int>(game_constants::WORLD_WIDTH),
                                static_cast<int>(game_constants::WORLD_HEIGHT));
  screenRT = raylib::LoadRenderTexture(Settings::get().get_screen_width(),
                                       Settings::get().get_screen_height());
  uiFont = raylib::LoadFont(
      afterhours::files::get_resource_path("fonts", "Gaegu-Bold.ttf")
          .string()
          .c_str());

  afterhours::SystemManager systems;

  {
    afterhours::window_manager::enforce_singletons(systems);
    afterhours::ui::enforce_singletons<InputAction>(systems);
    afterhours::input::enforce_singletons(systems);
  }

  TestSystem *test_system_ptr = nullptr;

  {
    afterhours::input::register_update_systems(systems);
    afterhours::window_manager::register_update_systems(systems);

    auto test_system = std::make_unique<TestSystem>();
    test_system_ptr = test_system.get();
    systems.register_update_system(std::move(test_system));
  }

  {
    systems.register_render_system(std::make_unique<BeginWorldRender>());
    systems.register_render_system(std::make_unique<EndWorldRender>());
    systems.register_render_system(
        std::make_unique<BeginPostProcessingRender>());
    systems.register_render_system(std::make_unique<RenderRenderTexture>());
    systems.register_render_system(std::make_unique<RenderLetterboxBars>());
    systems.register_render_system(std::make_unique<RenderFPS>());
    afterhours::ui::register_render_systems<InputAction>(
        systems, InputAction::ToggleUILayoutDebug);
    systems.register_render_system(std::make_unique<EndDrawing>());
  }

  TestApp test = it->second();
  test_system_ptr->set_test(test_name, std::move(test));

  while (running && !raylib::WindowShouldClose()) {
    if (raylib::IsKeyPressed(raylib::KEY_ESCAPE)) {
      running = false;
    }
    float dt = raylib::GetFrameTime();
    systems.run(dt);

    if (test_input::slow_test_mode) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (test_system_ptr && test_system_ptr->is_complete()) {
      std::string error = test_system_ptr->get_error();
      if (!error.empty()) {
        std::cout << "Test '" << test_system_ptr->get_test_name()
                  << "' failed: " << error << std::endl;
        running = false;
      } else {
        std::cout << "Test '" << test_system_ptr->get_test_name() << "' passed!"
                  << std::endl;
        running = false;
      }
    }
  }
}
