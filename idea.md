# Incremental Photo Uncovering Game

## High-Level Concept

An incremental game that combines pong and brick breaker mechanics, where players uncover interconnected photos designed as comic frames that tell a story and provide upgrades. The game starts at the pixel level and progressively scales up to reveal a massive 20k x 20k photo composed of comic frames. Each fully uncovered photo region unlocks a comic frame in the main menu gallery, allowing players to re-read the story they've uncovered.

### Core Gameplay Loop

1. **Play**: Control a bouncing ball to break blocks and collect pixels
2. **Collect**: Gather pixels (currency) from broken bricks
3. **Spend**: Use gold/pixels to purchase upgrades or buy new balls
4. **Uncover**: Reveal portions of interconnected comic frame photos as you progress
5. **Unlock Frames**: Fully uncovering a photo region unlocks its comic frame in the gallery
6. **Scale**: Progress from pixel-level detail to increasingly larger photo resolutions
7. **Prestige**: Reset map and double speed/size to unlock new comic frames
8. **Gallery**: Revisit unlocked comic frames in main menu to re-read the story

---

## Development Phases

### Phase 1: Core Gameplay Foundation
**Goal**: Get basic gameplay working with ball control and brick breaking

#### Tasks
- [ ] **Ball Control System**
  - Implement player-controlled ball with direct movement
  - Basic ball physics (velocity, bouncing)
  - Ball collision detection with walls
  - Ball rendering

- [ ] **Brick System**
  - Create brick grid layout
  - Brick rendering
  - Basic brick collision detection
  - Brick destruction on hit
  - Simple brick patterns

- [ ] **Basic Physics**
  - Ball-brick collision response
  - Ball-wall bouncing
  - Basic physics loop integration

- [ ] **Simple Rendering**
  - Render ball
  - Render brick grid
  - Basic camera/viewport
  - Clear background

- [ ] **Input System**
  - Ball movement controls (WASD/Arrow keys)
  - Basic input handling

**Deliverable**: Playable prototype where player controls a ball that breaks bricks

---

### Phase 2: Economy and Basic Progression
**Goal**: Add pixel collection, currency system, and basic upgrades

#### Tasks
- [ ] **Pixel Collection System**
  - Pixels drop from broken bricks
  - Pixel rendering (visual representation)
  - Pixel collection by player ball (proximity-based)
  - Pixel collection range/radius

- [ ] **Currency System**
  - Pixel counter/display
  - Currency tracking
  - UI for displaying pixel count

- [ ] **Basic Upgrade Shop**
  - Shop UI/menu
  - Purchase upgrades with pixels
  - Basic upgrade types:
    - Ball speed increase
    - Ball control improvement
    - Collection range increase
  - Upgrade cost scaling

- [ ] **Upgrade Application**
  - Apply upgrades to gameplay
  - Persist upgrades (in-memory for now)

**Deliverable**: Functional economy where breaking bricks earns pixels that can be spent on upgrades

---

### Phase 3: Photo Uncovering System (Basic - 500×500)
**Goal**: Implement basic photo reveal system at small scale

#### Tasks
- [ ] **Photo Loading**
  - Load 500×500 test photo
  - Simple texture loading with Raylib
  - Photo rendering as background

- [ ] **Reveal Tracking**
  - Track which regions are revealed (simple boolean grid or bitmap)
  - Reveal state data structure
  - Mark regions as revealed when bricks are broken

- [ ] **Basic Reveal Rendering**
  - Render only revealed portions of photo
  - Use RenderTexture2D or masking approach
  - Simple reveal visualization

- [ ] **Reveal Mechanics**
  - Uncover photo regions based on brick destruction
  - Calculate reveal percentage
  - Visual feedback for reveals

- [ ] **Test Photo Generation**
  - Create simple Python script to generate 500×500 test images
  - Script location: `scripts/generate_photos.py`
  - Generate basic test patterns

**Deliverable**: Working photo uncover system at 500×500 scale

---

### Phase 4: Advanced Gameplay Features
**Goal**: Add health system, powerups, multiple balls, and combo system

#### Tasks
- [ ] **Brick Health System**
  - Health calculation based on distance from spawn
  - Health tracking per brick
  - Multiple hits required to break high-health bricks
  - Health visualization (color coding, health bars, or damage states)

- [ ] **Health Visualization**
  - Color coding system (green → yellow → orange → red)
  - Optional: Health bars, damage states, crack patterns
  - Visual feedback for damage

- [ ] **Multiple Balls System**
  - Purchase additional autonomous balls
  - Autonomous ball AI (bouncing, brick breaking)
  - Ball management system
  - Render multiple balls

- [ ] **Combo System**
  - Track consecutive brick breaks
  - Combo multiplier for pixel collection
  - Combo decay over time
  - Combo display/UI

- [ ] **Basic Powerups (Temporary)**
  - Powerup drops from special bricks
  - Speed boost powerup
  - Magnet mode powerup
  - Powerup duration timers
  - Powerup visual indicators

- [ ] **Special Bricks**
  - Different brick types with varying pixel values
  - Rare bricks that drop powerups
  - Brick variety system

**Deliverable**: Enhanced gameplay with health, combos, multiple balls, and powerups

---

### Phase 5: Photo System (Advanced - Tile Pyramid)
**Goal**: Implement Google Maps-style tile system for large-scale photos

#### Tasks
- [ ] **Tile System Architecture**
  - Design TileKey and PhotoTile data structures
  - Tile cache system (std::map)
  - Viewport calculation system
  - Tile coordinate system

- [ ] **Zoom Level System**
  - Implement zoom level 0 (overview)
  - Zoom level calculation
  - Zoom level switching
  - For 500×500: Simple single-texture approach
  - For larger sizes: Multi-level tile pyramid

- [ ] **Tile Loading System**
  - On-demand tile loading
  - Tile file structure/organization
  - Load tiles from disk or generate from source
  - Tile caching in memory

- [ ] **Viewport-Based Rendering**
  - Calculate visible tiles based on viewport
  - Render only visible tiles
  - Tile position calculation
  - Smooth tile rendering

- [ ] **Reveal System Integration**
  - Track reveal state per tile
  - Update reveals when bricks break
  - Render revealed tiles
  - Partial reveal support (alpha blending or masking)

- [ ] **Memory Management**
  - Tile cache size limits
  - Cache eviction (LRU)
  - Unload distant tiles
  - Memory optimization

- [ ] **Zoom and Pan**
  - Zoom controls (mouse wheel, keys)
  - Pan controls (drag, arrow keys)
  - Smooth zoom transitions
  - Camera system with Raylib

**Deliverable**: Scalable tile-based photo system supporting large images

---

### Phase 6: Prestige and Meta Systems
**Goal**: Add prestige system, statistics, and save/load

#### Tasks
- [ ] **Prestige System**
  - Prestige button/UI
  - Reset map on prestige
  - Double ball speed on prestige (multiplicative)
  - Double map size on prestige
  - Track prestige level
  - Prestige confirmation dialog

- [ ] **Map Size Scaling**
  - Dynamic map size based on prestige level
  - 500×500 → 1000×1000 → 2000×2000 → etc.
  - Regenerate brick grid for new size
  - Adjust spawn point and health calculations

- [ ] **Statistics Tracking**
  - Total pixels collected (lifetime)
  - Balls purchased
  - Photos uncovered percentage
  - Playtime tracking
  - Prestige count
  - Statistics UI/display

- [ ] **Save/Load System**
  - Save game state to file
  - Load game state from file
  - Save: pixels, upgrades, prestige level, revealed tiles
  - Load: restore game state
  - Auto-save functionality

- [ ] **Progress Persistence**
  - Save uncovered photo regions
  - Save upgrade purchases
  - Save prestige progress
  - Persist across game sessions

**Deliverable**: Complete prestige system with save/load functionality

---

### Phase 7: Gallery and Main Menu
**Goal**: Add main menu, gallery system, and visual polish

#### Tasks
- [ ] **Main Menu**
  - Main menu screen
  - Play button (start/continue)
  - Gallery button
  - Settings button
  - Progress display (prestige, frames unlocked)

- [ ] **Comic Frame Gallery**
  - Gallery UI (scrollable grid)
  - Frame thumbnails
  - Unlock status indicators (locked/unlocked)
  - Frame viewer (full-screen view)
  - Frame metadata (story text, narrative)

- [ ] **Frame Unlocking System**
  - Detect 100% photo region completion
  - Unlock comic frame on completion
  - Save unlocked frames
  - Frame persistence across prestiges
  - Frame organization (chronological)

- [ ] **Visual Polish**
  - Unlock animations
  - Upgrade purchase feedback
  - Milestone celebrations
  - Progress indicators
  - UI polish and styling

- [ ] **Settings Menu**
  - Graphics settings
  - Audio settings
  - Control settings
  - Save/load settings

**Deliverable**: Complete main menu and gallery system

---

### Phase 8: Content Creation and Tools
**Goal**: Create tools for generating photos and comic frames

#### Tasks
- [ ] **Photo Generation Script**
  - Python script: `scripts/generate_photos.py`
  - Generate test images at various sizes (500×500, 1k×1k, 2k×2k, 4k×4k, etc.)
  - Create zoom pyramid (Level 0-3)
  - Output tiles in organized structure: `tiles/zoom{level}/tile_{x}_{y}.png`
  - Generate reveal masks for testing
  - Support comic frame generation

- [ ] **Comic Frame Content**
  - Design comic frame layouts
  - Create story/narrative content
  - Generate comic frame images
  - Organize frames by prestige level

- [ ] **Testing Tools**
  - Debug visualization tools
  - Performance profiling
  - Reveal state visualization
  - Tile loading visualization

**Deliverable**: Tools for generating game content

---

## Technical Details

### Gameplay Mechanics

#### Ball Control
- Player directly controls one bouncing ball
- Direct movement control (not paddle-based)
- Ball physics: bounces off walls and bricks
- Can purchase additional autonomous balls

#### Brick System
- Breakable blocks in grid layout
- Distance-based health (further from spawn = more health)
- Health visualization: color coding (green → yellow → orange → red)
- Different brick types with varying pixel values
- Special bricks drop powerups

#### Economy
- Pixels drop from broken bricks
- Player ball collects pixels within range
- Spend pixels on upgrades or new balls
- Combo system increases pixel multiplier
- Combo decays over time

#### Upgrades
- Ball speed, control, power
- Pixel collection rate
- Collection range
- New balls
- Automation upgrades (auto-collect, idle progression)

#### Powerups
**Permanent** (purchased): Ball speed, damage, collection range, multi-ball, auto-collect, etc.

**Temporary** (drops): Speed boost, magnet mode, multi-ball spawn, explosive hits, piercing mode, double pixels, etc.

### Photo Uncovering System

#### Google Maps-Style Tile Pyramid
- Multiple zoom levels (LOD pyramid)
- On-demand tile loading
- Viewport-based rendering
- Tile caching and memory management

**Zoom Levels:**
- 500×500: Single texture (simple)
- 1k×1k: Level 0 overview + Level 1 tiles (2×2 grid)
- 2k×2k: Multi-level pyramid
- 4k×4k+: Full pyramid (Level 0-3)

**Implementation:**
- TileKey struct (zoom_level, tile_x, tile_y)
- PhotoTile struct (texture, is_revealed, is_loaded, world_bounds)
- Tile cache (std::map<TileKey, PhotoTile>)
- Viewport-based tile loading
- Reveal tracking per tile

### Prestige System
- Resets map and upgrades
- Doubles ball speed (multiplicative: 2x, 4x, 8x, etc.)
- Doubles map size (500×500 → 1000×1000 → 2000×2000 → ... → 20k×20k)
- Each prestige unlocks new comic frames
- Progress persists across prestiges

### Gallery System
- Unlock frames by 100% uncovering photo regions
- View unlocked frames in main menu gallery
- Scrollable grid of frame thumbnails
- Full-screen frame viewer
- Frames organized chronologically
- Story continuity across prestige levels

### Raylib Implementation Notes

**Key Functions:**
- `LoadImage()` / `LoadTexture()` - Load photos/tiles
- `DrawTextureRec()` - Draw tile portions
- `BeginTextureMode()` / `EndTextureMode()` - Render to RenderTexture2D
- `BeginMode2D()` / `EndMode2D()` - Camera for zoom/pan
- `BeginScissorMode()` / `EndScissorMode()` - Restrict drawing area

**Performance:**
- 4k×4k tiles fit within GPU texture limits
- Cache ~20-50 tiles in memory
- Viewport culling for rendering
- Lazy loading and cache eviction

---

## Development Notes

### Starting Size
- Begin with 500×500 for testing
- Scale up progressively: 1k×1k, 2k×2k, etc.
- Validate performance at each size

### Testing Strategy
- Test core mechanics at small scale first
- Validate tile system before scaling up
- Performance testing at each size increment
- Ensure smooth gameplay before adding features

### Content Pipeline
- Python script generates test photos
- Generate zoom pyramid for tile system
- Create comic frame content
- Organize content by prestige level
