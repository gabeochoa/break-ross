#!/usr/bin/env python3

import os
from PIL import Image, ImageDraw

def generate_test_photo_500x500(output_path):
    width = 500
    height = 500
    image = Image.new('RGB', (width, height), color='black')
    draw = ImageDraw.Draw(image)
    
    center_x = width // 2
    center_y = height // 2
    
    for y in range(height):
        for x in range(width):
            dx = x - center_x
            dy = y - center_y
            distance = (dx * dx + dy * dy) ** 0.5
            max_dist = ((width/2)**2 + (height/2)**2) ** 0.5
            
            angle = (180 / 3.14159) * (3.14159 + (3.14159 if dx == 0 else (dy / dx if dx != 0 else 0)))
            if dx < 0:
                angle += 180
            
            r = int(128 + 127 * (distance / max_dist))
            g = int(128 + 127 * (x / width))
            b = int(128 + 127 * (y / height))
            
            spiral_factor = (distance / 50) % 1.0
            if spiral_factor < 0.5:
                r = min(255, r + 50)
                g = min(255, g + 30)
            
            image.putpixel((x, y), (r, g, b))
    
    for i in range(0, 360, 30):
        angle_rad = i * 3.14159 / 180
        end_x = center_x + int(200 * (3.14159 + angle_rad if angle_rad < 0 else angle_rad))
        end_y = center_y + int(200 * (3.14159 + angle_rad if angle_rad < 0 else angle_rad))
        draw.line([(center_x, center_y), (end_x, end_y)], fill=(255, 255, 255), width=2)
    
    for radius in range(50, 250, 50):
        draw.ellipse([center_x - radius, center_y - radius, center_x + radius, center_y + radius],
                     outline=(200, 150, 100), width=2)
    
    image.save(output_path, 'PNG')
    print(f"Generated test photo: {output_path}")

if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    output_dir = os.path.join(project_root, 'resources', 'images', 'photos')
    os.makedirs(output_dir, exist_ok=True)
    
    output_path = os.path.join(output_dir, 'test_photo_500x500.png')
    generate_test_photo_500x500(output_path)

