# Connected Component Tracking for Road Network

## Problem
The square is jumping too early because it doesn't know if there are still reachable unvisited segments in the current connected component. We need to track which segments are in the same connected component and only allow jumping when that component is 100% explored.

## Solution Overview
1. Build a graph representation of road segments (nodes) and their connections
2. Compute connected components at initialization (or lazily for large maps)
3. Track which connected component the square is currently in
4. Only allow jumping when all segments in current component are visited
5. Improve backtrack mode to actively navigate to unvisited segments in the component

## Implementation Plan

### 1. Add Spatial Chunking System (Memory-Efficient Foundation)
**File**: `src/components.h`

Add chunk-based spatial partitioning:
- Define chunk size (e.g., 1000x1000 world units)
- `struct RoadChunk` containing:
  - `std::vector<size_t> segment_indices` - indices into main segments array
  - `std::vector<size_t> component_ids` - component IDs for segments in this chunk
  - `bool is_loaded` - whether chunk data is in memory
  - `std::bitset` or sparse set for visited segments in chunk
- `RoadNetwork` changes:
  - Keep full `segments` vector (needed for rendering/connections)
  - Add `std::unordered_map<chunk_key, RoadChunk>` for chunk data
  - Add `std::vector<size_t> component_id` - sparse, only for loaded chunks
  - `chunk_key` = `(chunk_x, chunk_y)` pair or single hash

**Memory Strategy**:
- All segments stay in memory (needed for pathfinding/connections)
- Component IDs computed on-demand or cached per chunk
- Visited tracking: use `std::bitset` or `std::unordered_set<size_t>` (sparse)
- Only compute full component graph for currently active chunk + neighbors

### 2. Add Connected Component Tracking (Chunk-Aware)
**File**: `src/components.h`

Add to `RoadNetwork` struct:
- `std::vector<size_t> component_id` - sparse, computed on-demand
- `std::unordered_map<size_t, size_t> component_sizes` - cache component sizes
- `std::unordered_map<size_t, size_t> component_visited_counts` - cache visited counts
- `size_t current_component_id` - which component we're currently exploring
- `void build_connected_components_for_chunk(chunk_key, float connection_tolerance)` - builds components for a chunk
- `size_t get_component_id(size_t segment_index)` - get component (computes if needed)
- `size_t get_component_size(size_t component_id)` - get total segments in component
- `size_t get_visited_count_in_component(size_t component_id)` - count visited segments
- `bool is_component_complete(size_t component_id)` - check if all segments visited
- `std::vector<size_t> get_unvisited_in_component(size_t component_id)` - get unvisited segments

**Optimization**: 
- Compute components lazily when needed
- Cache results in chunk data structure
- For large components spanning chunks, use incremental computation

### 3. Build Graph on Road Network Load (Lazy/Chunk-Based)
**File**: `src/game_setup.cpp`

After loading road network:
- **Phase 1 (Current)**: For small maps (< 10k segments), compute all components immediately
- **Phase 2 (Future)**: For large maps, compute components lazily:
  - Determine starting chunk based on square's initial position
  - Load starting chunk + 8 neighboring chunks (3x3 grid)
  - Compute components only for loaded chunks
  - Set initial `current_component_id` based on starting segment
- Add chunk loading/unloading system:
  - Load chunks within view distance
  - Unload chunks far from square's position
  - Recompute components when chunks are loaded

### 4. Update Stuck Detection Logic
**File**: `src/systems/SquarePhysics.h`

In the stuck detection section (around line 298-372):
- Get current component ID: `size_t current_comp = road_network->get_component_id(road_following.current_segment_index)`
- Check if component is complete: `bool comp_complete = road_network->is_component_complete(current_comp)`
- Only allow jumping if:
  - Component is complete (all segments visited), OR
  - No path exists to any unvisited segment in the component (true dead end)
- Remove the current distance-based check (connection_tolerance * 10.0f) - rely on component analysis instead

### 5. Improve Backtrack Mode Navigation
**File**: `src/systems/SquarePhysics.h`

In backtrack mode (when `road_following.mode == ExplorationMode::Backtrack`):
- Get unvisited segments in current component: `road_network->get_unvisited_in_component(current_comp)`
- If unvisited segments exist:
  - Find nearest unvisited segment (by pathfinding distance)
  - Use pathfinding to navigate there
  - Prefer segments that lead away from recently visited areas
- If no unvisited in component but component not complete:
  - This shouldn't happen, but log a warning
- If component is complete:
  - Log "Component complete!" message
  - Future: transition to next map/component

### 6. Update Component Tracking on Segment Visit
**File**: `src/systems/SquarePhysics.h`

When a segment is visited:
- Check if we've moved to a new component
- If yes, update `road_network->current_component_id`
- Log component completion when appropriate

## Key Changes Summary

1. **Spatial Chunking**: Partition road network into chunks for memory efficiency
2. **Lazy Component Computation**: Compute connected components on-demand per chunk
3. **Sparse Data Structures**: Use bitsets/unordered_sets for visited tracking (memory efficient)
4. **Graph Building**: Build adjacency list based on segment endpoint proximity (connection_tolerance)
5. **Component Detection**: Use DFS/BFS to find connected components (chunk-aware)
6. **Jump Prevention**: Only jump when current component is 100% explored
7. **Smart Backtracking**: Actively navigate to unvisited segments in component using pathfinding
8. **Completion Detection**: Track when a component is fully explored

## Memory Optimization Strategies

### Current Implementation (Small Maps)
- Keep all segments in memory (needed for pathfinding)
- Use `std::bitset` for visited tracking (1 bit per segment)
- Compute components once at startup
- Component IDs: `std::vector<size_t>` (4-8 bytes per segment)

### Future Scalability (Large Maps)
- **Chunk-based loading**: Only keep visible chunks + neighbors in memory
- **Sparse visited tracking**: `std::unordered_set<size_t>` for visited segments (only stores visited)
- **Lazy component computation**: Compute components per chunk on-demand
- **Component caching**: Cache component data in chunk structures
- **Streaming**: Load/unload chunks as square moves (future enhancement)

### Memory Estimates
- Small map (2k segments): ~50KB for components + visited
- Medium map (20k segments): ~500KB for components + visited  
- Large map (200k segments): ~5MB (with chunking, only loaded chunks)
- World-scale: Chunk-based loading keeps memory bounded

## Benefits

- Prevents premature jumping - only jumps when truly necessary
- More efficient exploration - actively seeks unvisited segments in backtrack mode
- Clear completion criteria - 100% of component = done
- Foundation for multi-map progression - ready for "next city" feature
- **Memory scalable** - works for small maps now, ready for world-scale later
- **Lazy computation** - only compute what's needed when needed

## Implementation Notes

### Graph Building Algorithm
- For each segment, find all other segments within `connection_tolerance` distance
- Build adjacency list: `std::vector<std::vector<size_t>> adjacency`
- Use spatial indexing (grid or quadtree) to speed up neighbor finding

### Component Detection Algorithm
- Use DFS or BFS to traverse connected segments
- Assign component IDs: `component_id[segment_index] = component_id`
- Track component metadata: size, visited count, unvisited list

### Chunk System (Future)
- Divide world into fixed-size chunks (e.g., 1000x1000 units)
- Each chunk contains segments that overlap or are within its bounds
- Load chunks in 3x3 grid around square's position
- Unload chunks when square moves away
- Recompute components when chunks are loaded/unloaded

