import re
import glob
import os

profile_files = glob.glob('profile_*.txt')
if not profile_files:
    print("Error: No profile files found (profile_*.txt)")
    exit(1)

latest_profile = max(profile_files, key=os.path.getmtime)
print(f"Analyzing profile: {latest_profile}\n")

with open(latest_profile, 'r') as f:
    content = f.read()

# Extract sample counts for key functions
# Format: number followed by function name (indentation may include special chars)
patterns = {
    'RenderPhotoReveal': r'(\d+)\s+RenderPhotoReveal::for_each_with',
    'RenderBrick': r'(\d+)\s+RenderBrick::for_each_with',
    'HandleCollisions::once': r'(\d+)\s+HandleCollisions::once',
    'HandleCollisions::for_each': r'(\d+)\s+HandleCollisions::for_each_with',
    'CarPhysics::once': r'(\d+)\s+CarPhysics::once',
    'CarPhysics::for_each': r'(\d+)\s+CarPhysics::for_each_with',
    'RenderCar': r'(\d+)\s+RenderCar::for_each_with',
    'SystemManager::render': r'(\d+)\s+afterhours::SystemManager::render',
    'SystemManager::fixed_tick': r'(\d+)\s+afterhours::SystemManager::fixed_tick',
}

print("Key System Sample Counts:")
print("=" * 60)
for name, pattern in patterns.items():
    matches = re.findall(pattern, content)
    total = sum(int(m) for m in matches)
    if total > 0:
        print(f"{name:35} {total:5} samples")

# Look for specific bottlenecks
print("\nDetailed Breakdown:")
print("=" * 60)

# RenderPhotoReveal
render_photo = re.findall(r'(\d+)\s+RenderPhotoReveal::for_each_with', content)
if render_photo:
    total = sum(int(x) for x in render_photo)
    print(f"\nRenderPhotoReveal::for_each_with: {total} samples")
    scissor = re.findall(r'(\d+)\s+render_backend::EndScissorMode', content)
    if scissor:
        print(f"  -> EndScissorMode calls: {sum(int(x) for x in scissor)} samples")

# RenderBrick  
render_brick = re.findall(r'(\d+)\s+RenderBrick::for_each_with', content)
if render_brick:
    total = sum(int(x) for x in render_brick)
    print(f"\nRenderBrick::for_each_with: {total} samples")
    find_rect = re.findall(r'(\d+)\s+.*find_rect_height', content)
    if find_rect:
        print(f"  -> find_rect_height: {sum(int(x) for x in find_rect)} samples")

# HandleCollisions
handle_coll_once = re.findall(r'(\d+)\s+HandleCollisions::once', content)
handle_coll_each = re.findall(r'(\d+)\s+HandleCollisions::for_each_with', content)
if handle_coll_once or handle_coll_each:
    total_once = sum(int(x) for x in handle_coll_once)
    total_each = sum(int(x) for x in handle_coll_each)
    print(f"\nHandleCollisions::once: {total_once} samples")
    print(f"HandleCollisions::for_each_with: {total_each} samples")
