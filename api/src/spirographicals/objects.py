# api/src/spirographicals/objects.py

try:
    from . import spirographicals as _internal
except ImportError:
    # This allows the Python API to be inspected even if the Rust part isn't built,
    # though any function call will fail. The __init__.py provides a better error.
    _internal = None


class Figure:
    """
    The top-level container for all the plot elements.
    """
    def __init__(self, figsize=(8.0, 8.0), dpi=100.0, facecolor=None):
        """
        Initializes a new figure.

        Args:
            figsize (tuple, optional): Width, height in inches. Defaults to (8.0, 8.0).
            dpi (float, optional): Dots per inch. Defaults to 100.0.
            facecolor (optional): The background color of the figure. Defaults to None.
        """
        self.figsize = figsize
        self.dpi = dpi
        self.facecolor = facecolor
        self.axes = []

    def add_subplot(self):
        """
        Adds a new Axes subplot to the figure.

        For now, this only supports a single subplot filling the figure.

        Returns:
            Axes: The newly created Axes object.
        """
        if self.axes:
            # For simplicity, returning the existing axes if one exists.
            return self.axes[0]

        ax = Axes(figure=self)
        self.axes.append(ax)
        return ax

    def show(self):
        """
        Displays the figure.

        This method triggers the backend rendering pipeline, sending all collected
        plot data from its axes to the Rust/C++ core to be drawn.
        """
        if not _internal:
            raise RuntimeError("Spirographicals core extension not loaded.")

        print("Figure.show() -> Calling backend renderer...")
        # In a real implementation:
        # plot_data = self._collect_plot_data()
        # _internal.render(plot_data)
        _internal.create_plot()

    def savefig(self, path, dpi=None):
        """
        Saves the current figure to a file.

        Args:
            path (str): The file path to save the figure to.
            dpi (float, optional): The resolution in dots per inch. Defaults to the figure's dpi.
        """
        if not _internal:
            raise RuntimeError("Spirographicals core extension not loaded.")
        
        print(f"Figure.savefig() -> Calling backend to save to '{path}'...")
        # In a real implementation:
        # plot_data = self._collect_plot_data()
        # _internal.save(plot_data, path, dpi or self.dpi)
        pass
    
    def _collect_plot_data(self):
        """Internal helper to gather data from all axes."""
        return {
            "figsize": self.figsize,
            "dpi": self.dpi,
            "facecolor": self.facecolor,
            "axes": [ax._get_data() for ax in self.axes]
        }


class Axes:
    """
    Represents a single plot within a figure, managing artists and coordinate systems.
    """
    def __init__(self, figure):
        """
        Initializes an Axes object. Should not be called directly.
        Use `Figure.add_subplot()`.

        Args:
            figure (Figure): The parent figure this axes belongs to.
        """
        self._figure = figure
        self._plot_commands = []
        self._title = ""
        self._xlabel = ""
        self._ylabel = ""
        self._xlim = None
        self._ylim = None
        self._grid = False

    def plot(self, x, y, color='cyan', linewidth=1.5, label=None):
        """
        Plot y versus x as lines.

        Args:
            x (array-like): The x-coordinates of the data points.
            y (array-like): The y-coordinates of the data points.
            color (str, optional): The color of the line. Defaults to 'cyan'.
            linewidth (float, optional): The width of the line. Defaults to 1.5.
            label (str, optional): The label for the plot, for use in a legend. Defaults to None.
        """
        self._plot_commands.append({
            "type": "line",
            "x": x,
            "y": y,
            "color": color,
            "linewidth": linewidth,
            "label": label
        })
        return self
    
    def set_title(self, title, color='white'):
        """Sets the title for the axes."""
        self._title = {"text": title, "color": color}
        return self

    def set_xlabel(self, label, color='white'):
        """Sets the label for the x-axis."""
        self._xlabel = {"text": label, "color": color}
        return self

    def set_ylabel(self, label, color='white'):
        """Sets the label for the y-axis."""
        self._ylabel = {"text": label, "color": color}
        return self

    def set_xlim(self, left, right):
        """Sets the x-axis view limits."""
        self._xlim = (left, right)
        return self
    
    def set_ylim(self, bottom, top):
        """Sets the y-axis view limits."""
        self._ylim = (bottom, top)
        return self
    
    def grid(self, visible=True, color='gray', linestyle='--'):
        """Configures the grid visibility and style."""
        self._grid = {"visible": visible, "color": color, "linestyle": linestyle}
        return self
    
    def _get_data(self):
        """Internal helper to gather all data and settings for this axes."""
        return {
            "commands": self._plot_commands,
            "title": self._title,
            "xlabel": self._xlabel,
            "ylabel": self._ylabel,
            "xlim": self._xlim,
            "ylim": self._ylim,
            "grid": self._grid,
        }
