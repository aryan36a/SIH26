import numpy as np
import sys
import os

def convert_kitti_bin_to_xyz(bin_path, xyz_path):
    """
    Converts a KITTI LiDAR .bin file (x, y, z, reflectance) into a standard .xyz text file.
    """
    if not os.path.exists(bin_path):
        print(f"Error: Input file '{bin_path}' does not exist.")
        return False
        
    try:
        # KITTI bin files store point cloud data as 4-byte floating point values (x, y, z, reflectance)
        scan = np.fromfile(bin_path, dtype=np.float32)
        points = scan.reshape((-1, 4))
        
        # Extract only x, y, z coordinates (column index 0, 1, and 2)
        xyz_points = points[:, :3]
        
        # Save as a space-separated text file
        np.savetxt(xyz_path, xyz_points, fmt='%.4f', delimiter=' ')
        print(f"Success! Converted '{bin_path}' -> '{xyz_path}'")
        print(f"Total points converted: {xyz_points.shape[0]}")
        return True
        
    except Exception as e:
        print(f"An error occurred during conversion: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python convert.py <input_file.bin> [output_file.xyz]")
        sys.exit(1)
        
    input_bin = sys.argv[1]
    
    # If no output path is given, replace .bin with .xyz in the same folder
    if len(sys.argv) > 2:
        output_xyz = sys.argv[2]
    else:
        output_xyz = os.path.splitext(input_bin)[0] + ".xyz"
        
    convert_kitti_bin_to_xyz(input_bin, output_xyz)
