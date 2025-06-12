// logic/spiro-logic/src/data.rs
// Author: Aitzaz Imtiaz
// Date: June 13, 2025

use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;

// --- Enums for Styling and Configuration ---

/// Specifies the horizontal alignment for text or other objects.
#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HorizontalAlign {
    Left,
    Center,
    Right,
}

/// Specifies the vertical alignment for text or other objects.
#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum VerticalAlign {
    Top,
    Middle,
    Bottom,
}

/// Defines the style for drawing lines.
#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineStyle {
    Solid,
    Dashed,
    Dotted,
    DashDot,
}

/// Defines the marker shape for scatter plots.
#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MarkerStyle {
    Circle,
    Square,
    Triangle,
    Cross,
    Plus,
}

// --- Core Data Structures ---

/// Represents a 2D vector or point.
#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Vec2 {
    #[pyo3(get, set)]
    pub x: f32,
    #[pyo3(get, set)]
    pub y: f32,
}

#[pymethods]
impl Vec2 {
    #[new]
    fn new(x: f32, y: f32) -> Self {
        Vec2 { x, y }
    }

    fn __repr__(&self) -> String {
        format!("Vec2(x={}, y={})", self.x, self.y)
    }

    fn __str__(&self) -> String {
        self.__repr__()
    }
}

/// Represents a color with Red, Green, Blue, and Alpha components.
#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Color {
    #[pyo3(get, set)]
    pub r: f32,
    #[pyo3(get, set)]
    pub g: f32,
    #[pyo3(get, set)]
    pub b: f32,
    #[pyo3(get, set)]
    pub a: f32,
}

#[pymethods]
impl Color {
    #[new]
    fn new(r: f32, g: f32, b: f32, a: f32) -> Self {
        Color { r, g, b, a }
    }

    /// Creates a Color from a hex string (e.g., "#FF00AA" or "#FF00AAFF").
    #[staticmethod]
    fn from_hex(hex_str: &str) -> PyResult<Self> {
        let hex = hex_str.trim_start_matches('#');
        let (r, g, b, a) = match hex.len() {
            6 => (
                u8::from_str_radix(&hex[0..2], 16)?,
                u8::from_str_radix(&hex[2..4], 16)?,
                u8::from_str_radix(&hex[4..6], 16)?,
                255,
            ),
            8 => (
                u8::from_str_radix(&hex[0..2], 16)?,
                u8::from_str_radix(&hex[2..4], 16)?,
                u8::from_str_radix(&hex[4..6], 16)?,
                u8::from_str_radix(&hex[6..8], 16)?,
            ),
            _ => return Err(PyValueError::new_err("Hex string must be 6 or 8 characters long")),
        };
        Ok(Color {
            r: r as f32 / 255.0,
            g: g as f32 / 255.0,
            b: b as f32 / 255.0,
            a: a as f32 / 255.0,
        })
    }

    fn __repr__(&self) -> String {
        format!("Color(r={}, g={}, b={}, a={})", self.r, self.g, self.b, self.a)
    }
}

// --- Configuration Structs ---

/// Holds configuration for plot titles or axis labels.
#[pyclass]
#[derive(Debug, Clone, PartialEq)]
pub struct TextConfig {
    #[pyo3(get, set)]
    pub text: String,
    #[pyo3(get, set)]
    pub color: Color,
    #[pyo3(get, set)]
    pub size: f32,
}

#[pymethods]
impl TextConfig {
    #[new]
    fn new(text: String, color: Color, size: f32) -> Self {
        TextConfig { text, color, size }
    }
}

/// Holds configuration for the plot grid.
#[pyclass]
#[derive(Debug, Clone, PartialEq)]
pub struct GridConfig {
    #[pyo3(get, set)]
    pub visible: bool,
    #[pyo3(get, set)]
    pub color: Color,
    #[pyo3(get, set)]
    pub style: LineStyle,
}

#[pymethods]
impl GridConfig {
    #[new]
    fn new(visible: bool, color: Color, style: LineStyle) -> Self {
        GridConfig { visible, color, style }
    }
}

/// Stores configuration for a single axis.
#[pyclass]
#[derive(Debug, Clone, PartialEq)]
pub struct AxisConfig {
    #[pyo3(get, set)]
    pub label: Option<TextConfig>,
    #[pyo3(get, set)]
    pub limits: Option<(f32, f32)>,
    #[pyo3(get, set)]
    pub visible: bool,
}

#[pymethods]
impl AxisConfig {
    #[new]
    fn new(label: Option<TextConfig>, limits: Option<(f32, f32)>, visible: bool) -> Self {
        AxisConfig { label, limits, visible }
    }
}


// --- Artist Structs (Drawable Elements) ---

/// Represents a line plot artist.
#[pyclass]
#[derive(Debug, Clone, PartialEq)]
pub struct LineArtist {
    #[pyo3(get, set)]
    pub points: Vec<Vec2>,
    #[pyo3(get, set)]
    pub color: Color,
    #[pyo3(get, set)]
    pub linewidth: f32,
    #[pyo3(get, set)]
    pub style: LineStyle,
}

#[pymethods]
impl LineArtist {
    #[new]
    fn new(points: Vec<Vec2>, color: Color, linewidth: f32, style: LineStyle) -> Self {
        LineArtist { points, color, linewidth, style }
    }
}

/// Represents a text artist.
#[pyclass]
#[derive(Debug, Clone, PartialEq)]
pub struct TextArtist {
    #[pyo3(get, set)]
    pub config: TextConfig,
    #[pyo3(get, set)]
    pub position: Vec2,
    #[pyo3(get, set)]
    pub h_align: HorizontalAlign,
    #[pyo3(get, set)]
    pub v_align: VerticalAlign,
}

#[pymethods]
impl TextArtist {
    #[new]
    fn new(config: TextConfig, position: Vec2, h_align: HorizontalAlign, v_align: VerticalAlign) -> Self {
        TextArtist { config, position, h_align, v_align }
    }
}

/// An enum to hold any type of drawable artist.
#[derive(Debug, Clone, PartialEq, FromPyObject)]
pub enum Artist {
    Line(LineArtist),
    Text(TextArtist),
}

// --- High-Level Container Structs ---

/// Represents the data model for a single plot (Axes in Matplotlib terms).
#[pyclass]
#[derive(Debug, Clone, PartialEq)]
pub struct PlotAxes {
    #[pyo3(get, set)]
    pub artists: Vec<PyObject>, // Holds Python objects of artists
    #[pyo3(get, set)]
    pub title: Option<TextConfig>,
    #[pyo3(get, set)]
    pub x_axis: AxisConfig,
    #[pyo3(get, set)]
    pub y_axis: AxisConfig,
    #[pyo3(get, set)]
    pub grid: GridConfig,
}

#[pymethods]
impl PlotAxes {
    #[new]
    fn new() -> Self {
        let default_text_config = TextConfig { text: "".to_string(), color: Color {r:1.0, g:1.0, b:1.0, a:1.0}, size: 12.0 };
        let default_axis_config = AxisConfig { label: None, limits: None, visible: true };
        let default_grid_config = GridConfig { visible: false, color: Color {r:0.5, g:0.5, b:0.5, a:0.5}, style: LineStyle::Dashed };

        PlotAxes {
            artists: Vec::new(),
            title: Some(default_text_config),
            x_axis: default_axis_config.clone(),
            y_axis: default_axis_config,
            grid: default_grid_config,
        }
    }

    /// Adds a new line artist to be drawn.
    pub fn add_line(&mut self, py: Python<'_>, line: LineArtist) {
        self.artists.push(line.into_py(py));
    }

    /// Adds a new text artist to be drawn.
    pub fn add_text(&mut self, py: Python<'_>, text: TextArtist) {
        self.artists.push(text.into_py(py));
    }

    fn __repr__(&self) -> String {
        format!("<PlotAxes with {} artists>", self.artists.len())
    }
}

/// Represents the top-level figure containing all plots.
#[pyclass]
#[derive(Debug, Clone, PartialEq)]
pub struct Figure {
    #[pyo3(get, set)]
    pub axes: Vec<PlotAxes>,
    #[pyo3(get, set)]
    pub face_color: Color,
    #[pyo3(get, set)]
    pub size_pixels: (u32, u32),
}

#[pymethods]
impl Figure {
    #[new]
    fn new() -> Self {
        Figure {
            axes: vec![PlotAxes::new()],
            face_color: Color {r:0.1, g:0.1, b:0.1, a:1.0},
            size_pixels: (800, 800),
        }
    }

    fn __repr__(&self) -> String {
        format!("<Figure with {} axes>", self.axes.len())
    }
}
