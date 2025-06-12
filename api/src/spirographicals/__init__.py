# spirographicals/__init__.py
# Author: Aitzaz Imtiaz
# Date: June 13, 2025

__version__ = "0.1.0"
__author__ = "Aitzaz Imtiaz"

# --- Attempt to import the core Rust extension ---
try:
    # This imports the functions and classes defined in the `#[pymodule]` in Rust.
    from .spirographicals import (
        create_plot,
        PlotConfig
    )
except ImportError as e:
    # Provide a helpful error message if the Rust extension isn't built.
    raise ImportError(
        "Could not import the compiled Rust extension for spirographicals. "
        "Please build the project using 'make build' or 'make install-dev'. "
        f"Original error: {e}"
    ) from e

# --- Import the high-level Python API objects ---
from .objects import Figure, Axes
from . import pyplot

# --- Define what gets imported with `from spirographicals import *` ---
__all__ = [
    'Figure',
    'Axes',
    'pyplot',
    'create_plot',
    'PlotConfig',
    '__version__',
    '__author__'
]
