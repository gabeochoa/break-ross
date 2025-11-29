# Shader Rendering Research Findings

## Raylib Shader Capabilities

### Available Infrastructure
- `rlgl.h` is already included via `external.h`
- Shader loading functions exist in `render_backend.h`:
  - `LoadShader(vsFileName, fsFileName)`
  - `BeginShaderMode(shader)`
  - `EndShaderMode()`
  - `SetShaderValue()`
  - `SetShaderValueTexture()`
  - `GetShaderLocation()`

### Research Needed
- Check if Raylib/rlgl supports instanced rendering
- Verify texture-based data storage capabilities
- Check for geometry shader support
- Determine SSBO/UBO support

## Current Rendering Performance Analysis

### Draw Call Counts (Estimated)
- **RenderBrick**: Up to 5,000 draw calls (one per brick in 100×50 grid)
- **RenderFogOfWar**: Up to 5,000 draw calls (one per unrevealed cell)
- **RenderRoads**: Thousands of draw calls (one per road segment)
- **RenderCar**: N draw calls (one per car, scales with upgrades)
- **RenderSquare**: M draw calls (one per square entity)

### Performance Bottlenecks
- Each primitive = 1 CPU draw call
- Linear scaling with number of objects
- Grid systems iterate over all cells every frame

## Shader Architecture Design

### Approach: Texture-Based Rendering (Recommended for Grid Systems)

#### Advantages:
- Single draw call per grid system
- GPU handles all iteration
- Easy to update via texture uploads
- Works well with existing Raylib infrastructure

#### Implementation:
1. Store game state in textures:
   - Brick health → RGBA texture (100×50)
   - Fog of war → Single channel texture (100×50)
   - Car positions/colors → Dynamic texture (1 pixel per car)

2. Shader reads texture and renders:
   - Vertex shader: Generate positions from grid coordinates
   - Fragment shader: Sample texture for color/visibility

### Approach: Instanced Rendering (For Dynamic Objects)

#### Advantages:
- Single draw call for all instances
- Per-instance attributes (position, color, size)
- Good for cars and squares

#### Implementation:
- Use rlgl functions if available
- Create base mesh (circle, rectangle)
- Pass instance data as vertex attributes

## Migration Strategy

### Phase 1: Grid Systems (Highest Impact)
1. RenderBrick → Texture-based shader
2. RenderFogOfWar → Texture-based shader

### Phase 2: Dynamic Objects
1. RenderCar → Instanced rendering or texture-based
2. RenderSquare → Instanced rendering

### Phase 3: Roads
1. RenderRoads → Geometry shader or instanced lines

