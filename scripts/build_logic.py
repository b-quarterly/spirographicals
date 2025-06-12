# scripts/build_logic.py

import subprocess
import sys
import os
from pathlib import Path

def main():
    """
    Builds the Rust logic layer (Python extension) using Maturin.
    
    This script correctly sets up environment variables so that the Rust
    compiler can find and link against the C++ core library.
    """
    project_root = Path(__file__).parent.parent.resolve()
    cpp_lib_dir = project_root / "target" / "cpp" / "lib"
    wheelhouse_dir = project_root / "target" / "python" / "wheelhouse"

    print(f"Project Root: {project_root}")
    print(f"Expecting C++ library in: {cpp_lib_dir}")
    print(f"Placing Python wheel in: {wheelhouse_dir}")
    
    if not cpp_lib_dir.exists():
        print(f"\n[!!!] Error: C++ library directory not found at '{cpp_lib_dir}'.", file=sys.stderr)
        print("Please run 'make build-core' first.", file=sys.stderr)
        sys.exit(1)

    # Create a copy of the current environment to modify
    env = os.environ.copy()

    # Set up linker paths based on the operating system
    if sys.platform == "win32":
        # On Windows, the linker uses the PATH to find dependent DLLs
        env["PATH"] = f"{cpp_lib_dir};{env.get('PATH', '')}"
    else:
        # On Linux and macOS, RUSTFLAGS is used to pass linker arguments
        rustflags = env.get("RUSTFLAGS", "")
        env["RUSTFLAGS"] = f"{rustflags} -L {cpp_lib_dir}"

    maturin_cmd = [
        "maturin",
        "build",
        "--release",
        "--out", str(wheelhouse_dir)
    ]

    try:
        print("\n[--- Building Rust Logic Layer ---]")
        # We run maturin from the project root where pyproject.toml is located
        subprocess.run(maturin_cmd, check=True, cwd=project_root, env=env)

    except subprocess.CalledProcessError as e:
        print(f"\n[!!!] The Rust build step failed: {e}", file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print("\n[!!!] Error: 'maturin' not found. Have you run 'pip install maturin' in your virtual environment?", file=sys.stderr)
        sys.exit(1)
        
    print(f"\n[OK] Rust build successful. Wheel saved to '{wheelhouse_dir}'.")

if __name__ == "__main__":
    main()
