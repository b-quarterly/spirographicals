# scripts/clean.py

import shutil
from pathlib import Path

def main():
    """
    Removes all build artifacts and temporary files from the project.
    """
    project_root = Path(__file__).parent.parent.resolve()
    print(f"Cleaning workspace in: {project_root}")

    # List of top-level directories to remove completely
    dirs_to_remove = [
        project_root / "build",
        project_root / "target",
        project_root / "logic" / "target",
    ]

    # Glob patterns for finding and removing nested artifact directories
    glob_patterns = [
        "**/.pytest_cache",
        "**/*.egg-info",
        "**/__pycache__",
    ]

    for path in dirs_to_remove:
        if path.exists() and path.is_dir():
            try:
                print(f"Removing directory: {path}")
                shutil.rmtree(path)
            except OSError as e:
                print(f"Error removing directory {path}: {e}", file=sys.stderr)

    for pattern in glob_patterns:
        for path in project_root.glob(pattern):
            if path.exists() and path.is_dir():
                try:
                    print(f"Removing directory: {path}")
                    shutil.rmtree(path)
                except OSError as e:
                    print(f"Error removing directory {path}: {e}", file=sys.stderr)

    print("\n[OK] Workspace cleaned.")

if __name__ == "__main__":
    main()
