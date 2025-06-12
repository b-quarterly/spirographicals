import subprocess
import sys
from pathlib import Path

def run_command(command, working_dir, error_message):
    try:
        print(f"\n[--- Running: {' '.join(command)} in {working_dir} ---]")
        subprocess.run(command, check=True, cwd=working_dir)
    except subprocess.CalledProcessError as e:
        print(f"\n[!!!] Test step failed: {error_message}", file=sys.stderr)
        print(f"Command failed with exit code {e.returncode}", file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"\n[!!!] Error: Command '{command[0]}' not found.", file=sys.stderr)
        print("Please ensure required tools are installed and in your PATH.", file=sys.stderr)
        sys.exit(1)

def main():
    project_root = Path(__file__).parent.parent.resolve()
    cpp_build_dir = project_root / "build"
    rust_workspace_dir = project_root / "logic"

    if cpp_build_dir.exists():
        ctest_cmd = ["ctest", "--output-on-failure", "--test-dir", str(cpp_build_dir)]
        run_command(ctest_cmd, cpp_build_dir, "CTest execution failed.")
    else:
        print(f"[WARN] C++ build directory '{cpp_build_dir}' not found. C++ tests skipped.")

    if rust_workspace_dir.exists():
        cargo_cmd = ["cargo", "test", "--workspace"]
        run_command(cargo_cmd, rust_workspace_dir, "Cargo test execution failed.")
    else:
        print(f"[WARN] Rust logic directory '{rust_workspace_dir}' not found. Rust tests skipped.")
    
    maturin_cmd = ["maturin", "develop"]
    run_command(maturin_cmd, project_root, "Maturin develop (for testing) failed.")

    pytest_cmd = ["pytest", "-v"]
    run_command(pytest_cmd, project_root, "Pytest execution failed.")

    print("\n[OK] All test suites passed successfully.")

if __name__ == "__main__":
    main()
