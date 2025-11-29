# World Mapping Incremental Game

## High-Level Concept

An incremental game where you control Google Maps-style cars that drive on roads to map the entire world. Start with a single car mapping your local area, then purchase more cars and upgrades to expand coverage. The game uses a tile-based system showing the world map, with roads that cars follow. As cars drive on roads, they reveal the map underneath. Each fully mapped region unlocks a comic frame in the gallery telling the story of that part of the world. The goal is to map the entire world, starting from a small area and scaling up to a massive 20k×20k world map.

### Core Gameplay Loop

1. **Drive**: Control a mapping car (or watch autonomous cars) drive on roads
2. **Map**: Cars reveal the world map as they drive on roads
3. **Collect**: Earn currency from mapping roads and discovering areas
4. **Spend**: Use currency to purchase more cars or upgrades (speed, range, etc.)
5. **Expand**: Uncover new regions of the world map
6. **Unlock Frames**: Fully mapping a region unlocks its comic frame in the gallery
7. **Scale**: Progress from local area to entire world (500×500 → 20k×20k)
8. **Prestige**: Reset map and double speed/size to unlock new world regions and comic frames
9. **Gallery**: Revisit unlocked comic frames in main menu to re-read the world's story

---

## Development Phases

### Phase 1: Core Gameplay Foundation
**Goal**: Get basic gameplay working with cars driving on roads

#### Tasks
- [ ] **Car System**
  - Implement player-controlled mapping car
  - Car movement along roads (follow road paths)
  - Car physics (speed, turning, road following)
  - Car rendering (simple car sprite/icon)
  - Car direction/orientation based on road direction

- [ ] **Road System**
  - Road network generation/loading
  - Road rendering (lines/paths)
  - Road path following for cars
  - Road intersections and branching
  - Simple road network (start with grid-based roads)

- [ ] **World Map System**
  - World map tile system (start with 500×500)
  - Map rendering (unmapped = dark/covered, mapped = revealed)
  - Map reveal tracking (which tiles are mapped)
  - Basic map texture/background

- [ ] **Mapping Mechanics**
  - Reveal map tiles as car drives on roads
  - Track mapped vs unmapped regions
  - Visual feedback for newly mapped areas
  - Mapping progress calculation

- [ ] **Basic Rendering**
  - Render world map
  - Render roads
  - Render cars
  - Basic camera/viewport
  - Zoom/pan controls

- [ ] **Input System**
  - Car movement controls (or autonomous driving)
  - Camera controls (zoom, pan)
  - Basic input handling

**Deliverable**: Playable prototype where player controls a car that drives on roads and reveals the map

---

### Phase 2: Economy and Basic Progression
**Goal**: Add currency system and basic upgrades

#### Tasks
- [x] **Currency System**
  - Pixel counter/display
  - Currency tracking
  - UI for displaying pixel count

- [x] **Basic Upgrade Shop**
  - Shop UI/menu
  - Purchase upgrades with pixels
  - Basic upgrade types:
    - Ball speed increase
  - Upgrade cost scaling

- [x] **Upgrade Application**
  - Apply upgrades to gameplay
  - Persist upgrades (in-memory for now)

**Deliverable**: Functional economy where breaking bricks earns pixels that can be spent on upgrades

---

### Phase 3: Photo Uncovering System (Basic - 500×500)
**Goal**: Implement basic photo reveal system at small scale

#### Tasks
- [x] **Photo Loading**
  - Load 500×500 test photo
  - Simple texture loading with Raylib
  - Photo rendering as background

- [x] **Reveal Tracking**
  - Track which regions are revealed (simple boolean grid or bitmap)
  - Reveal state data structure
  - Mark regions as revealed when bricks are broken

- [x] **Basic Reveal Rendering**
  - Render only revealed portions of photo
  - Use RenderTexture2D or masking approach
  - Simple reveal visualization

- [x] **Reveal Mechanics**
  - Uncover photo regions based on brick destruction
  - Calculate reveal percentage
  - Visual feedback for reveals

- [x] **Test Photo Generation**
  - Create simple Python script to generate 500×500 test images
  - Script location: `scripts/generate_photos.py`
  - Generate basic test patterns

**Deliverable**: Working photo uncover system at 500×500 scale

---

### Phase 4: Advanced Gameplay Features
**Goal**: Add road difficulty, powerups, discovery bonuses, and combo system

#### Tasks
- [ ] **Road Difficulty System**
  - Road difficulty based on distance from starting area
  - Harder roads require more time/upgrades to map
  - Road difficulty visualization (color coding)
  - Progressive difficulty as you expand outward

- [ ] **Road Visualization**
  - Color coding system for road difficulty
  - Visual feedback for mapped vs unmapped roads
  - Road type indicators (highway, street, path)
  - Road completion status

- [ ] **Multiple Cars System** (Enhanced)
  - Purchase additional autonomous mapping cars
  - Autonomous car AI (pathfinding, road following)
  - Car management system (assign cars to regions)
  - Render multiple cars with different colors/icons
  - Car efficiency/upgrade system

- [ ] **Discovery System**
  - Discover points of interest (POIs) while mapping
  - POIs give bonus currency
  - Different POI types (landmarks, cities, etc.)
  - Discovery notifications

- [ ] **Combo System**
  - Track consecutive road segments mapped
  - Combo multiplier for currency collection
  - Combo decay over time
  - Combo display/UI

- [ ] **Basic Powerups (Temporary)**
  - Powerups from discovering special areas
  - Speed boost powerup (cars move faster)
  - Mapping range boost (larger reveal radius)
  - Currency multiplier powerup
  - Powerup duration timers
  - Powerup visual indicators

- [ ] **Special Areas**
  - Different area types with varying currency values
  - Rare areas that drop powerups
  - Area variety system (cities, landmarks, etc.)

**Deliverable**: Enhanced gameplay with road difficulty, discovery system, multiple cars, and powerups

---

### Phase 5: World Map System (Advanced - Tile Pyramid)
**Goal**: Implement Google Maps-style tile system for large-scale world maps

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

- [ ] **Mapping System Integration**
  - Track mapping state per tile
  - Update mapping when cars drive on roads
  - Render mapped tiles
  - Partial mapping support (gradual reveal as cars approach)

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
  - Reset world map on prestige (all areas unmapped again)
  - Double car speed on prestige (multiplicative)
  - Double world map size on prestige
  - Track prestige level
  - Prestige confirmation dialog

- [ ] **World Map Size Scaling**
  - Dynamic map size based on prestige level
  - 500×500 → 1000×1000 → 2000×2000 → etc.
  - Regenerate road network for new size
  - Adjust starting area and road difficulty calculations
  - New world regions unlock with each prestige

- [ ] **Statistics Tracking**
  - Total currency collected (lifetime)
  - Cars purchased
  - World map uncovered percentage
  - Total road distance mapped
  - Areas discovered
  - Playtime tracking
  - Prestige count
  - Statistics UI/display

- [ ] **Save/Load System**
  - Save game state to file
  - Load game state from file
  - Save: currency, upgrades, prestige level, mapped tiles, road network state
  - Load: restore game state
  - Auto-save functionality

- [ ] **Progress Persistence**
  - Save mapped world regions
  - Save upgrade purchases
  - Save prestige progress
  - Save road network state
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
  - Frame metadata (story text, narrative about world regions)

- [ ] **Frame Unlocking System**
  - Detect 100% world region completion (all roads mapped)
  - Unlock comic frame on completion
  - Save unlocked frames
  - Frame persistence across prestiges
  - Frame organization (chronological, by world region)

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
- [ ] **World Map Generation Script**
  - Python script: `scripts/generate_maps.py`
  - Generate test world maps at various sizes (500×500, 1k×1k, 2k×2k, 4k×4k, etc.)
  - Create zoom pyramid (Level 0-3)
  - Output tiles in organized structure: `tiles/zoom{level}/tile_{x}_{y}.png`
  - Generate road networks
  - Generate mapping masks for testing
  - Support comic frame generation for world regions

- [ ] **Road Network Generation**
  - **Option A: Procedural Generation** (Easier, faster to implement)
    - Generate road networks procedurally
    - Grid-based or algorithm-based road patterns
    - Road pathfinding data
    - Road hierarchy (highways, streets, paths)
    - Intersection generation
    - Good for testing and controlled gameplay
  
  - **Option B: Real-World Road Data** (More complex, but authentic)
    - Use OpenStreetMap (OSM) data - free, open-source
    - Download OSM data in PBF format (compressed, efficient)
    - Parse OSM data using library (libosmium, or custom parser)
    - Extract road ways and nodes
    - Convert to game coordinate system
    - Simplify road network (reduce detail for performance)
    - Pre-process into game-friendly format
    - **Challenges:**
      - Data size: Full world = terabytes (need partitioning)
      - Memory: Can't load entire world at once
      - Processing: Need to simplify/sample roads
      - Coordinate conversion: Lat/lon to game coords
    - **Approach:**
      - Pre-process OSM data into tiles (match game tile system)
      - Load tiles on-demand (same as world map tiles)
      - Simplify roads (remove minor streets, reduce points)
      - Store in efficient format (binary, compressed)
    - **Libraries/Tools:**
      - libosmium (C++ OSM parser)
      - osmium-tool (command-line OSM processing)
      - OSMnx (Python, for preprocessing)
      - PostGIS (if using database approach)

- [ ] **Comic Frame Content**
  - Design comic frame layouts for world regions
  - Create story/narrative content about world areas
  - Generate comic frame images
  - Organize frames by prestige level/world region

- [ ] **Testing Tools**
  - Debug visualization tools
  - Performance profiling
  - Reveal state visualization
  - Tile loading visualization

**Deliverable**: Tools for generating game content

---

## Road Data Loading: Feasibility Analysis

### Real-World Road Data Options

#### Option 1: OpenStreetMap (OSM) - Recommended
**Pros:**
- Free and open-source
- Comprehensive global coverage
- Regularly updated
- Multiple data formats available
- Well-documented

**Cons:**
- Large data size (full world = terabytes)
- Need to parse and convert formats
- Requires simplification for game use

**Data Formats:**
- **PBF** (Protocol Buffer Format) - Compressed, efficient, recommended
- **XML** - Human-readable but much larger
- **Shapefile** - Common GIS format

**Implementation Approach:**
1. **Pre-processing** (Python/offline):
   - Download OSM extract for region (country/continent)
   - Use osmium-tool or OSMnx to extract roads
   - Simplify road network (remove minor roads, reduce points)
   - Convert to game coordinate system
   - Split into tiles matching game tile system
   - Export to binary format for fast loading

2. **Runtime Loading** (C++):
   - Load road tiles on-demand (same as world map tiles)
   - Parse binary road data format
   - Build road graph for pathfinding
   - Cache loaded road tiles

**Libraries:**
- **libosmium** (C++) - Fast OSM parser
- **osmium-tool** - Command-line OSM processing
- **OSMnx** (Python) - Good for preprocessing/analysis

**Data Size Estimates:**
- Single country (e.g., USA): ~1-5 GB raw OSM data
- After simplification: ~100-500 MB
- Per tile (500×500 equivalent): ~1-10 MB
- Full world: Terabytes (not practical to load all at once)

#### Option 2: Procedural Generation
**Pros:**
- Full control over road patterns
- Predictable performance
- Smaller data size
- Can generate on-the-fly
- Easier to implement

**Cons:**
- Not authentic real-world roads
- May feel less interesting
- Need to design good algorithms

**Approach:**
- Grid-based road networks
- L-system or similar for organic patterns
- Perlin noise for natural variation
- Road hierarchy (highways → streets → paths)

#### Option 3: Hybrid Approach
**Best of Both Worlds:**
- Use real OSM data for major regions/landmarks
- Use procedural generation for filler areas
- Mix authentic and generated content
- Pre-process OSM data into simplified game format

### Recommended Implementation Strategy

**For Development/Testing:**
- Start with procedural generation
- Get gameplay working first
- Add real road data later as enhancement

**For Production:**
- Pre-process OSM data offline
- Extract and simplify roads
- Convert to game tile format
- Load on-demand like world map tiles
- Use same tile pyramid system

**Practical Steps:**
1. **Phase 1**: Procedural roads (get game working)
2. **Phase 2**: Add OSM data for specific regions (cities, countries)
3. **Phase 3**: Expand to more regions based on player interest
4. **Phase 4**: Full world coverage (if desired)

**Memory Management:**
- Same approach as world map tiles
- Load road data per tile
- Cache recently used tiles
- Unload distant tiles
- Road data per tile: ~1-10 MB (after simplification)
- Can keep ~50-100 road tiles in memory (~100-1000 MB total)

---

## Technical Details

### Gameplay Mechanics

#### Ball System
- Autonomous bouncing balls
- Ball physics: bounces off walls and bricks
- Can purchase additional balls

#### Brick System
- Breakable blocks in grid layout
- Distance-based health (further from spawn = more health)
- Health visualization: color coding (green → yellow → orange → red)
- Different brick types with varying pixel values
- Special bricks drop powerups

#### Economy
- Pixels are earned when bricks are broken
- Spend pixels on upgrades or new balls
- Combo system increases pixel multiplier
- Combo decays over time

#### Upgrades
- Ball speed, power
- New balls
- Automation upgrades (auto-collect, idle progression)

#### Powerups
**Permanent** (purchased): Ball speed, damage, multi-ball, auto-collect, etc.

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
