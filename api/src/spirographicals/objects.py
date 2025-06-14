# api/src/spirographicals/objects.py

from . import spirographicals as _internal

class Figure:
    """The top-level container for all the plot elements."""
    def __init__(self, figsize=(10.0, 10.0), dpi=100.0, facecolor='#121212'):
        self.figsize = figsize
        self.dpi = dpi
        self.face_color = _internal.Color.from_hex(facecolor)
        self.axes = []

    def add_subplot(self):
        """Adds a new Axes subplot to the figure."""
        if self.axes:
            return self.axes[0]
        ax = Axes(figure=self)
        self.axes.append(ax)
        return ax

    def show(self):
        """
        Triggers the backend rendering pipeline by converting the Python object
        model into Rust data structures and calling the native render function.
        """
        if not _internal:
            raise RuntimeError("Spirographicals core extension not loaded.")

        # 1. Create the top-level Rust Figure object
        rust_figure = _internal.Figure()
        rust_figure.size_pixels = (int(self.figsize[0] * self.dpi), int(self.figsize[1] * self.dpi))
        rust_figure.face_color = self.face_color
        
        # 2. Convert each Python Axes object into a Rust PlotAxes object
        for ax in self.axes:
            rust_axes = _internal.PlotAxes()
            
            # 3. Convert each Python plot command into a Rust Artist object
            for command in ax._plot_commands:
                if command["type"] == "line":
                    points = [_internal.Vec2(p[0], p[1]) for p in zip(command["x"], command["y"])]
                    color = _internal.Color.from_hex(command["color"])
                    
                    line_artist = _internal.LineArtist(
                        points=points,
                        color=color,
                        linewidth=command["linewidth"],
                        style=_internal.LineStyle.Solid # Placeholder
                    )
                    rust_axes.add_artist(line_artist)
            
            rust_figure.add_axes(rust_axes)

        # 4. Call the main render function in Rust
        _internal.render_figure(rust_figure)

    def savefig(self, path, dpi=None):
        """Saves the current figure to a file."""
        print(f"Figure.savefig() -> Not yet implemented. Would save to '{path}'")
        pass


class Axes:
    """Represents a single plot within a figure."""
    def __init__(self, figure):
        self._figure = figure
        self._plot_commands = []
        self._title = ""

    def plot(self, x, y, color='#00FFFF', linewidth=1.5, label=None):
        """Plot y versus x as lines."""
        self._plot_commands.append({
            "type": "line",
            "x": list(x),
            "y": list(y),
            "color": color,
            "linewidth": linewidth,
            "label": label
        })
        return self
    
    def set_title(self, title, color='white'):
        """Sets the title for the axes."""
        self._title = {"text": title, "color": color}
        return self
