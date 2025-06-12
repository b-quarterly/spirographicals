# =============================================================================
# Makefile for the Spirographicals Project
#
# This file provides a high-level command interface for building, testing,
# and managing the project. The complex logic is delegated to the scripts
# located in the ./scripts/ directory.
# =============================================================================

# --- Variables ---------------------------------------------------------------

# Use python3 by default. Can be overridden from the command line.
# e.g., make PYTHON=python3.9 build
PYTHON ?= python3

# Project directories
SCRIPTS_DIR := ./scripts

# --- Phony Targets -----------------------------------------------------------
# Declares targets that are not actual files. This prevents conflicts and
# ensures that the commands will run every time the target is invoked.
.PHONY: all build build-core build-logic package install install-dev test clean help run-example

# --- Primary Targets ---------------------------------------------------------

# The default target, executed when you just run `make`.
all: build

# Builds the entire project from source.
# The dependency chain ensures the C++ core is built before the Rust logic.
build: build-logic
	@echo "✅ Full project build complete."

# Builds only the C++ core library.
build-core:
	@echo "⚙️  Building C++ core library..."
	@$(PYTHON) $(SCRIPTS_DIR)/build_core.py

# Builds the Rust logic layer, which depends on the C++ core.
build-logic: build-core
	@echo "🦀 Building Rust logic layer..."
	@$(PYTHON) $(SCRIPTS_DIR)/build_logic.py

# Packages the project into a distributable Python wheel.
package: build
	@echo "📦 Packaging Python wheel..."
	@$(PYTHON) $(SCRIPTS_DIR)/package.py
	@echo "📦 Wheel available in ./target/python/wheelhouse/"

# --- Development & Workflow Targets ------------------------------------------

# Installs the package in editable/development mode using maturin.
# This is the recommended command for active development, as it provides
# a fast build-and-test loop without full packaging.
install-dev:
	@echo "🛠️  Installing in development (editable) mode..."
	@maturin develop
	@echo "✅ Project installed. You can now import 'spirographicals' in your environment."

# Installs the fully packaged project from the wheel.
# This simulates how a user would install the published package.
install: package
	@echo "🐍 Installing from packaged wheel..."
	@$(PYTHON) -m pip install target/python/wheelhouse/*.whl --force-reinstall

# Runs the complete test suite (C++, Rust, and Python).
# Depends on 'build' to ensure everything is compiled before testing.
test: build
	@echo "🧪 Running full test suite..."
	@$(PYTHON) $(SCRIPTS_DIR)/run_tests.py

# Runs a specific example file to visually verify the build.
# Depends on 'install-dev' to ensure the library is in the current environment.
run-example: install-dev
	@echo "🎨 Running basic example: api/examples/simple_line_plot.py"
	@$(PYTHON) api/examples/simple_line_plot.py

# Cleans up all build artifacts and temporary files.
clean:
	@echo "🧹 Cleaning up build artifacts..."
	@$(PYTHON) $(SCRIPTS_DIR)/clean.py
	@echo "🧹 Workspace is clean."

# --- Help --------------------------------------------------------------------

# Prints this help message.
help:
	@echo "Spirographicals Project Makefile"
	@echo "--------------------------------"
	@echo "Usage: make [target]"
	@echo ""
	@echo "Primary Targets:"
	@echo "  all           Builds the entire project (default)."
	@echo "  build         Alias for 'all'."
	@echo "  package       Builds and packages the project into a Python wheel."
	@echo "  install       Installs the project from the packaged wheel."
	@echo "  test          Runs the full test suite."
	@echo "  clean         Removes all build artifacts."
	@echo ""
	@echo "Development Targets:"
	@echo "  install-dev   Installs the package in editable mode for development."
	@echo "  run-example   Builds and runs a sample visualization script."
	@echo "  build-core    Builds only the C++ core."
	@echo "  build-logic   Builds the Rust layer (and its C++ dependency)."
	@echo "  help          Shows this help message."
