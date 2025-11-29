# Shader-Based Rendering Optimization Research Plan

## Current State Analysis

### Rendering Systems (CPU-side individual draw calls):
- **RenderBrick**: Draws rectangles for each brick in 100×50 grid (up to 5,000 bricks) using `DrawRectangleRec`
- **RenderFogOfWar**: Draws rectangles for each unrevealed cell (up to 5,000 cells) using `DrawRectangleV`
- **RenderRoads**: Draws lines for each road segment (NYC roads = thousands of segments) using `DrawLineEx`
- **RenderCar**: Draws circles for each car (scales with upgrades) using `DrawCircleV`
- **RenderSquare**: Draws rectangles for square entities using `DrawRectangleV`

### Performance Bottleneck:
- Each primitive = 1 CPU draw call
- Linear scaling: more cars/segments = more draw calls = CPU bottleneck
- Grid-based systems (bricks, fog) iterate over 5,000 cells per frame

### Existing Assets:
- Shader infrastructure exists in `render_backend.h` (LoadShader, BeginShaderMode, etc.)
- Brick shaders exist but unused: `brick_vertex.glsl`, `brick_fragment.glsl`
- Using Raylib (OpenGL-based, supports shaders)

## Research Areas

### 1. Raylib Shader Capabilities
- **Instanced Rendering**: Does Raylib support instanced rendering? (rlDrawMeshInstanced)
- **Shader Storage Buffer Objects (SSBOs)**: For passing large data arrays to shaders
- **Uniform Buffer Objects (UBOs)**: For batch uniform data
- **Texture-based Data Storage**: Using textures as data arrays (RGBA32F for positions/colors)
- **Geometry Shaders**: For generating geometry on GPU (lines from points, etc.)

### 2. Batch Rendering Strategies

#### A. Instanced Rendering for Similar Objects
- **Cars**: Single instanced draw call for all cars (position, color, size as instance data)
- **Bricks**: Single instanced draw call for all bricks (grid position, health as instance data)
- **Fog Cells**: Single instanced draw call for all unrevealed cells
- **Squares**: Single instanced draw call for all square entities

#### B. Texture-Based Data Storage
- Store brick health in texture (100×50 = 5,000 pixels)
- Store fog of war state in texture (1 bit per cell)
- Store car positions/colors in texture (1 pixel per car)
- Shader reads texture to determine what to render

#### C. Geometry Shader for Lines
- Pass road segment endpoints as points
- Geometry shader generates line quads on GPU
- Single draw call for all road segments

### 3. Implementation Approaches

#### Approach 1: Instanced Rendering (Recommended if supported)
- Create vertex buffers for base shapes (circle, rectangle, line)
- Use instance attributes for per-object data (position, color, size)
- Single draw call per object type
- **Files to modify**: All Render*.h systems, create new shader files

#### Approach 2: Texture-Based Rendering
- Update textures each frame with game state
- Shader samples texture to render entire grid at once
- **Files to modify**: RenderBrick.h, RenderFogOfWar.h, create texture update system

#### Approach 3: Hybrid Approach
- Use instanced rendering for dynamic objects (cars, squares)
- Use texture-based for grid systems (bricks, fog)
- Use geometry shader for roads

### 4. Raylib-Specific Considerations
- Check if Raylib exposes OpenGL functions directly
- May need to use `rlgl.h` (Raylib's OpenGL wrapper) for advanced features
- Verify shader uniform limits and buffer size limits
- Test performance on target platforms

### 5. Migration Strategy
1. **Phase 1**: Migrate grid-based systems (Bricks, FogOfWar) - highest impact
2. **Phase 2**: Migrate dynamic objects (Cars, Squares) - medium impact
3. **Phase 3**: Migrate roads - lower impact but still significant

## Research Tasks

1. **Document Raylib shader capabilities**
   - Check Raylib documentation for instanced rendering
   - Test rlgl.h for advanced OpenGL features
   - Verify shader uniform/buffer limits

2. **Benchmark current performance**
   - Profile draw calls per frame
   - Measure frame time with varying car/segment counts
   - Identify worst offenders

3. **Design shader architecture**
   - Create shader file structure
   - Design data layout (instance attributes, textures, uniforms)
   - Plan shader parameter system

4. **Create proof-of-concept**
   - Start with one system (e.g., RenderBrick)
   - Implement texture-based or instanced rendering
   - Measure performance improvement

5. **Plan migration path**
   - Determine which systems to migrate first
   - Identify breaking changes
   - Plan backward compatibility if needed

## Key Files to Research/Modify

- `src/systems/RenderBrick.h` - Grid-based, high draw call count
- `src/systems/RenderFogOfWar.h` - Grid-based, high draw call count  
- `src/systems/RenderCar.h` - Dynamic, scales with upgrades
- `src/systems/RenderRoads.h` - Many segments, line rendering
- `src/systems/RenderSquare.h` - Dynamic entities
- `src/render_backend.h` - May need to add new shader functions
- `resources/shaders/` - Create new shader files
- `src/game.cpp` - Render system registration order may change

## Expected Outcomes

- **Performance**: Reduce draw calls from O(n) to O(1) per object type
- **Scalability**: Support 100+ cars and thousands of segments without frame drops
- **Maintainability**: Centralize rendering logic in shaders
- **Flexibility**: Easier to add visual effects (shadows, glow, etc.)

## Implementation Todos

1. **research-raylib-capabilities**: Research Raylib shader capabilities: instanced rendering, SSBOs, UBOs, texture-based data, geometry shaders. Check rlgl.h for advanced OpenGL features.

2. **benchmark-current-performance**: Profile current rendering: count draw calls per frame, measure frame time with varying car/segment counts, identify performance bottlenecks.

3. **design-shader-architecture**: Design shader architecture: file structure, data layout (instance attributes vs textures vs uniforms), shader parameter system. (Depends on: research-raylib-capabilities)

4. **create-poc-brick-rendering**: Create proof-of-concept: migrate RenderBrick to shader-based rendering (texture-based or instanced), measure performance improvement. (Depends on: design-shader-architecture)

5. **plan-migration-strategy**: Plan full migration: determine order (grid systems first, then dynamic objects, then roads), identify breaking changes, plan backward compatibility. (Depends on: create-poc-brick-rendering)

