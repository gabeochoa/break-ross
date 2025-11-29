#pragma once

#include "game_constants.h"
#include "log.h"
#include "render_backend.h"
#include "rl.h"
#include "std_include.h"
#include <afterhours/ah.h>
#include <bitset>
#include <magic_enum/magic_enum.hpp>
#include <unordered_map>

struct Transform : afterhours::BaseComponent {
  vec2 position{0.f, 0.f};
  vec2 velocity{0.f, 0.f};
  vec2 size{0.f, 0.f};

  Transform() = default;
  Transform(vec2 pos, vec2 vel, vec2 sz)
      : position(pos), velocity(vel), size(sz) {}
};

enum struct ColliderTag : afterhours::TagId {
  Circle = 0,
  Rect = 1,
  Square = 2,
};

struct HasHealth : afterhours::BaseComponent {
  int max_amount{0};
  int amount{0};

  HasHealth() = default;
  HasHealth(int max_amount_in)
      : max_amount(max_amount_in), amount(max_amount_in) {}
  HasHealth(int max_amount_in, int amount_in)
      : max_amount(max_amount_in), amount(amount_in) {}
};

struct CanDamage : afterhours::BaseComponent {
  afterhours::EntityID id;
  int amount;

  CanDamage() = default;
  CanDamage(afterhours::EntityID id_in, int amount_in)
      : id(id_in), amount(amount_in) {}
};

enum class MazeAlgorithm { WallFollower, Tremaux, DFS, AStar };

struct IsShopManager : afterhours::BaseComponent {
  int car_cost;
  int car_damage;
  int pixels_collected;

  int car_speed_level{0};
  int car_damage_level{0};
  int car_count{20};

  bool shop_open{false};

  IsShopManager() = default;
  IsShopManager(int cost, int damage, int pixels_in)
      : car_cost(cost), car_damage(damage), pixels_collected(pixels_in) {}

  int get_upgrade_cost(int base_cost, int level) const {
    return static_cast<int>(base_cost * std::pow(1.5, level));
  }

  int get_car_speed_cost() const {
    return get_upgrade_cost(50, car_speed_level);
  }

  int get_car_damage_cost() const {
    return get_upgrade_cost(500, car_damage_level);
  }

  int get_new_car_cost() const {
    // TODO scale this cost based on the number of cars t
    return 50;
    //   get_upgrade_cost(100, car_count - 1);
  }

  float get_car_speed_multiplier() const {
    return 1.0f + (car_speed_level * 0.2f);
  }

  int get_car_damage_value() const {
    return car_damage + (car_damage_level * 1);
  }

  bool purchase_car_speed() {
    int cost = get_car_speed_cost();
    if (pixels_collected >= cost) {
      pixels_collected -= cost;
      car_speed_level++;
      return true;
    }
    return false;
  }

  bool purchase_car_damage() {
    int cost = get_car_damage_cost();
    if (pixels_collected >= cost) {
      pixels_collected -= cost;
      car_damage_level++;
      return true;
    }
    return false;
  }

  bool purchase_new_car() {
    int cost = get_new_car_cost();
    log_info(
        "purchase_new_car: cost={}, pixels_collected={}, current_car_count={}",
        cost, pixels_collected, car_count);
    if (pixels_collected >= cost) {
      pixels_collected -= cost;
      car_count++;
      log_info("purchase_new_car: purchase successful! new car_count={}, "
               "remaining_pixels={}",
               car_count, pixels_collected);
      return true;
    }
    log_warn("purchase_new_car: insufficient funds! need {}, have {}", cost,
             pixels_collected);
    return false;
  }

  int maze_algorithm_level{0};

  int get_maze_algorithm_cost() const {
    return get_upgrade_cost(200, maze_algorithm_level);
  }

  bool purchase_maze_algorithm() {
    int cost = get_maze_algorithm_cost();
    if (pixels_collected >= cost) {
      pixels_collected -= cost;
      maze_algorithm_level++;
      return true;
    }
    return false;
  }

  MazeAlgorithm get_current_algorithm() const {
    if (maze_algorithm_level == 0) {
      return MazeAlgorithm::WallFollower;
    } else if (maze_algorithm_level == 1) {
      return MazeAlgorithm::Tremaux;
    } else if (maze_algorithm_level == 2) {
      return MazeAlgorithm::DFS;
    } else {
      return MazeAlgorithm::AStar;
    }
  }
};

struct RevealedRect {
  float x;
  float y;
  float width;
  float height;
};

struct IsPhotoReveal : afterhours::BaseComponent {
  std::bitset<game_constants::GRID_SIZE> revealed_cells;
  std::vector<RevealedRect> merged_rects;
  float cell_size;
  raylib::Texture2D photo_texture{};
  mutable raylib::Texture2D mask_texture{};
  raylib::Shader mask_shader{};
  int mask_shader_mask_loc{-1};
  int mask_shader_mask_scale_loc{-1};
  bool is_loaded{false};
  float reveal_percentage{0.0f};
  bool merged_rects_dirty{false};
  mutable bool mask_texture_dirty{true};

  IsPhotoReveal() = default;
  IsPhotoReveal(float cell_size_in) : cell_size(cell_size_in) {}

  bool is_revealed(int grid_x, int grid_y) const {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return false;
    }
    return revealed_cells[grid_y * game_constants::GRID_WIDTH + grid_x];
  }

  void set_revealed(int grid_x, int grid_y) {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return;
    }
    int idx = grid_y * game_constants::GRID_WIDTH + grid_x;
    if (!revealed_cells[idx]) {
      revealed_cells[idx] = true;
      merged_rects_dirty = true;
      mask_texture_dirty = true;
    }
  }

  void update_mask_texture() const {
    if (!mask_texture_dirty || !is_loaded) {
      return;
    }
    mask_texture_dirty = false;

    std::vector<unsigned char> mask_data(game_constants::GRID_WIDTH *
                                         game_constants::GRID_HEIGHT * 4);
    for (int y = 0; y < game_constants::GRID_HEIGHT; ++y) {
      for (int x = 0; x < game_constants::GRID_WIDTH; ++x) {
        int idx = y * game_constants::GRID_WIDTH + x;
        int pixel_idx = idx * 4;
        unsigned char value =
            revealed_cells[idx] ? static_cast<unsigned char>(255) : 0;
        mask_data[pixel_idx] = value;
        mask_data[pixel_idx + 1] = value;
        mask_data[pixel_idx + 2] = value;
        mask_data[pixel_idx + 3] = value;
      }
    }

    if (mask_texture.id == 0) {
      raylib::Image mask_image = render_backend::GenImageColor(
          game_constants::GRID_WIDTH, game_constants::GRID_HEIGHT,
          raylib::BLACK);
      mask_texture = render_backend::LoadTextureFromImage(mask_image);
      render_backend::UnloadImage(mask_image);
    }

    render_backend::UpdateTexture(mask_texture, mask_data.data());
  }

  void rebuild_merged_rects() {
    if (!merged_rects_dirty) {
      return;
    }
    merged_rects_dirty = false;
    merged_rects.clear();
    std::bitset<game_constants::GRID_SIZE> processed;

    for (int grid_y = 0; grid_y < game_constants::GRID_HEIGHT; ++grid_y) {
      for (int grid_x = 0; grid_x < game_constants::GRID_WIDTH; ++grid_x) {
        int idx = grid_y * game_constants::GRID_WIDTH + grid_x;
        if (processed[idx] || !is_revealed(grid_x, grid_y)) {
          continue;
        }

        int rect_width = 1;
        int rect_height = 1;

        while (grid_x + rect_width < game_constants::GRID_WIDTH &&
               is_revealed(grid_x + rect_width, grid_y) &&
               !processed[grid_y * game_constants::GRID_WIDTH + grid_x +
                          rect_width]) {
          rect_width++;
        }

        bool can_extend_down = true;
        while (can_extend_down &&
               grid_y + rect_height < game_constants::GRID_HEIGHT) {
          for (int x = 0; x < rect_width; ++x) {
            if (!is_revealed(grid_x + x, grid_y + rect_height) ||
                processed[(grid_y + rect_height) * game_constants::GRID_WIDTH +
                          grid_x + x]) {
              can_extend_down = false;
              break;
            }
          }
          if (can_extend_down) {
            rect_height++;
          }
        }

        for (int y = 0; y < rect_height; ++y) {
          for (int x = 0; x < rect_width; ++x) {
            processed[(grid_y + y) * game_constants::GRID_WIDTH + grid_x + x] =
                true;
          }
        }

        RevealedRect rect;
        rect.x = game_constants::BRICK_START_X +
                 grid_x * game_constants::BRICK_CELL_SIZE;
        rect.y = game_constants::BRICK_START_Y +
                 grid_y * game_constants::BRICK_CELL_SIZE;
        rect.width = rect_width * game_constants::BRICK_CELL_SIZE;
        rect.height = rect_height * game_constants::BRICK_CELL_SIZE;
        merged_rects.push_back(rect);
      }
    }
    update_reveal_percentage();
  }

  float get_reveal_percentage() const {
    int revealed_count = 0;
    for (int i = 0; i < game_constants::GRID_SIZE; ++i) {
      if (revealed_cells[i]) {
        revealed_count++;
      }
    }
    return (static_cast<float>(revealed_count) /
            static_cast<float>(game_constants::GRID_SIZE)) *
           100.0f;
  }

  void update_reveal_percentage() {
    reveal_percentage = get_reveal_percentage();
  }
};

struct MergedBrickRect {
  int grid_x;
  int grid_y;
  int width;
  int height;
};

struct BrickGrid : afterhours::BaseComponent {
  std::array<std::array<uint8_t, 50>, game_constants::GRID_HEIGHT> health_data;
  mutable std::vector<MergedBrickRect> cached_rects;
  mutable bool rects_dirty{true};
  mutable raylib::Texture2D health_texture{};
  mutable bool health_texture_dirty{true};

  BrickGrid() {
    for (auto &row : health_data) {
      row.fill(0);
    }
  }

  short get_health(int grid_x, int grid_y) const {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return 0;
    }
    uint8_t byte = health_data[grid_y][grid_x / 2];
    int shift = (grid_x & 1) * 4;
    return (byte >> shift) & 0x0F;
  }

  void set_health(int grid_x, int grid_y, short health) {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return;
    }
    health =
        static_cast<short>(std::max(0, std::min(15, static_cast<int>(health))));

    uint8_t &byte = health_data[grid_y][grid_x / 2];
    int shift = (grid_x & 1) * 4;
    uint8_t inverse_mask = static_cast<uint8_t>(0xF0 >> shift);
    byte =
        (byte & inverse_mask) | static_cast<uint8_t>((health & 0x0F) << shift);
    rects_dirty = true;
    health_texture_dirty = true;
  }

  void add_health(int grid_x, int grid_y, short delta) {
    short current = get_health(grid_x, grid_y);
    set_health(grid_x, grid_y, current + delta);
  }

  bool has_brick(int grid_x, int grid_y) const {
    return get_health(grid_x, grid_y) > 0;
  }
};

struct RoadSegment {
  vec2 start;
  vec2 end;
  float width{2.0f};
};

struct RoadNetwork : afterhours::BaseComponent {
  std::vector<RoadSegment> segments;
  std::vector<bool> visited_segments;
  bool is_loaded{false};

  // Connected component tracking
  std::vector<size_t> component_id; // maps segment index to component ID
  std::vector<std::vector<size_t>>
      components; // list of segments in each component
  std::unordered_map<size_t, size_t> component_sizes; // cache component sizes
  size_t current_component_id{SIZE_MAX};

  // Precomputed segment connections for fast candidate lookup
  // For each segment, stores two vectors: [0] = connections from start, [1] =
  // connections from end Each connection is (connected_segment_index,
  // use_reverse)
  std::vector<std::array<std::vector<std::pair<size_t, bool>>, 2>>
      segment_connections;

  RoadNetwork() = default;

  void mark_visited(size_t segment_index) {
    if (segment_index < visited_segments.size()) {
      visited_segments[segment_index] = true;
    }
  }

  bool is_visited(size_t segment_index) const {
    if (segment_index >= visited_segments.size()) {
      return false;
    }
    return visited_segments[segment_index];
  }

  size_t find_random_unvisited_segment() const {
    std::vector<size_t> unvisited;
    for (size_t i = 0; i < segments.size(); ++i) {
      if (!is_visited(i)) {
        unvisited.push_back(i);
      }
    }
    if (unvisited.empty()) {
      return SIZE_MAX;
    }
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, unvisited.size() - 1);
    return unvisited[dist(rng)];
  }

  void build_connected_components(float connection_tolerance) {
    if (segments.empty()) {
      return;
    }

    component_id.clear();
    component_id.resize(segments.size(), SIZE_MAX);
    components.clear();
    component_sizes.clear();
    segment_connections.clear();
    segment_connections.resize(segments.size());

    size_t next_component_id = 0;

    // Build adjacency list and precompute connections
    std::vector<std::vector<size_t>> adjacency(segments.size());

    for (size_t i = 0; i < segments.size(); ++i) {
      const RoadSegment &seg_i = segments[i];
      vec2 seg_i_start = seg_i.start;
      vec2 seg_i_end = seg_i.end;

      for (size_t j = i + 1; j < segments.size(); ++j) {
        const RoadSegment &seg_j = segments[j];
        vec2 seg_j_start = seg_j.start;
        vec2 seg_j_end = seg_j.end;

        // Check all endpoint combinations
        float dist_ss = std::sqrt(
            (seg_i_start.x - seg_j_start.x) * (seg_i_start.x - seg_j_start.x) +
            (seg_i_start.y - seg_j_start.y) * (seg_i_start.y - seg_j_start.y));
        float dist_se = std::sqrt(
            (seg_i_start.x - seg_j_end.x) * (seg_i_start.x - seg_j_end.x) +
            (seg_i_start.y - seg_j_end.y) * (seg_i_start.y - seg_j_end.y));
        float dist_es = std::sqrt(
            (seg_i_end.x - seg_j_start.x) * (seg_i_end.x - seg_j_start.x) +
            (seg_i_end.y - seg_j_start.y) * (seg_i_end.y - seg_j_start.y));
        float dist_ee = std::sqrt(
            (seg_i_end.x - seg_j_end.x) * (seg_i_end.x - seg_j_end.x) +
            (seg_i_end.y - seg_j_end.y) * (seg_i_end.y - seg_j_end.y));

        if (dist_ss < connection_tolerance || dist_se < connection_tolerance ||
            dist_es < connection_tolerance || dist_ee < connection_tolerance) {
          adjacency[i].push_back(j);
          adjacency[j].push_back(i);

          // Precompute connections from seg_i's END (index 1)
          // seg_i.end connects to seg_j.start -> use seg_j normally
          // (reverse=false) seg_i.end connects to seg_j.end -> use seg_j
          // reversed (reverse=true)
          if (dist_es < connection_tolerance) {
            segment_connections[i][1].push_back({j, false});
          }
          if (dist_ee < connection_tolerance) {
            segment_connections[i][1].push_back({j, true});
          }

          // Precompute connections from seg_i's START (index 0)
          // seg_i.start connects to seg_j.start -> use seg_j normally
          // (reverse=false) seg_i.start connects to seg_j.end -> use seg_j
          // reversed (reverse=true)
          if (dist_ss < connection_tolerance) {
            segment_connections[i][0].push_back({j, false});
          }
          if (dist_se < connection_tolerance) {
            segment_connections[i][0].push_back({j, true});
          }

          // Same for seg_j: connections from seg_j's END
          if (dist_ee < connection_tolerance) {
            segment_connections[j][1].push_back({i, true});
          }
          if (dist_se < connection_tolerance) {
            segment_connections[j][1].push_back({i, false});
          }

          // Connections from seg_j's START
          if (dist_ss < connection_tolerance) {
            segment_connections[j][0].push_back({i, false});
          }
          if (dist_es < connection_tolerance) {
            segment_connections[j][0].push_back({i, true});
          }
        }
      }
    }

    // DFS to find connected components
    std::vector<bool> visited(segments.size(), false);
    for (size_t i = 0; i < segments.size(); ++i) {
      if (!visited[i]) {
        std::vector<size_t> component;
        std::vector<size_t> stack;
        stack.push_back(i);
        visited[i] = true;

        while (!stack.empty()) {
          size_t current = stack.back();
          stack.pop_back();
          component.push_back(current);
          component_id[current] = next_component_id;

          for (size_t neighbor : adjacency[current]) {
            if (!visited[neighbor]) {
              visited[neighbor] = true;
              stack.push_back(neighbor);
            }
          }
        }

        components.push_back(component);
        component_sizes[next_component_id] = component.size();
        next_component_id++;
      }
    }
  }

  size_t get_component_id(size_t segment_index) const {
    if (segment_index >= component_id.size()) {
      return SIZE_MAX;
    }
    return component_id[segment_index];
  }

  size_t get_component_size(size_t comp_id) const {
    auto it = component_sizes.find(comp_id);
    if (it != component_sizes.end()) {
      return it->second;
    }
    return 0;
  }

  size_t get_visited_count_in_component(size_t comp_id) const {
    if (comp_id >= components.size()) {
      return 0;
    }
    size_t count = 0;
    for (size_t seg_idx : components[comp_id]) {
      if (is_visited(seg_idx)) {
        count++;
      }
    }
    return count;
  }

  bool is_component_complete(size_t comp_id) const {
    if (comp_id >= components.size()) {
      return true;
    }
    size_t total = get_component_size(comp_id);
    size_t visited = get_visited_count_in_component(comp_id);
    return total > 0 && visited == total;
  }

  std::vector<size_t> get_unvisited_in_component(size_t comp_id) const {
    std::vector<size_t> unvisited;
    if (comp_id >= components.size()) {
      return unvisited;
    }
    for (size_t seg_idx : components[comp_id]) {
      if (!is_visited(seg_idx)) {
        unvisited.push_back(seg_idx);
      }
    }
    return unvisited;
  }
};

struct FogOfWar : afterhours::BaseComponent {
  std::bitset<game_constants::GRID_SIZE> revealed_cells;
  std::bitset<game_constants::GRID_SIZE> reachable_cells;
  float reveal_radius{50.0f};
  bool is_dirty{false};
  bool reachable_computed{false};

  FogOfWar() = default;

  bool is_revealed(int grid_x, int grid_y) const {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return false;
    }
    return revealed_cells[grid_y * game_constants::GRID_WIDTH + grid_x];
  }

  void set_revealed(int grid_x, int grid_y) {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return;
    }
    int idx = grid_y * game_constants::GRID_WIDTH + grid_x;
    if (!revealed_cells[idx]) {
      revealed_cells[idx] = true;
      is_dirty = true;
    }
  }

  bool is_reachable(int grid_x, int grid_y) const {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return false;
    }
    return reachable_cells[grid_y * game_constants::GRID_WIDTH + grid_x];
  }

  void set_reachable(int grid_x, int grid_y) {
    if (grid_x < 0 || grid_x >= game_constants::GRID_WIDTH || grid_y < 0 ||
        grid_y >= game_constants::GRID_HEIGHT) {
      return;
    }
    int idx = grid_y * game_constants::GRID_WIDTH + grid_x;
    reachable_cells[idx] = true;
  }

  bool are_all_reachable_revealed() const {
    for (int i = 0; i < game_constants::GRID_SIZE; ++i) {
      if (reachable_cells[i] && !revealed_cells[i]) {
        return false;
      }
    }
    return true;
  }

  void reveal_all_unreachable() {
    for (int i = 0; i < game_constants::GRID_SIZE; ++i) {
      if (!reachable_cells[i] && !revealed_cells[i]) {
        revealed_cells[i] = true;
        is_dirty = true;
      }
    }
  }

  float get_reveal_percentage() const {
    int revealed_count = 0;
    for (int i = 0; i < game_constants::GRID_SIZE; ++i) {
      if (revealed_cells[i]) {
        revealed_count++;
      }
    }
    return (static_cast<float>(revealed_count) /
            static_cast<float>(game_constants::GRID_SIZE)) *
           100.0f;
  }
};

struct RoadFollowing : afterhours::BaseComponent {
  size_t current_segment_index{0};
  float progress_along_segment{0.0f};
  float speed{100.0f};
  bool reverse_direction{false};

  vec2 last_position{0.0f, 0.0f};
  size_t segments_without_reveal{0};  // Count consecutive visited segments
  float forced_direction_timer{0.0f}; // Timer for forced direction mode
  size_t forced_direction_steps{0};   // Steps remaining in forced direction
  vec2 forced_direction{0.0f, 0.0f}; // Direction to maintain during forced mode
  size_t forced_direction_attempts{
      0}; // Count how many times we've tried forced direction
  MazeAlgorithm current_algorithm{MazeAlgorithm::WallFollower};

  std::vector<size_t>
      segment_history; // Track last 10 segments for loop detection
  static constexpr size_t MAX_HISTORY_SIZE = 10;

  static constexpr size_t LOOP_DETECTION_THRESHOLD =
      10; // Consecutive visited segments before loop detected (increased)
  static constexpr size_t FORCED_DIRECTION_STEPS =
      20; // Steps to force direction when breaking loop (increased)
  static constexpr size_t MAX_FORCED_DIRECTION_ATTEMPTS =
      5; // Max forced direction attempts before jumping to unvisited segment
         // (increased)

  RoadFollowing() = default;
  RoadFollowing(float speed_in) : speed(speed_in) {}
};
