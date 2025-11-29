#!/usr/bin/env python3

import os
import json
import math

try:
    import osmnx as ox
except ImportError:
    print("osmnx not found. Please install it with: pip install osmnx")
    print("Or activate the venv and run: source venv/bin/activate && pip install osmnx")
    raise

def lat_lon_to_game_coords(lat, lon, bounds):
    min_lat, max_lat, min_lon, max_lon = bounds
    
    lat_range = max_lat - min_lat
    lon_range = max_lon - min_lon
    
    normalized_lat = (lat - min_lat) / lat_range if lat_range > 0 else 0
    normalized_lon = (lon - min_lon) / lon_range if lon_range > 0 else 0
    
    BRICK_START_X = 50.0
    BRICK_START_Y = 50.0
    BRICK_SIZE = 30.0
    BRICK_SPACING = 5.0
    BRICK_CELL_SIZE = BRICK_SIZE + BRICK_SPACING
    GRID_WIDTH = 100
    GRID_HEIGHT = 50
    
    world_width = BRICK_START_X + (GRID_WIDTH * BRICK_CELL_SIZE)
    world_height = BRICK_START_Y + (GRID_HEIGHT * BRICK_CELL_SIZE)
    
    x = normalized_lon * world_width
    y = (1.0 - normalized_lat) * world_height
    
    return x, y

SQUARE_SIZE = 12.0
MIN_ROAD_WIDTH = 1.5
ROAD_SCALE_FACTOR = SQUARE_SIZE / MIN_ROAD_WIDTH

def get_road_width(highway_type):
    if isinstance(highway_type, list):
        if len(highway_type) == 0:
            return 2.0 * ROAD_SCALE_FACTOR
        highway_type = highway_type[0]
    
    width_map = {
        'motorway': 4.0,
        'motorway_link': 3.5,
        'trunk': 4.0,
        'trunk_link': 3.5,
        'primary': 3.5,
        'primary_link': 3.0,
        'secondary': 3.0,
        'secondary_link': 2.5,
        'tertiary': 2.5,
        'tertiary_link': 2.0,
        'residential': 2.0,
        'unclassified': 2.0,
        'service': 1.5,
    }
    base_width = width_map.get(highway_type, 2.0)
    return base_width * ROAD_SCALE_FACTOR

def download_nyc_roads():
    print("Downloading Times Square area road network from OpenStreetMap...")
    print("This may take a minute...")
    
    times_square_lat = 40.7580
    times_square_lon = -73.9855
    
    try:
        G = ox.graph_from_point((times_square_lat, times_square_lon), dist=1000, network_type='drive', simplify=True)
        print(f"Downloaded graph with {len(G.nodes)} nodes and {len(G.edges)} edges")
    except Exception as e:
        print(f"Error downloading: {e}")
        print("Trying with a slightly larger area (1500m)...")
        try:
            G = ox.graph_from_point((times_square_lat, times_square_lon), dist=1500, network_type='drive', simplify=True)
            print(f"Downloaded graph with {len(G.nodes)} nodes and {len(G.edges)} edges")
        except Exception as e2:
            print(f"Error: {e2}")
            return None
    
    if G is None or len(G.nodes) == 0:
        print("Failed to get graph")
        return None
    
    bounds = (
        min(node[1]['y'] for node in G.nodes(data=True)),
        max(node[1]['y'] for node in G.nodes(data=True)),
        min(node[1]['x'] for node in G.nodes(data=True)),
        max(node[1]['x'] for node in G.nodes(data=True))
    )
    
    north = bounds[1]
    south = bounds[0]
    east = bounds[3]
    west = bounds[2]
    
    print(f"Bounds: {bounds}")
    
    segments = []
    
    for u, v, data in G.edges(data=True):
        if 'geometry' in data:
            coords = list(data['geometry'].coords)
        else:
            u_node = G.nodes[u]
            v_node = G.nodes[v]
            coords = [
                (u_node['x'], u_node['y']),
                (v_node['x'], v_node['y'])
            ]
        
        highway_type = data.get('highway', 'unclassified')
        width = get_road_width(highway_type)
        
        for i in range(len(coords) - 1):
            lon1, lat1 = coords[i]
            lon2, lat2 = coords[i + 1]
            
            x1, y1 = lat_lon_to_game_coords(lat1, lon1, bounds)
            x2, y2 = lat_lon_to_game_coords(lat2, lon2, bounds)
            
            segments.append({
                'start': {'x': float(x1), 'y': float(y1)},
                'end': {'x': float(x2), 'y': float(y2)},
                'width': float(width)
            })
    
    print(f"Created {len(segments)} road segments")
    
    return {
        'segments': segments,
        'bounds': bounds
    }

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    output_dir = os.path.join(project_root, 'resources')
    os.makedirs(output_dir, exist_ok=True)
    
    road_data = download_nyc_roads()
    
    if road_data is None:
        print("Failed to download road data")
        return
    
    output_path = os.path.join(output_dir, 'nyc_roads.json')
    
    with open(output_path, 'w') as f:
        json.dump(road_data, f, indent=2)
    
    print(f"Saved {len(road_data['segments'])} road segments to {output_path}")

if __name__ == '__main__':
    main()

