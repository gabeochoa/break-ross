# World Mapping Incremental Game

## High-Level Concept

An incremental game where you control a square that travels along real-world roads (starting with New York City) to uncover a fog of war. The square follows road paths, revealing the map underneath as it travels. Start with a single square, then purchase more squares and upgrades to expand coverage faster. The game uses a tile-based system showing the world map with real road networks. As squares travel on roads, they reveal the map in a fog-of-war style. Each fully mapped region unlocks a comic frame in the gallery. The goal is to map the entire area, starting with NYC and eventually scaling up to larger regions.

### Core Gameplay Loop

1. **Travel**: Control a square (or watch autonomous squares) travel along roads
2. **Reveal**: Squares reveal the world map in fog-of-war style as they travel on roads
3. **Collect**: Earn currency from mapping roads and discovering areas
4. **Spend**: Use currency to purchase more squares or upgrades (speed, reveal radius, etc.)
5. **Expand**: Uncover new regions of the world map
6. **Unlock Frames**: Fully mapping a region unlocks its comic frame in the gallery
7. **Scale**: Progress from NYC to larger regions (eventually 20k×20k)
8. **Prestige**: Reset map and double speed/size to unlock new regions and comic frames
9. **Gallery**: Revisit unlocked comic frames in main menu to re-read the story

---

## Development Phases

### Phase 1: Core Gameplay Foundation
**Goal**: Get basic gameplay working with square traveling on NYC roads with fog of war

#### Tasks
- [ ] **Square System**
  - Implement player-controlled square
  - Square movement along roads (follow road paths)
  - Square physics (speed, road following)
  - Square rendering (simple colored square)
  - Square orientation based on road direction

- [ ] **NYC Road System**
  - Load NYC road data from OpenStreetMap
  - Parse OSM data (PBF format) for NYC
  - Extract road ways and nodes
  - Convert to game coordinate system
  - Road rendering (lines/paths)
  - Road path following for squares
  - Road intersections and branching
  - Build road graph for pathfinding

- [ ] **Fog of War System**
  - World map tile system (NYC area)
  - Fog of war rendering (unmapped = dark/fog, mapped = revealed)
  - Map reveal tracking (which tiles are revealed)
  - Reveal radius around square
  - Gradual fog reveal as square travels
  - Basic map texture/background (satellite imagery or stylized)

- [ ] **Mapping Mechanics**
  - Reveal map tiles in fog-of-war style as square travels on roads
  - Track revealed vs unrevealed regions
  - Visual feedback for newly revealed areas
  - Mapping progress calculation
  - Fog of war shader/rendering

- [ ] **Basic Rendering**
  - Render world map with fog of war
  - Render roads (visible on revealed areas)
  - Render squares
  - Basic camera/viewport
  - Zoom/pan controls

- [ ] **Input System**
  - Square movement controls (click to move, or autonomous)
  - Camera controls (zoom, pan)
  - Basic input handling

**Deliverable**: Playable prototype where player controls a square that travels on NYC roads and reveals fog of war

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

- [ ] **Multiple Squares System** (Enhanced)
  - Purchase additional autonomous mapping squares
  - Autonomous square AI (pathfinding, road following)
  - Square management system (assign squares to regions)
  - Render multiple squares with different colors
  - Square efficiency/upgrade system

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
  - Speed boost powerup (squares move faster)
  - Fog reveal range boost (larger reveal radius)
  - Currency multiplier powerup
  - Powerup duration timers
  - Powerup visual indicators

- [ ] **Special Areas**
  - Different area types with varying currency values
  - Rare areas that drop powerups
  - Area variety system (cities, landmarks, etc.)

**Deliverable**: Enhanced gameplay with road difficulty, discovery system, multiple squares, and powerups

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

- [ ] **Fog of War System Integration**
  - Track fog of war state per tile
  - Update fog reveal when squares travel on roads
  - Render revealed tiles (fog cleared)
  - Partial reveal support (gradual fog clearing as squares approach)

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
  - Reset world map on prestige (all areas covered in fog again)
  - Double square speed on prestige (multiplicative)
  - Expand to larger regions on prestige (NYC → NYC+surrounding → larger area)
  - Track prestige level
  - Prestige confirmation dialog

- [ ] **World Map Size Scaling**
  - Dynamic map size based on prestige level
  - Start: NYC area
  - Prestige 1: NYC + surrounding areas
  - Prestige 2: Larger region (state/country)
  - Prestige 3+: Continue expanding
  - Load new road networks for expanded areas
  - Adjust starting area and road difficulty calculations
  - New world regions unlock with each prestige

- [ ] **Statistics Tracking**
  - Total currency collected (lifetime)
  - Squares purchased
  - World map uncovered percentage (fog of war cleared)
  - Total road distance traveled
  - Areas discovered
  - Playtime tracking
  - Prestige count
  - Statistics UI/display

- [ ] **Save/Load System**
  - Save game state to file
  - Load game state from file
  - Save: currency, upgrades, prestige level, revealed tiles (fog of war state), road network state
  - Load: restore game state
  - Auto-save functionality

- [ ] **Progress Persistence**
  - Save revealed world regions (fog of war state)
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

- [ ] **NYC Road Data Loading** (Phase 1 Implementation)
  - **Download NYC OSM Data:**
    - Get NYC extract from OpenStreetMap
    - Use Geofabrik downloads (free OSM extracts)
    - NYC area: ~50-200 MB in PBF format
    - URL: https://download.geofabrik.de/north-america/us/new-york.html
  
  - **Pre-processing Pipeline:**
    - Use Python script to process OSM data
    - Extract only roads (filter by highway tags)
    - Extract traffic signals and stop signs
    - Simplify road network (remove footpaths, very minor roads)
    - Convert lat/lon to game coordinates
    - Build road graph (nodes and edges)
    - Export to binary format for fast loading (see Binary Map Format Specification)
    - Script: `scripts/process_osm_to_binary.py`
  
  - **Runtime Loading:**
    - Load pre-processed NYC road data from binary format
    - Fast binary deserialization in C++
    - Build road graph in memory
    - Road rendering system
    - Pathfinding for square movement
    - Traffic system initialization (lights, stop signs)
  
  - **Libraries/Tools:**
    - **osmium-tool** - Extract roads from OSM PBF
    - **OSMnx** (Python) - Road network analysis and simplification
    - **NetworkX** (Python) - Graph manipulation
    - Custom binary format for C++ loading
  
  - **Data Simplification:**
    - Keep: highways, primary roads, secondary roads, residential streets
    - Remove: footpaths, cycleways, very minor paths
    - Reduce points (simplify curves) for performance
    - Target: ~10-50k road segments for NYC (manageable size)

- [ ] **Road Network Generation** (Future: Other Regions)
  - **Option A: Procedural Generation** (For testing/other regions)
    - Generate road networks procedurally
    - Grid-based or algorithm-based road patterns
    - Road pathfinding data
    - Road hierarchy (highways, streets, paths)
    - Intersection generation
  
  - **Option B: Real-World Road Data** (For authentic regions)
    - Use same OSM pipeline as NYC
    - Process other cities/regions
    - Expand coverage as game scales

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
- Start with NYC OSM data (real roads from the start)
- Get gameplay working with NYC roads
- Add more regions later as enhancement

**For Production:**
- Pre-process OSM data offline
- Extract and simplify roads
- Convert to game tile format
- Load on-demand like world map tiles
- Use same tile pyramid system

**Practical Steps:**
1. **Phase 1**: NYC OSM roads (get game working with real roads)
2. **Phase 2**: Add more cities/regions using OSM data
3. **Phase 3**: Expand to larger regions based on player interest
4. **Phase 4**: Full world coverage (if desired)

**Memory Management:**
- Same approach as world map tiles
- Load road data per tile
- Cache recently used tiles
- Unload distant tiles
- Road data per tile: ~1-10 MB (after simplification)
- Can keep ~50-100 road tiles in memory (~100-1000 MB total)

---

## Binary Map Format Specification

### Format Design
- **Binary format** for dense, efficient storage and fast loading
- Compact representation of road networks, traffic signals, and map data
- **Multiple LOD (Level of Detail) levels** for efficient rendering at different zoom levels
- Optimized for C++ runtime loading

### Data Structure
- **Header**: Magic number, version, metadata (bounds, LOD count, segment counts per LOD, etc.)
- **Multiple LOD Levels** (e.g., LOD 0-3 matching zoom levels):
  - **LOD 0** (Overview/Zoomed Out): Only major highways and primary roads, highly simplified
  - **LOD 1** (Medium Zoom): Highways, primary, and secondary roads
  - **LOD 2** (Close Zoom): All major roads including residential
  - **LOD 3** (Full Detail/Zoomed In): Complete road network with all detail
- **Road Segments per LOD**: 
  - Start/end coordinates (float32)
  - Road type/hierarchy (uint8)
  - Traffic rules (stop signs, traffic lights) at intersections
  - Connected segment indices
- **Traffic Data**:
  - Traffic light positions and states (per LOD, simplified at lower LODs)
  - Stop sign positions (only at higher LODs)
  - Intersection data
- **Node Graph**: Road network connectivity for pathfinding (per LOD)

### Conversion Script
- **Script**: `scripts/process_osm_to_binary.py`
- **Input**: OSM PBF file (downloaded from Geofabrik or OSM)
- **Process**:
  1. Download OSM data (or use existing PBF file)
  2. Parse OSM PBF format
  3. Extract roads (filter by highway tags)
  4. Extract traffic signals and stop signs
  5. Generate multiple LOD levels:
     - LOD 0: Filter to only highways/primary, heavy simplification
     - LOD 1: Add secondary roads, moderate simplification
     - LOD 2: Add residential roads, light simplification
     - LOD 3: Full detail, minimal simplification
  6. Convert lat/lon to game coordinates
  7. Build road graph (nodes and edges) for each LOD
  8. Serialize all LODs to binary format
- **Output**: Binary map file (`.bmap` or similar) containing all LOD levels
- **Libraries**: 
  - `osmium` (Python) - OSM PBF parsing
  - `OSMnx` - Road network analysis
  - `NetworkX` - Graph manipulation
  - Custom binary serialization

### Runtime Loading
- Fast binary deserialization in C++
- **Simulation vs Rendering**:
  - **Simulation** (pathfinding, car movement, traffic): Always uses full detail (LOD 3) for accurate gameplay
  - **Rendering** (visual display): Switches between LOD levels based on camera zoom level
    - Zoomed out → LOD 0 (overview, fewer segments for performance)
    - Zoomed in → LOD 3 (full detail, all segments)
- Load full detail LOD for simulation, appropriate rendering LOD for display
- Direct memory mapping possible for very large files
- Load entire map or tile-based loading for large regions
- Smooth LOD transitions in rendering as player zooms in/out

---

## Technical Details

### Gameplay Mechanics

#### Square System
- Player-controlled or autonomous squares
- Square movement: follows road paths
- Square physics: speed, road following, pathfinding
- Can purchase additional autonomous squares
- Squares reveal fog of war as they travel

#### Fog of War System
- World map starts completely covered in fog
- Squares reveal map in radius around them as they travel
- Reveal radius increases with upgrades
- Fog gradually clears as squares approach
- Mapped areas show world map underneath
- Unmapped areas remain dark/foggy

#### Road System
- Real-world roads from OpenStreetMap (starting with NYC)
- Roads visible only in revealed areas
- Road hierarchy: highways, primary, secondary, residential
- Road difficulty based on distance from starting area
- Pathfinding for square movement along roads

#### NPC Cars and Traffic System
- NPC cars drive around on roads to make the city feel alive
- NPC cars are visible but do not reveal fog of war
- Traffic simulation with red lights and stop signs (using data from OpenStreetMap dataset)
- NPC cars follow traffic rules (stop at red lights, yield at stop signs)
- Adds atmosphere and makes the world feel more dynamic and lived-in

#### Economy
- Currency earned from mapping roads (distance-based)
- Currency earned from discovering new areas
- Spend currency on upgrades or new squares
- Combo system increases currency multiplier (consecutive road segments)
- Combo decays over time

#### Upgrades
- Square speed
- Fog reveal radius
- Currency generation rate
- New squares
- Automation upgrades (idle progression)
- Laser reveal upgrade: Squares can shoot out a laser every X ms that reveals any part of the road ahead of them
- Radar pulse upgrade: Periodic circular pulse that reveals a radius around the square (like sonar), revealing nearby roads and intersections

#### Powerups
**Permanent** (purchased): Square speed, reveal radius, currency rate, multi-square, etc.

**Temporary** (drops): Speed boost, reveal range boost, currency multiplier, etc.

#### Day/Night Cycle
- Visual day/night cycle that affects the atmosphere and lighting of the world map
- Fog of war color changes: blue/green during day, gradually transitions to black at night
- Implemented as a shader for performance
- Gameplay effects:
  - Cars/squares move slower at night
  - Fog of war reveal radius/cone should be more visually obvious (enhanced visibility of reveal area)
- Adds visual variety and sense of time passing as you explore

### Fog of War and World Map System

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
- Resets map (all areas covered in fog again) and upgrades
- Doubles square speed (multiplicative: 2x, 4x, 8x, etc.)
- Expands to larger regions (NYC → NYC+surrounding → larger areas → ... → 20k×20k)
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
