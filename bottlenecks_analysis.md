# Top 5 Performance Bottlenecks (After Grid Refactor)

Based on profile analysis of `profile_grid_new.txt`:

## 1. **RenderPhotoReveal::for_each_with** - 3,429 samples (47% of total)
**Location**: `RenderPhotoReveal.h:59`

**Issue**: 
- Calls `EndScissorMode()` 3,428 times per frame
- Each scissor mode change triggers GPU command buffer submission
- Heavy Metal/GPU overhead (1,583 samples in `-[_MTLCommandQueue submitCommandBuffer:]`)

**Fix Options** (sorted by complexity, simplest first):

### Option D: Reduce Rect Count with Better Merging (Simplest)
- Improve `rebuild_merged_rects()` algorithm to create fewer, larger rects
- Use greedy rectangle packing or optimal rectangle merging
- **Implementation**:
  - Implement rectangle union algorithm
  - Merge overlapping or adjacent rects more aggressively
  - Cache merge results until grid changes
- **Expected gain**: 2-3x reduction (3,428 → 1,142-1,714 operations)
- **Complexity**: Low (algorithm improvement only)

### Option B: Batch Scissor Operations by Sorting
- Sort `merged_rects` by position (top-to-bottom, left-to-right)
- Group adjacent rects and render with single scissor region
- Use union of bounding boxes for grouped rects
- **Implementation**:
  - Sort `merged_rects` in `rebuild_merged_rects()` by `y` then `x`
  - Group rects that are horizontally/vertically adjacent
  - Render grouped rects with expanded scissor bounds
- **Expected gain**: 2-5x reduction (3,428 → 685-1,714 operations)
- **Complexity**: Medium (requires sorting and grouping logic)

### Option A: Render to Texture with Stencil/Mask (Recommended)
- Create a single render texture for revealed areas
- Use stencil buffer or alpha blending to mask revealed regions
- Render all revealed rects in one pass without scissor mode
- **Implementation**: 
  - Create `revealed_mask_texture` in `IsPhotoReveal` component
  - Update mask texture when `rebuild_merged_rects()` is called
  - Render photo once with mask applied
- **Expected gain**: 10-100x reduction (3,428 → 34-342 operations)
- **Complexity**: Medium-High (texture management and rendering pipeline)

### Option C: Single-Pass Rendering with Custom Shader (Most Complex)
- Render photo texture once with a shader that samples revealed cells
- Use `revealed_cells` bitset to determine visibility per pixel
- Eliminate scissor mode entirely
- **Implementation**:
  - Create custom shader that checks `revealed_cells` bitset
  - Pass bitset data as uniform buffer
  - Single `DrawTexturePro` call with shader
- **Expected gain**: 100x+ reduction (3,428 → ~34 operations)
- **Complexity**: High (requires shader knowledge and GLSL/HLSL)


## 2. **SystemManager::render** - 4,222 samples (58% of render time)
**Location**: `system.h:405`

**Issue**:
- Iterates through all render systems
- Most time spent in RenderPhotoReveal (see #1)

**Fix Options** (sorted by complexity, simplest first):

### Option B: Early Exit for Empty Reveals (Simplest)
- Skip RenderPhotoReveal entirely if no cells are revealed
- Check `revealed_cells.count() == 0` before rendering
- **Implementation**: Add early return in `RenderPhotoReveal::for_each_with`
- **Expected gain**: Minimal (only helps at game start)
- **Complexity**: Low (single line check)

### Option C: Conditional System Registration
- Only register RenderPhotoReveal when photo is loaded
- Unregister when photo is unloaded
- **Implementation**: Dynamic system registration in game loop
- **Expected gain**: Minimal (only helps when photo not loaded)
- **Complexity**: Medium (requires system lifecycle management)

### Option A: Fix RenderPhotoReveal First (Recommended)
- This bottleneck is primarily caused by #1
- Fixing RenderPhotoReveal will automatically improve this
- **Expected gain**: Proportional to #1 fix (47% reduction)
- **Complexity**: Depends on which #1 option is chosen

## 3. **RenderBrick::for_each_with** - 143 samples (2% of total)
**Location**: `RenderBrick.h:83`

**Issue**:
- Rect merging algorithm (`find_rect_height`) takes time
- `std::vector<bool>` operations for tracking processed cells

**Fix Options** (sorted by complexity, simplest first):

### Option B: Use std::bitset Instead of std::vector<bool> (Simplest)
- Replace `std::vector<bool> processed` with `std::bitset<GRID_SIZE>`
- Better memory locality and faster operations
- **Implementation**: Change `processed` type in `RenderBrick.h`
- **Expected gain**: 1.5-2x reduction (143 → 71-95 samples)
- **Complexity**: Low (single type change)

### Option A: Cache Merged Rects in BrickGrid Component (Recommended)
- Store `std::vector<MergedBrickRect>` in `BrickGrid` component
- Recalculate only when `add_health()` or `set_health()` is called
- Add `dirty` flag to track when recalculation is needed
- **Implementation**:
  - Add `mutable std::vector<MergedBrickRect> cached_rects` to `BrickGrid`
  - Add `mutable bool rects_dirty = true` flag
  - Mark dirty in `add_health()` and `set_health()`
  - Calculate on-demand in `RenderBrick` if dirty
- **Expected gain**: 10-20x reduction (143 → 7-14 samples)
- **Complexity**: Medium (adds caching logic)

### Option D: Combine Options A + B
- Cache merged rects AND use `std::bitset`
- Best of both worlds
- **Expected gain**: 15-40x reduction (143 → 3-9 samples)
- **Complexity**: Medium (combines two simpler options)

### Option C: Incremental Rect Updates (Most Complex)
- Instead of full rebuild, update only affected rects when brick dies
- Track which rects contain each brick cell
- Merge/split rects incrementally
- **Implementation**: More complex - requires rect-to-cell mapping
- **Expected gain**: 5-10x reduction (143 → 14-28 samples)
- **Complexity**: High (requires data structure redesign and adjacency tracking)

## 4. **HandleCollisions::once** - 208 samples (2.9% of total)
**Location**: `HandleCollisions.h:158`

**Issue**:
- Still queries balls via EntityQuery (36 samples)
- Grid lookups are fast, but collision math takes time
- Photo reveal rebuild (55 samples) happens during collision

**Fix Options** (sorted by complexity, simplest first):

### Option B: Cache Ball Query Results (Simplest)
- Store ball entities in member variable, update only when balls added/removed
- Use `std::vector<afterhours::Entity*>` cached list
- **Implementation**:
  - Add `mutable std::vector<afterhours::Entity*> cached_balls` to `HandleCollisions`
  - Update cache in `once()` only if entities changed
  - Use cached list instead of query
- **Expected gain**: 17% reduction (208 → 172 samples, saves 36 samples)
- **Complexity**: Low (simple member variable cache)

### Option A: Defer Photo Reveal Rebuild (Recommended)
- Remove `photo_reveal->rebuild_merged_rects()` from collision handler
- Add `dirty` flag to `IsPhotoReveal` component
- Rebuild once at end of frame in a separate system
- **Implementation**:
  - Add `bool merged_rects_dirty = false` to `IsPhotoReveal`
  - Set flag in `set_revealed()` instead of calling `rebuild_merged_rects()`
  - Create `RebuildPhotoReveal` system that runs after collisions
  - Rebuild only if dirty flag is set
- **Expected gain**: 26% reduction (208 → 153 samples, saves 55 samples)
- **Complexity**: Medium (adds system and dirty flag logic)

### Option C: Batch Multiple Reveals Before Rebuild
- Collect all revealed cells during collision phase
- Rebuild merged rects once after all collisions processed
- **Implementation**:
  - Add `std::vector<std::pair<int, int>> pending_reveals` to `IsPhotoReveal`
  - Push to vector in `set_revealed()` instead of immediate rebuild
  - Process all pending reveals in `rebuild_merged_rects()`
- **Expected gain**: 26% reduction (208 → 153 samples)
- **Complexity**: Medium (similar to Option A, adds batching)
- **Note**: Works well with Option A

### Option D: SIMD Collision Math (Most Complex)
- Use SIMD instructions for vector math (distance calculations, normal calculations)
- Requires platform-specific code or library (e.g., xsimd)
- **Implementation**: Rewrite collision math functions with SIMD
- **Expected gain**: 10-20% reduction (208 → 166-187 samples)
- **Complexity**: High (platform-specific, requires SIMD knowledge, harder to maintain)

## 5. **IsPhotoReveal::rebuild_merged_rects** - 55 samples (0.8% of total)
**Location**: `components.h:97`

**Issue**:
- Called during collision handling (when brick dies)
- Rebuilds entire merged rect structure

**Fix Options** (sorted by complexity, simplest first):

### Option C: Use std::bitset for Processed Tracking (Simplest)
- Replace `std::vector<bool> processed` with `std::bitset<GRID_SIZE>`
- Better performance for bit operations
- **Implementation**: Change type in `rebuild_merged_rects()`
- **Expected gain**: 1.2-1.5x reduction (55 → 36-45 samples)
- **Complexity**: Low (single type change)

### Option D: Only Rebuild When Dirty
- Add dirty flag, rebuild only when cells actually changed
- Skip rebuild if no new cells revealed since last rebuild
- **Implementation**: 
  - Add `bool merged_rects_dirty = false` flag
  - Set in `set_revealed()` if cell wasn't already revealed
  - Check flag before rebuilding
- **Expected gain**: Variable (eliminates unnecessary rebuilds)
- **Complexity**: Low (adds simple flag check)
- **Note**: Works well with Option A

### Option A: Defer to End of Frame (Recommended)
- Move rebuild to separate system that runs after collisions
- Same as Option A from #4
- **Implementation**: See #4 Option A
- **Expected gain**: Eliminates 55 samples from collision path (moves to render phase)
- **Complexity**: Medium (adds system and dirty flag logic)

### Option B: Incremental Rect Updates (Most Complex)
- Instead of full rebuild, update only affected rects
- When cell is revealed, find containing rect and expand it
- Or create new small rect and merge with adjacent rects
- **Implementation**:
  - Track rect-to-cell mapping
  - On reveal, find affected rects and update them
  - Merge adjacent rects if they become connected
- **Expected gain**: 5-10x reduction (55 → 5-11 samples)
- **Complexity**: High (requires maintaining rect adjacency graph and incremental update logic)

## Summary

### Recommended Implementation Order

**Phase 1: Quick Wins (Low effort, good gains)**
1. **Defer photo reveal rebuild** (#4 Option A, #5 Option A)
   - Move `rebuild_merged_rects()` to end of frame
   - Add dirty flag to `IsPhotoReveal`
   - **Expected gain**: 55 samples saved from collision path
   - **Effort**: Low (30-60 minutes)

2. **Cache RenderBrick merged rects** (#3 Option A)
   - Cache rects in `BrickGrid` component
   - Recalculate only when bricks change
   - **Expected gain**: 130-136 samples saved (143 → 7-14)
   - **Effort**: Low-Medium (1-2 hours)

**Phase 2: Major Performance Fix (High impact)**
3. **Fix RenderPhotoReveal scissor batching** (#1 Option A or C)
   - **Option A (Texture + Mask)**: Medium effort, 10-100x gain
   - **Option C (Shader)**: Higher effort, 100x+ gain
   - **Expected gain**: 3,000-3,400 samples saved (47% of total)
   - **Effort**: Medium-High (2-4 hours)

**Phase 3: Additional Optimizations (Diminishing returns)**
4. **Cache ball query** (#4 Option B)
   - Store ball entities in member variable
   - **Expected gain**: 36 samples saved
   - **Effort**: Low (30 minutes)

5. **Use std::bitset** (#3 Option B, #5 Option C)
   - Replace `std::vector<bool>` with `std::bitset`
   - **Expected gain**: 20-30 samples saved
   - **Effort**: Low (15 minutes)

### Expected Total Improvement
- **Before**: 7,257 total samples
- **After Phase 1**: 7,072 samples (2.5% improvement)
- **After Phase 2**: 3,672-4,072 samples (43-49% improvement)
- **After Phase 3**: 3,606-4,016 samples (45-50% improvement)

### Biggest Win
**RenderPhotoReveal scissor mode batching** (#1)
- **Current**: 3,428 scissor mode changes → 1,583 GPU submissions
- **Potential**: 10-100x reduction → 34-342 operations
- **Impact**: 47% of total frame time

