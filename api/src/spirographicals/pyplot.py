# api/src/spirographicals/pyplot.py
# Author: Aitzaz Imtiaz
# Date: June 13, 2025

from .objects import Figure

# --- Module-level state management ---
# This is the essence of the state-machine nature of a pyplot-style API.
# It keeps track of the "current" or "active" figure and axes.

_active_figure = None

def _get_active_axes(create_if_none=True):
    """
    Internal helper to retrieve the current active axes.
    If no figure or axes exists, it can create a new one.
    """
    global _active_figure
    if _active_figure is None and create_if_none:
        # If no figure is active, create a default one.
        subplots()
    
    if _active_figure and _active_figure.axes:
        # Return the first (and for now, only) axes of the active figure.
        return _active_figure.axes[0]
    
    return None

# --- Public API Functions ---

def figure(figsize=(8.0, 8.0), **kwargs):
    """
    Creates a new figure and makes it the active one.

    Args:
        figsize (tuple, optional): Width, height in inches. Defaults to (8.0, 8.0).
        **kwargs: Additional keyword arguments to pass to the Figure constructor.

    Returns:
        Figure: The newly created figure object.
    """
    global _active_figure
    _active_figure = Figure(figsize=figsize, **kwargs)
    return _active_figure

def subplots(figsize=(8.0, 8.0), **kwargs):
    """
    Create a figure and a set of subplots.

    This is a convenient wrapper that creates a new figure and adds a single
    subplot to it, making them the active figure and axes.

    Args:
        figsize (tuple, optional): Width, height in inches. Defaults to (8.0, 8.0).
        **kwargs: Additional keyword arguments to pass to the Figure constructor.

    Returns:
        tuple: A tuple containing the new Figure and Axes objects.
    """
    global _active_figure
    fig = figure(figsize=figsize, **kwargs)
    ax = fig.add_subplot()
    return fig, ax

def plot(*args, **kwargs):
    """
    Plot y versus x as lines and/or markers on the active axes.

    Args:
        *args: Variable length argument list. Can be `y`, or `x, y`.
        **kwargs: Keyword arguments to be passed to the Axes.plot() method.
    """
    ax = _get_active_axes()
    if ax:
        ax.plot(*args, **kwargs)

def title(label, **kwargs):
    """
    Set a title for the current axes.

    Args:
        label (str): The title text.
        **kwargs: Keyword arguments for title styling (e.g., color).
    """
    ax = _get_active_axes()
    if ax:
        ax.set_title(label, **kwargs)

def xlabel(label, **kwargs):
    """
    Set the label for the x-axis of the current axes.

    Args:
        label (str): The label text.
        **kwargs: Keyword arguments for label styling.
    """
    ax = _get_active_axes()
    if ax:
        ax.set_xlabel(label, **kwargs)

def ylabel(label, **kwargs):
    """
    Set the label for the y-axis of the current axes.

    Args:
        label (str): The label text.
        **kwargs: Keyword arguments for label styling.
    """
    ax = _get_active_axes()
    if ax:
        ax.set_ylabel(label, **kwargs)

def grid(visible=True, **kwargs):
    """
    Configure the grid lines of the current axes.

    Args:
        visible (bool, optional): Whether the grid should be visible. Defaults to True.
        **kwargs: Keyword arguments for grid styling (e.g., color, linestyle).
    """
    ax = _get_active_axes()
    if ax:
        ax.grid(visible=visible, **kwargs)

def show():
    """
    Display all open figures and block until they are closed.

    This function finds the active figure and calls its `show()` method,
    which triggers the backend rendering process.
    """
    if _active_figure:
        _active_figure.show()
    else:
        print("No active figure to show.")

def savefig(path, **kwargs):
    """
    Save the current figure to a file.

    Args:
        path (str): The file path to save the figure to.
        **kwargs: Additional keyword arguments to pass to the Figure's savefig method.
    """
    if _active_figure:
        _active_figure.savefig(path, **kwargs)
    else:
        print("No active figure to save.")
