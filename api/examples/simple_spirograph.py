# api/examples/simple_spirograph.py
#
# A basic example demonstrating the use of the pyplot-style API
# to create and display a spirograph pattern (a hypotrochoid).
#
# Author: Aitzaz Imtiaz
# Date: June 13, 2025

import numpy as np
import spirographicals.pyplot as plt
import sys

def create_hypotrochoid(R, r, d, steps=2000):
    """
    Generates the x and y coordinates for a hypotrochoid curve.
    
    A hypotrochoid is the curve traced by a point attached to a circle of
    radius 'r' rolling around the inside of a fixed circle of radius 'R'.
    The distance of the point from the center of the interior circle is 'd'.
    
    Args:
        R (float): Radius of the fixed circle.
        r (float): Radius of the rolling circle.
        d (float): Distance of the tracing point from the rolling circle's center.
        steps (int): The number of points to generate for the curve.

    Returns:
        tuple: A tuple containing the numpy arrays for x and y coordinates.
    """
    # To ensure the curve is closed, we calculate the required angular range.
    # The curve repeats after lcm(R,r)/R rotations of the larger circle.
    # This simplifies to r/gcd(R,r) rotations of the angle theta.
    if r == 0:
        return np.array([0]), np.array([0])
    
    num_revolutions = r / np.gcd(int(r), int(R))
    theta = np.linspace(0, 2 * np.pi * num_revolutions, steps)
    
    x = (R - r) * np.cos(theta) + d * np.cos(((R - r) / r) * theta)
    y = (R - r) * np.sin(theta) - d * np.sin(((R - r) / r) * theta)
    
    return x, y

def main():
    """Main function to create and display the plot."""
    print("--- Running Spirographicals Example ---")

    # Define the parameters for our spirograph pattern
    # Feel free to change these values to see different patterns!
    large_radius = 10.0
    small_radius = 3.0
    point_distance = 7.0
    
    # Generate the coordinate data for the curve
    x_coords, y_coords = create_hypotrochoid(large_radius, small_radius, point_distance)
    
    # Use the spirographicals pyplot API to build the plot
    try:
        # Create a figure and axes with a dark background
        fig, ax = plt.subplots(figsize=(10, 10), facecolor='#121212')
        
        # Plot the data with a bright, glowing color
        plt.plot(x_coords, y_coords, color='#00FFFF', linewidth=1.5)
        
        # Set a title for the plot
        title_text = f"Hypotrochoid (R={large_radius}, r={small_radius}, d={point_distance})"
        plt.title(title_text, color='white')
        
        # Configure a subtle grid
        plt.grid(visible=True, color='#444444', linestyle='--')
        
        # Display the final rendered window
        plt.show()

    except ImportError as e:
        print(f"\nFATAL ERROR: {e}", file=sys.stderr)
        print("It seems the core library is not built.", file=sys.stderr)
        print("Please run 'make install-dev' from the project root and try again.", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"\nAn unexpected error occurred: {e}", file=sys.stderr)
        sys.exit(1)

    print("\n--- Example Finished ---")
    print("Window should be displayed. Close the window to exit.")

if __name__ == "__main__":
    main()
