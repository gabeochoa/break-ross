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
#include <limits>
#include <nlohmann/json.hpp>
#include <unordered_map>

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

afterhours::Entity &make_car(vec2 position, vec2 velocity, float radius,
                             int damage) {
  afterhours::Entity &car = afterhours::EntityHelper::createEntity();
  car.addComponent<Transform>(position, velocity,
                              vec2{radius * 2.0f, radius * 2.0f});
  car.enableTag(ColliderTag::Circle);
  car.addComponent<CanDamage>(car.id, damage);

  RoadNetwork *road_network =
      afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
  IsShopManager *shop =
      afterhours::EntityHelper::get_singleton_cmp<IsShopManager>();
  invariant(shop, "IsShopManager singleton not found");

  float speed = 250.0f;
  speed *= shop->get_car_speed_multiplier();

  RoadFollowing &road_following = car.addComponent<RoadFollowing>(speed);

  if (!road_network || !road_network->is_loaded ||
      road_network->segments.empty()) {
    road_following.current_segment_index = 0;
    road_following.progress_along_segment = 0.0f;
    return car;
  }
  size_t nearest_segment = 0;
  float min_dist_sq = std::numeric_limits<float>::max();

  for (size_t i = 0; i < road_network->segments.size(); ++i) {
    const RoadSegment &seg = road_network->segments[i];
    vec2 seg_start = seg.start;
    vec2 seg_end = seg.end;
    vec2 seg_dir = {seg_end.x - seg_start.x, seg_end.y - seg_start.y};
    float seg_len_sq = seg_dir.x * seg_dir.x + seg_dir.y * seg_dir.y;

    if (seg_len_sq < 0.001f) {
      float dist_sq = (position.x - seg_start.x) * (position.x - seg_start.x) +
                      (position.y - seg_start.y) * (position.y - seg_start.y);
      if (dist_sq < min_dist_sq) {
        min_dist_sq = dist_sq;
        nearest_segment = i;
      }
      continue;
    }

    float t =
        std::max(0.0f, std::min(1.0f, ((position.x - seg_start.x) * seg_dir.x +
                                       (position.y - seg_start.y) * seg_dir.y) /
                                          seg_len_sq));

    vec2 closest_point = {seg_start.x + t * seg_dir.x,
                          seg_start.y + t * seg_dir.y};
    float dist_sq =
        (position.x - closest_point.x) * (position.x - closest_point.x) +
        (position.y - closest_point.y) * (position.y - closest_point.y);

    if (dist_sq < min_dist_sq) {
      min_dist_sq = dist_sq;
      nearest_segment = i;

      vec2 seg_to_pos = {position.x - seg_start.x, position.y - seg_start.y};
      float dot = seg_dir.x * seg_to_pos.x + seg_dir.y * seg_to_pos.y;
      road_following.reverse_direction = (dot < 0.0f);
    }
  }

  road_following.current_segment_index = nearest_segment;

  const RoadSegment &seg = road_network->segments[nearest_segment];
  vec2 seg_start = road_following.reverse_direction ? seg.end : seg.start;
  vec2 seg_end = road_following.reverse_direction ? seg.start : seg.end;
  vec2 seg_dir = {seg_end.x - seg_start.x, seg_end.y - seg_start.y};
  float seg_len = std::sqrt(seg_dir.x * seg_dir.x + seg_dir.y * seg_dir.y);

  if (seg_len > 0.001f) {
    vec2 to_pos = {position.x - seg_start.x, position.y - seg_start.y};
    float t =
        (to_pos.x * seg_dir.x + to_pos.y * seg_dir.y) / (seg_len * seg_len);
    road_following.progress_along_segment = std::max(0.0f, std::min(1.0f, t));
  } else {
    road_following.progress_along_segment = 0.0f;
  }

  return car;
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

      if (seg_json.contains("road_type")) {
        std::string road_type_str = seg_json["road_type"].get<std::string>();
        if (road_type_str == "highway") {
          segment.road_type = RoadType::Highway;
        } else if (road_type_str == "primary") {
          segment.road_type = RoadType::Primary;
        } else if (road_type_str == "secondary") {
          segment.road_type = RoadType::Secondary;
        } else {
          segment.road_type = RoadType::Residential;
        }
      } else {
        segment.road_type = RoadType::Residential;
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
    RoadSegment seg;
    seg.start = {x, 0.0f};
    seg.end = {x, world_height};
    seg.width = 3.0f;
    seg.road_type = RoadType::Primary;
    road_network.segments.push_back(seg);
  }

  for (int i = 0; i <= num_vertical; ++i) {
    float y = i * grid_spacing;
    RoadSegment seg;
    seg.start = {0.0f, y};
    seg.end = {world_width, y};
    seg.width = 3.0f;
    seg.road_type = RoadType::Primary;
    road_network.segments.push_back(seg);
  }

  for (int i = 0; i < num_horizontal; ++i) {
    for (int j = 0; j < num_vertical; ++j) {
      float x = i * grid_spacing;
      float y = j * grid_spacing;

      if (i < num_horizontal - 1) {
        RoadSegment seg;
        seg.start = {x, y};
        seg.end = {x + grid_spacing, y + grid_spacing * 0.5f};
        seg.width = 2.0f;
        seg.road_type = RoadType::Residential;
        road_network.segments.push_back(seg);
      }
      if (j < num_vertical - 1) {
        RoadSegment seg;
        seg.start = {x, y};
        seg.end = {x + grid_spacing * 0.5f, y + grid_spacing};
        seg.width = 2.0f;
        seg.road_type = RoadType::Residential;
        road_network.segments.push_back(seg);
      }
    }
  }

  road_network.visited_segments.resize(road_network.segments.size(), false);
  road_network.is_loaded = true;
}

static void spawn_pois(RoadNetwork *road_network) {
  invariant(road_network, "RoadNetwork singleton not found");
  if (!road_network->is_loaded || road_network->segments.empty()) {
    return;
  }

  const float tolerance = 10.0f;
  std::vector<std::pair<vec2, int>> intersection_points;

  for (size_t i = 0; i < road_network->segments.size(); ++i) {
    const RoadSegment &seg = road_network->segments[i];
    vec2 start = seg.start;
    vec2 end = seg.end;

    bool start_found = false;
    bool end_found = false;

    for (auto &[pos, count] : intersection_points) {
      float start_dist_sq = (start.x - pos.x) * (start.x - pos.x) +
                            (start.y - pos.y) * (start.y - pos.y);
      float end_dist_sq =
          (end.x - pos.x) * (end.x - pos.x) + (end.y - pos.y) * (end.y - pos.y);

      if (start_dist_sq < tolerance * tolerance) {
        count++;
        start_found = true;
      }
      if (end_dist_sq < tolerance * tolerance) {
        count++;
        end_found = true;
      }
    }

    if (!start_found) {
      intersection_points.push_back({start, 1});
    }
    if (!end_found) {
      intersection_points.push_back({end, 1});
    }
  }

  int poi_count = 0;
  int landmark_count = 0;
  int city_count = 0;

  for (const auto &[pos, connection_count] : intersection_points) {
    if (connection_count >= 3) {
      POIType poi_type = POIType::Area;
      int reward = 10;

      if (connection_count >= 5 && landmark_count < 3) {
        poi_type = POIType::Landmark;
        reward = 100;
        landmark_count++;
      } else if (connection_count >= 4 && city_count < 5) {
        poi_type = POIType::City;
        reward = 50;
        city_count++;
      }

      afterhours::Entity &poi = afterhours::EntityHelper::createEntity();
      poi.addComponent<PointOfInterest>(pos, poi_type, reward);
      poi_count++;

      if (poi_count >= 20) {
        break;
      }
    }
  }

  log_info("Spawned {} POIs ({} landmarks, {} cities)", poi_count,
           landmark_count, city_count);
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
  invariant(camera, "HasCamera singleton not found");
  camera->set_position({game_constants::WORLD_WIDTH * 0.5f,
                        game_constants::WORLD_HEIGHT * 0.5f});
  camera->set_offset({game_constants::WORLD_WIDTH * 0.5f,
                      game_constants::WORLD_HEIGHT * 0.5f});
  camera->set_zoom(0.75f);

  IsPhotoReveal *photo_reveal =
      afterhours::EntityHelper::get_singleton_cmp<IsPhotoReveal>();
  invariant(photo_reveal, "IsPhotoReveal singleton not found");
  if (photo_reveal->is_loaded) {
    return;
  }
  std::filesystem::path photo_path = afterhours::files::get_resource_path(
      "images/photos", "test_photo_500x500.png");
  photo_reveal->photo_texture =
      render_backend::LoadTexture(photo_path.string().c_str());
  render_backend::SetTextureFilter(photo_reveal->photo_texture,
                                   raylib::TEXTURE_FILTER_POINT);

  std::filesystem::path vs_path = afterhours::files::get_resource_path(
      "shaders", "photo_reveal_vertex.glsl");
  std::filesystem::path fs_path = afterhours::files::get_resource_path(
      "shaders", "photo_reveal_fragment.glsl");
  photo_reveal->mask_shader = render_backend::LoadShader(
      vs_path.string().c_str(), fs_path.string().c_str());

  if (photo_reveal->mask_shader.id != 0) {
    photo_reveal->mask_shader_mask_loc = render_backend::GetShaderLocation(
        photo_reveal->mask_shader, "maskTexture");
    photo_reveal->mask_shader_mask_scale_loc =
        render_backend::GetShaderLocation(photo_reveal->mask_shader,
                                          "maskScale");
  }

  photo_reveal->is_loaded = true;

  RoadNetwork *road_network =
      afterhours::EntityHelper::get_singleton_cmp<RoadNetwork>();
  invariant(road_network, "RoadNetwork singleton not found");
  if (road_network->is_loaded) {
    return;
  }
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
  if (road_network->segments.empty()) {
    spawn_pois(road_network);
    return;
  }

  road_network->build_connected_components(15.0f);
  if (road_network->segments.empty()) {
    spawn_pois(road_network);
    return;
  }

  road_network->current_component_id = road_network->get_component_id(0);
  log_info("Built {} connected components, starting in component {}",
           road_network->components.size(), road_network->current_component_id);

  spawn_pois(road_network);

  float square_size = 12.0f;
  float square_speed = 250.0f;

  vec2 square_start_position{0.0f, 0.0f};
  size_t initial_segment_index = 0;
  if (road_network->segments.empty()) {
    square_start_position = vec2{game_constants::WORLD_WIDTH * 0.5f,
                                 game_constants::WORLD_HEIGHT * 0.5f};
    initial_segment_index = 0;
  } else {
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
  }

  make_square(square_start_position, square_size, square_speed,
              initial_segment_index);
}
