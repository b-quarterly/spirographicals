use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;

#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HorizontalAlign {
    Left,
    Center,
    Right,
}

#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum VerticalAlign {
    Top,
    Middle,
    Bottom,
}

#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineStyle {
    Solid,
    Dashed,
    Dotted,
    DashDot,
}

#[pyclass]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MarkerStyle {
    Circle,
    Square,
    Triangle,
    Cross,
    Plus,
}

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
}

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
        Ok(Color { r: r as f32 / 255.0, g: g as f32 / 255.0, b: b as f32 / 255.0, a: a as f32 / 255.0 })
    }

    fn __repr__(&self) -> String {
        format!("Color(r={}, g={}, b={}, a={})", self.r, self.g, self.b, self.a)
    }
}

#[pyclass]
#[derive(Debug, Clone)]
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

#[pyclass]
#[derive(Debug, Clone)]
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

#[pyclass]
#[derive(Debug, Clone)]
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
    #[pyo3(signature = (label = None, limits = None, visible = true))]
    fn new(label: Option<TextConfig>, limits: Option<(f32, f32)>, visible: bool) -> Self {
        AxisConfig { label, limits, visible }
    }
}

#[pyclass]
#[derive(Debug, Clone)]
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

#[pyclass]
#[derive(Debug, Clone)]
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

#[derive(Debug, FromPyObject)]
pub enum Artist {
    Line(LineArtist),
    Text(TextArtist),
}

#[pyclass]
#[derive(Debug)]
pub struct PlotAxes {
    #[pyo3(get, set)]
    pub artists: Vec<PyObject>,
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
    pub fn add_line(&mut self, py: Python<'_>, line: LineArtist) {
        self.artists.push(line.into_py(py));
    }
    pub fn add_text(&mut self, py: Python<'_>, text: TextArtist) {
        self.artists.push(text.into_py(py));
    }
    fn __repr__(&self) -> String {
        format!("<PlotAxes with {} artists>", self.artists.len())
    }
}

#[pyclass]
#[derive(Debug)]
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
