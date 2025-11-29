#include "game_setup.h"

#include "components.h"
#include "eq.h"
#include "game_constants.h"
#include "render_backend.h"
#include "settings.h"
#include <afterhours/ah.h>
#include <afterhours/src/plugins/autolayout.h>
#include <afterhours/src/plugins/camera.h>
#include <afterhours/src/plugins/files.h>
#include <algorithm>
#include <cmath>
#include <nlohmann/json.hpp>

template <typename Component, typename... Args>
static void addIfMissing(afterhours::Entity &entity, Args &&...args) {
  if (!entity.has<Component>()) {
    entity.addComponent<Component>(std::forward<Args>(args)...);
    afterhours::EntityHelper::registerSingleton<Component>(entity);
  }
}

static afterhours::Entity &get_sophie() {
  afterhours::Entity &sophie =
      afterhours::EntityHelper::get_singleton<afterhours::ui::AutoLayoutRoot>();
  if (!sophie.has<afterhours::ui::AutoLayoutRoot>()) {
    log_error("Sophie entity not found - make_singleton() must be called "
              "before setup_game()");
  }
  return sophie;
}

afterhours::Entity &make_ball(vec2 position, vec2 velocity, float radius,
                              int damage) {
  afterhours::Entity &ball = afterhours::EntityHelper::createEntity();
  ball.addComponent<Transform>(position, velocity,
                               vec2{radius * 2.0f, radius * 2.0f});
  ball.enableTag(ColliderTag::Circle);
  ball.addComponent<CanDamage>(ball.id, damage);
  return ball;
}

afterhours::Entity &make_square(vec2 position, float size, float speed,
                                size_t initial_segment_index) {
  afterhours::Entity &square = afterhours::EntityHelper::createEntity();
  square.addComponent<Transform>(position, vec2{0.0f, 0.0f}, vec2{size, size});
  square.enableTag(ColliderTag::Square);
  RoadFollowing &road_following = square.addComponent<RoadFollowing>(speed);
  road_following.current_segment_index = initial_segment_index;
  return square;
}

static bool
load_road_network_from_json(RoadNetwork &road_network,
                            const std::filesystem::path &json_path) {
  std::ifstream ifs(json_path);
  if (!ifs.is_open()) {
    return false;
  }

  try {
    nlohmann::json j;
    ifs >> j;
    ifs.close();

    if (!j.contains("segments") || !j["segments"].is_array()) {
      return false;
    }

    road_network.segments.clear();
    for (const auto &seg_json : j["segments"]) {
      if (!seg_json.contains("start") || !seg_json.contains("end")) {
        continue;
      }

      RoadSegment segment;
      segment.start.x = seg_json["start"]["x"].get<float>();
      segment.start.y = seg_json["start"]["y"].get<float>();
      segment.end.x = seg_json["end"]["x"].get<float>();
      segment.end.y = seg_json["end"]["y"].get<float>();

      if (seg_json.contains("width")) {
        segment.width = seg_json["width"].get<float>();
      } else {
        segment.width = 2.0f;
      }

      road_network.segments.push_back(segment);
    }

    road_network.visited_segments.resize(road_network.segments.size(), false);
    road_network.is_loaded = true;
    return true;
  } catch (const std::exception &e) {
    log_error("Failed to parse road network JSON: %s", e.what());
    return false;
  }
}

static void create_simple_road_network(RoadNetwork &road_network) {
  road_network.segments.clear();

  float world_width = game_constants::WORLD_WIDTH;
  float world_height = game_constants::WORLD_HEIGHT;

  float grid_spacing = 200.0f;
  int num_horizontal = static_cast<int>(world_width / grid_spacing);
  int num_vertical = static_cast<int>(world_height / grid_spacing);

  for (int i = 0; i <= num_horizontal; ++i) {
    float x = i * grid_spacing;
    road_network.segments.push_back({{x, 0.0f}, {x, world_height}, 3.0f});
  }

  for (int i = 0; i <= num_vertical; ++i) {
    float y = i * grid_spacing;
    road_network.segments.push_back({{0.0f, y}, {world_width, y}, 3.0f});
  }

  for (int i = 0; i < num_horizontal; ++i) {
    for (int j = 0; j < num_vertical; ++j) {
      float x = i * grid_spacing;
      float y = j * grid_spacing;

      if (i < num_horizontal - 1) {
        road_network.segments.push_back(
            {{x, y}, {x + grid_spacing, y + grid_spacing * 0.5f}, 2.0f});
      }
      if (j < num_vertical - 1) {
        road_network.segments.push_back(
            {{x, y}, {x + grid_spacing * 0.5f, y + grid_spacing}, 2.0f});
      }
    }
  }

  road_network.visited_segments.resize(road_network.segments.size(), false);
  road_network.is_loaded = true;
}

void setup_game() {
  afterhours::Entity &sophie = get_sophie();

  addIfMissing<IsShopManager>(sophie, 100, 1, 100);
  addIfMissing<IsPhotoReveal>(sophie, game_constants::BRICK_CELL_SIZE);
  addIfMissing<BrickGrid>(sophie);
  addIfMissing<RoadNetwork>(sophie);
  addIfMissing<FogOfWar>(sophie);
  addIfMissing<afterhours::camera::HasCamera>(sophie);

  afterhours::camera::HasCamera *camera =
      afterhours::EntityHelper::get_singleton_cmp<
          afterhours::camera::HasCamera>();
  if (camera) {
    camera->set_position({game_constants::WORLD_WIDTH * 0.5f,
                          game_constants::WORLD_HEIGHT * 0.5f});
    camera->set_offset({game_constants::WORLD_WIDTH * 0.5f,
                        game_constants::WORLD_HEIGHT * 0.5f});
    camera->set_zoom(0.75f);
  }

  IsPhotoReveal *photo_reveal =
      afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
  if (photo_reveal && !photo_reveal->is_loaded) {
    std::filesystem::path photo_path = afterhours::files::get_resource_path(
        "images/photos", "test_photo_500x500.png");
    photo_reveal->photo_texture =
        render_backend::LoadTexture(photo_path.string().c_str());
    render_backend::SetTextureFilter(photo_reveal->photo_texture,
                                     raylib::TEXTURE_FILTER_BILINEAR);
    photo_reveal->is_loaded = true;
  }

  RoadNetwork *road_network =
      afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
  if (road_network && !road_network->is_loaded) {
    std::filesystem::path nyc_roads_path =
        afterhours::files::get_resource_path("", "nyc_roads.json");
    if (!load_road_network_from_json(*road_network, nyc_roads_path)) {
      log_info("NYC roads not found, using procedural road network");
      create_simple_road_network(*road_network);
    } else {
      log_info("Loaded NYC road network with {} segments",
               road_network->segments.size());
    }

    // Build connected components
    // Use tolerance matching road width (square size = 12.0, so ~15.0 for
    // connections)
    if (road_network && !road_network->segments.empty()) {
      road_network->build_connected_components(15.0f);
      if (!road_network->segments.empty()) {
        road_network->current_component_id = road_network->get_component_id(0);
        log_info("Built {} connected components, starting in component {}",
                 road_network->components.size(),
                 road_network->current_component_id);
      }
    }
  }

  float square_size = 12.0f;
  float square_speed = 250.0f;

  vec2 square_start_position{0.0f, 0.0f};
  size_t initial_segment_index = 0;
  if (road_network && !road_network->segments.empty()) {
    // For debugging: spawn near the problematic spot (segment 745)
    // If segment 745 exists, use it; otherwise fall back to segment 0
    size_t debug_segment = 745;
    if (debug_segment < road_network->segments.size()) {
      square_start_position = road_network->segments[debug_segment].start;
      initial_segment_index = debug_segment;
      log_info("DEBUG: Spawning square at segment {} (problematic area) - "
               "position=({:.1f}, {:.1f})",
               debug_segment, square_start_position.x, square_start_position.y);
    } else {
      square_start_position = road_network->segments[0].start;
      initial_segment_index = 0;
    }
  } else {
    square_start_position = vec2{game_constants::WORLD_WIDTH * 0.5f,
                                 game_constants::WORLD_HEIGHT * 0.5f};
    initial_segment_index = 0;
  }

  make_square(square_start_position, square_size, square_speed,
              initial_segment_index);
}
