# scripts/package.py

import subprocess
import sys
import os
from pathlib import Path

def main():
    """
    Builds the final, release-optimized Python wheel for distribution.
    """
    project_root = Path(__file__).parent.parent.resolve()
    cpp_lib_dir = project_root / "target" / "cpp" / "lib"
    wheelhouse_dir = project_root / "target" / "python" / "wheelhouse"

    print(f"Project Root: {project_root}")
    print(f"Packaging release wheel to: {wheelhouse_dir}")

    if not cpp_lib_dir.exists():
        print(f"\n[!!!] Error: C++ library not found at '{cpp_lib_dir}'.", file=sys.stderr)
        print("A full build is required before packaging. Run 'make build' first.", file=sys.stderr)
        sys.exit(1)

    # Create a copy of the current environment to modify for the linker
    env = os.environ.copy()

    if sys.platform == "win32":
        env["PATH"] = f"{cpp_lib_dir};{env.get('PATH', '')}"
    else:
        rustflags = env.get("RUSTFLAGS", "")
        env["RUSTFLAGS"] = f"{rustflags} -L {cpp_lib_dir}"

    maturin_cmd = [
        "maturin",
        "build",
        "--release",
        "--out", str(wheelhouse_dir),
    ]

    try:
        print("\n[--- Packaging Release Wheel ---]")
        subprocess.run(maturin_cmd, check=True, cwd=project_root, env=env)

    except subprocess.CalledProcessError as e:
        print(f"\n[!!!] The packaging step failed: {e}", file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print("\n[!!!] Error: 'maturin' not found. Is it installed in your environment?", file=sys.stderr)
        sys.exit(1)
        
    print(f"\n[OK] Package created successfully in '{wheelhouse_dir}'.")

if __name__ == "__main__":
    main()
