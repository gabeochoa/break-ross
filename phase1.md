# Phase 1: Core Gameplay Foundation

## Overview
Implement the foundational gameplay systems using the afterhours ECS architecture. This includes autonomous ball physics, brick grid, collision detection, basic rendering, and minimal economy to purchase one starting ball.

## Components to Create

### Core Components (`src/components.h`)
- **Transform**: `struct Transform : BaseComponent { vec2 position; vec2 velocity; vec2 size; }` - Combined position, velocity, and size (inspired by kart-afterhours pattern)
- **HasCollider**: `struct HasCollider : BaseComponent { enum class Shape { Circle, Rect }; Shape shape; }` - Collision shape component
- **HasHealth**: `struct HasHealth : BaseComponent { int max_amount; int amount; }` - Health component (used by bricks, from kart-afterhours pattern)
- **CanDamage**: `struct CanDamage : BaseComponent { EntityID id; int amount; }` - Damage component (used by balls, from kart-afterhours pattern)
- **IsShopManager**: `struct IsShopManager : BaseComponent { int ball_cost; int ball_damage; int pixels; }` - Shop configuration and state (singleton)

## Systems to Create

### 1. Ball Physics System (`src/systems/BallPhysics.h`)
- **Purpose**: Update ball position and handle wall bouncing
- **Components**: `System<Transform, HasCollider>` where `HasCollider.shape == Circle`
- **Logic**:
  - Query for all entities with `Transform` and `HasCollider` where shape is Circle
  - Update position: `transform.position += transform.velocity * dt`
  - Check wall collisions (screen bounds or game area bounds)
  - Reflect velocity on wall collision (invert x or y component)
  - Keep ball within bounds

### 2. Handle Collisions System (`src/systems/HandleCollisions.h`)
- **Purpose**: Detect and handle all collisions in the game (ball-brick, ball-wall, etc.)
- **Components**: Query for all entities with `Transform` and `HasCollider`
- **Logic**:
  - Get all entities with `Transform` and `HasCollider`
  - For each pair of colliding entities:
    - Circle-Circle: Check distance between centers vs sum of radii
    - Circle-Rect: Check if circle overlaps AABB
    - Rect-Rect: Check AABB overlap
  - Ball-Brick collisions (Circle with Rect that has HasHealth):
    - Reflect ball velocity on collision
    - If ball has `CanDamage`, apply damage to brick's `HasHealth`
    - Reduce `HasHealth.amount` by `CanDamage.amount`
    - If `HasHealth.amount <= 0`, mark entity for cleanup
  - Ball-Wall collisions handled in BallPhysics system (or could be moved here)
  - Extensible for future collision types (pixels, powerups, etc.)

### 3. Cleanup Dead Bricks System (`src/systems/CleanupDeadBricks.h`)
- **Purpose**: Cleanup bricks when health reaches 0
- **Components**: `System<Transform, HasHealth>`
- **Logic**:
  - Query for entities with `Transform` and `HasHealth`
  - Check if `hasHealth.amount <= 0`
  - Mark entity for cleanup: `entity.cleanup = true`
  - Note: Pixels will drop in Phase 2, so no pixel spawning here

### 4. Ball Rendering System (`src/systems/RenderBall.h`)
- **Purpose**: Render ball entities
- **Components**: `System<Transform, HasCollider>` (const) where shape is Circle
- **Logic**:
  - Query for all entities with `Transform` and `HasCollider` where shape is Circle
  - Calculate radius: `transform.size.x / 2` (assuming square size)
  - Draw circle at `transform.position + vec2{radius, radius}` with radius
  - Use `raylib::DrawCircleV()` or `DrawCircle()`

### 5. Brick Rendering System (`src/systems/RenderBrick.h`)
- **Purpose**: Render brick grid
- **Components**: `System<Transform, HasCollider, HasHealth>` (const) where shape is Rect
- **Logic**:
  - Query for all entities with `Transform`, `HasCollider` (Rect), and `HasHealth` where `hasHealth.amount > 0`
  - Draw rectangles using `transform.position` and `transform.size`
  - Use `raylib::DrawRectangleV()` or `DrawRectangle()`
  - Simple color (e.g., GRAY or WHITE) - health-based coloring in Phase 4

## Game Initialization

### Create Game Setup Function (`src/game_setup.h` / `src/game_setup.cpp`)
- **Function**: `void setup_game()`
- **Logic**:
  - Create IsShopManager singleton entity:
    ```cpp
    auto &shop_entity = EntityHelper::createPermanentEntity();
    shop_entity.addComponent<IsShopManager>(100, 1, 100); // ball_cost=100, ball_damage=1, pixels=100
    EntityHelper::registerSingleton<IsShopManager>(shop_entity);
    ```
    - Start with `pixels = ball_cost` (enough for one ball)
  
  - Create starting ball entity:
    ```cpp
    auto &shop = EntityHelper::get_singleton_cmp<IsShopManager>();
    auto &ball = EntityHelper::createEntity();
    float radius = 10.0f;
    ball.addComponent<Transform>(
      vec2{screen_width/2.0f, screen_height/2.0f}, // position
      vec2{200.0f, 200.0f}, // velocity (initial speed)
      vec2{radius * 2.0f, radius * 2.0f} // size (diameter for circle)
    );
    ball.addComponent<HasCollider>(HasCollider::Shape::Circle);
    ball.addComponent<CanDamage>(ball.id, shop->ball_damage);
    ```
  
  - Create brick grid:
    - Calculate grid layout (e.g., 10 columns × 5 rows)
    - Brick size: e.g., 60×30 pixels
    - Spacing: e.g., 5 pixels between bricks
    - For each brick position, create entity:
      ```cpp
      auto &brick = EntityHelper::createEntity();
      brick.addComponent<Transform>(
        vec2{x, y}, // position
        vec2{0, 0}, // velocity (bricks don't move)
        vec2{brick_width, brick_height} // size
      );
      brick.addComponent<HasCollider>(HasCollider::Shape::Rect);
      brick.addComponent<HasHealth>(1, 1); // max=1, current=1 for Phase 1
      ```

## System Registration

### Update `src/game.cpp`
- Register new systems in appropriate order:
  ```cpp
  // Update systems (order matters)
  systems.register_update_system(std::make_unique<BallPhysics>());
  systems.register_update_system(std::make_unique<HandleCollisions>());
  systems.register_update_system(std::make_unique<CleanupDeadBricks>());
  
  // Render systems
  systems.register_render_system(std::make_unique<BeginWorldRender>());
  systems.register_render_system(std::make_unique<RenderBrick>());
  systems.register_render_system(std::make_unique<RenderBall>());
  systems.register_render_system(std::make_unique<EndWorldRender>());
  // ... existing render systems
  ```
- Call `setup_game()` after system registration, before game loop

## File Structure
- `src/components.h` - Add new component definitions (Transform, HasCollider, HasHealth, CanDamage, IsShopManager)
- `src/systems/BallPhysics.h` - Ball movement and wall bouncing
- `src/systems/HandleCollisions.h` - Collision detection, damage application
- `src/systems/CleanupDeadBricks.h` - Cleanup bricks when health <= 0
- `src/systems/RenderBall.h` - Ball rendering
- `src/systems/RenderBrick.h` - Brick rendering
- `src/game_setup.h` / `src/game_setup.cpp` - Game initialization

## Implementation Notes
- Use `Transform` component combining position, velocity, and size (following kart-afterhours pattern)
- Use `HasCollider` with Shape enum (Circle for balls, Rect for bricks)
- Use `HasHealth` for bricks (check `amount <= 0` for destruction, no IsDestroyed needed)
- Use `CanDamage` for balls (from kart-afterhours pattern)
- Use `IsShopManager` component on singleton entity (merged ShopManager into IsShopManager)
- Register singleton with `EntityHelper::registerSingleton<IsShopManager>(entity)`
- Access singleton with `EntityHelper::get_singleton_cmp<IsShopManager>()`
- Use `EntityQuery` with `whereHasComponent<>()` and `whereMissingComponent<>()` for filtering
- Use `EQ().gen()` pattern for iterating entities (as per project rules)
- Follow naming conventions: systems use PascalCase, components use PascalCase
- Keep systems focused and single-purpose
- Transform.size for balls: diameter (radius * 2) for circle colliders
- Transform.size for bricks: actual brick dimensions for rect colliders
- Camera/viewport can be added later - start with fixed screen coordinates
- Player control will be added in a later phase - balls are autonomous for now
- Pixels drop from destroyed bricks will be added in Phase 2
- Pixel collection will use a singleton manager approach (not individual entities per pixel) for performance - a singleton component will track pixel positions/collections and handle rendering/collection logic

