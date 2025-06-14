# api/src/spirographicals/__init__.py

__version__ = "0.1.0"
__author__ = "Aitzaz Imtiaz"

try:
    # This imports the functions and classes defined in the #[pymodule] in Rust.
    from . import spirographicals as _internal
except ImportError as e:
    raise ImportError(
        "Could not import the compiled Rust extension for spirographicals. "
        "Please run 'make build' or 'make install-dev'. "
        f"Original error: {e}"
    ) from e

# Import the high-level Python API objects that users will interact with.
from .objects import Figure, Axes
from . import pyplot

# Define what gets imported with `from spirographicals import *`
__all__ = [
    'Figure',
    'Axes',
    'pyplot',
    '__version__',
    '__author__',
]
