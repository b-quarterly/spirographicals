# scripts/build_core.py

import subprocess
import sys
import os
from pathlib import Path

def main():
    """
    Configures, builds, and installs the C++ core library using CMake.
    """
    project_root = Path(__file__).parent.parent.resolve()
    build_dir = project_root / "build"
    install_dir = project_root / "target" / "cpp"

    print(f"Project Root: {project_root}")
    print(f"Build Directory: {build_dir}")
    print(f"Install Directory: {install_dir}")
    
    os.makedirs(build_dir, exist_ok=True)
    
    cmake_configure_cmd = [
        "cmake",
        "-S", str(project_root),
        "-B", str(build_dir),
        "-DCMAKE_BUILD_TYPE=Release",
    ]

    if sys.platform == "win32":
        cmake_configure_cmd.extend(["-G", "Ninja"])
    
    cmake_build_cmd = [
        "cmake",
        "--build", str(build_dir),
        "--config", "Release",
    ]
    
    cmake_install_cmd = [
        "cmake",
        "--install", str(build_dir),
        "--prefix", str(install_dir),
    ]
    
    try:
        print("\n[--- Configuring C++ Core ---]")
        subprocess.run(cmake_configure_cmd, check=True)
        
        print("\n[--- Building C++ Core ---]")
        subprocess.run(cmake_build_cmd, check=True)
        
        print(f"\n[--- Installing C++ Artifacts ---]")
        subprocess.run(cmake_install_cmd, check=True)
        
    except subprocess.CalledProcessError as e:
        print(f"\n[!!!] A C++ build step failed: {e}", file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print("\n[!!!] Error: 'cmake' or 'ninja' not found. Is your C++ build environment configured correctly?", file=sys.stderr)
        sys.exit(1)
        
    print("\n[OK] C++ Core build successful.")

if __name__ == "__main__":
    main()
