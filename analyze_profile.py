import re

with open('profile_grid_new.txt', 'r') as f:
    content = f.read()

# Extract sample counts for key functions
patterns = {
    'RenderPhotoReveal': r'RenderPhotoReveal[^:]*::for_each_with[^\n]*\n[^+]*\+\s+(\d+)',
    'RenderBrick': r'RenderBrick[^:]*::for_each_with[^\n]*\n[^+]*\+\s+(\d+)',
    'HandleCollisions': r'HandleCollisions::once[^\n]*\n[^+]*\+\s+(\d+)',
    'CarPhysics': r'CarPhysics[^:]*::for_each_with[^\n]*\n[^+]*\+\s+(\d+)',
    'RenderCar': r'RenderCar[^:]*::for_each_with[^\n]*\n[^+]*\+\s+(\d+)',
    'SystemManager::render': r'SystemManager::render[^\n]*\n[^+]*\+\s+(\d+)',
    'SystemManager::fixed_tick': r'SystemManager::fixed_tick[^\n]*\n[^+]*\+\s+(\d+)',
}

print("Key System Sample Counts:")
print("=" * 60)
for name, pattern in patterns.items():
    matches = re.findall(pattern, content)
    total = sum(int(m) for m in matches)
    if total > 0:
        print(f"{name:30} {total:5} samples")

# Look for specific bottlenecks
print("\nDetailed Breakdown:")
print("=" * 60)

# RenderPhotoReveal
render_photo = re.findall(r'RenderPhotoReveal[^:]*::for_each_with[^\n]*\n[^+]*\+\s+(\d+)', content)
if render_photo:
    print(f"\nRenderPhotoReveal::for_each_with: {sum(int(x) for x in render_photo)} samples")
    scissor = re.findall(r'EndScissorMode[^\n]*\n[^+]*\+\s+(\d+)', content)
    if scissor:
        print(f"  -> EndScissorMode calls: {sum(int(x) for x in scissor)} samples")

# RenderBrick  
render_brick = re.findall(r'RenderBrick[^:]*::for_each_with[^\n]*\n[^+]*\+\s+(\d+)', content)
if render_brick:
    print(f"\nRenderBrick::for_each_with: {sum(int(x) for x in render_brick)} samples")
    find_rect = re.findall(r'find_rect_height[^\n]*\n[^+]*\+\s+(\d+)', content)
    if find_rect:
        print(f"  -> find_rect_height: {sum(int(x) for x in find_rect)} samples")

# HandleCollisions
handle_coll = re.findall(r'HandleCollisions::once[^\n]*\n[^+]*\+\s+(\d+)', content)
if handle_coll:
    print(f"\nHandleCollisions::once: {sum(int(x) for x in handle_coll)} samples")
