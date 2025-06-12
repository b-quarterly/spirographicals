// logic/spiro-logic/src/lib.rs
// Author: Aitzaz Imtiaz
// Date: June 13, 2025

use pyo3::prelude::*;
use pyo3::types::{PyDict, PyList};

// Make the data structures defined in `data.rs` available within this file.
mod data;

// Import the unsafe FFI bindings generated from the C++ headers.
use spiro_core_sys as ffi;

// --- Helper Functions ---

/// Converts our high-level Rust Color struct to the C-style FFI struct.
fn to_c_color(color: &data::Color) -> ffi::sp_color_rgba_t {
    ffi::sp_color_rgba_t {
        r: color.r,
        g: color.g,
        b: color.b,
        a: color.a,
    }
}

/// A struct that mirrors the Python Figure object's data structure.
/// `FromPyObject` allows PyO3 to automatically convert a Python dict to this struct.
#[derive(FromPyObject)]
struct FigureData {
    facecolor: data::Color,
    size_pixels: (u32, u32),
    axes: Vec<PyObject>, // We'll parse these manually
}

/// A struct that mirrors the Python Axes object's data structure.
#[derive(FromPyObject)]
struct AxesData {
    artists: Vec<PyObject>,
    title: Option<data::TextConfig>,
    grid: data::GridConfig,
}

/// An enum that mirrors the Python artist dictionary structure.
#[derive(FromPyObject)]
struct ArtistData {
    #[pyo3(from_py_with = "PyObject::extract")]
    r#type: String,
    // We'll extract other fields based on the type.
}

// --- Core Rendering Function ---

/// The main entry point called from Python to render a figure.
/// This function orchestrates the entire drawing process.
#[pyfunction]
fn render_figure(py: Python<'_>, figure_data: &PyDict) -> PyResult<()> {
    // Convert the Python dictionary into our Rust FigureData struct.
    let figure: FigureData = figure_data.extract()?;

    // Use the figure data to configure the canvas window.
    let window_config = ffi::sp_window_config_t {
        width: figure.size_pixels.0 as i32,
        height: figure.size_pixels.1 as i32,
        title: "Spirographicals".as_ptr() as *const i8,
        resizable: true,
        vsync: true,
    };

    // All drawing happens within an `unsafe` block because we are calling C functions.
    unsafe {
        // Create the main canvas window.
        let canvas = ffi::sp_create_canvas(&window_config);
        if canvas.is_null() {
            return Err(pyo3::exceptions::PyRuntimeError::new_err("Failed to create canvas"));
        }

        // Main render loop. Continues until the user closes the window.
        while !ffi::sp_canvas_should_close(canvas) {
            ffi::sp_begin_frame(canvas);
            ffi::sp_clear(canvas, to_c_color(&figure.facecolor));

            // Iterate through each set of axes in the figure (for now, only one).
            for axes_obj in &figure.axes {
                let axes_dict: &PyDict = axes_obj.downcast_bound(py)?.as_gil_ref().into();
                let axes_data: AxesData = axes_dict.extract()?;

                // Iterate through each artist (line, text, etc.) in the axes.
                for artist_obj in &axes_data.artists {
                    let artist_dict: &PyDict = artist_obj.downcast_bound(py)?.as_gil_ref().into();
                    
                    let artist_type: String = artist_dict.get_item("type")?.unwrap().extract()?;

                    // Use the artist 'type' to decide how to draw it.
                    match artist_type.as_str() {
                        "line" => {
                            let line: data::LineArtist = artist_dict.extract()?;
                            draw_line_artist(canvas, &line);
                        }
                        "text" => {
                             let text: data::TextArtist = artist_dict.extract()?;
                             draw_text_artist(canvas, &text);
                        }
                        _ => {
                            // Silently ignore unknown artist types.
                        }
                    }
                }
            }

            ffi::sp_end_frame(canvas);
        }

        // Clean up the canvas and its resources.
        ffi::sp_destroy_canvas(canvas);
    }

    Ok(())
}

/// Helper function to render a `LineArtist`.
unsafe fn draw_line_artist(canvas: *mut ffi::sp_canvas_t, line: &data::LineArtist) {
    if line.points.len() < 2 {
        return;
    }

    // Create a path object to store the line segments.
    let path = ffi::sp_create_path(canvas);
    
    // Move to the starting point of the line.
    ffi::sp_path_move_to(path, line.points[0].x, line.points[0].y);

    // Draw a line to each subsequent point.
    for point in line.points.iter().skip(1) {
        ffi::sp_path_line_to(path, point.x, point.y);
    }

    // Configure a "pen" with the desired line width.
    let pen_config = ffi::sp_pen_config_t {
        line_width: line.linewidth,
        line_cap: ffi::sp_line_cap_t_SP_LINE_CAP_ROUND,
        line_join: ffi::sp_line_join_t_SP_LINE_JOIN_ROUND,
        miter_limit: 10.0,
    };
    let pen = ffi::sp_create_pen(canvas, &pen_config);

    // Set the current drawing context to use our pen and color.
    ffi::sp_set_pen(canvas, pen);
    ffi::sp_set_color(canvas, to_c_color(&line.color));

    // Execute the drawing command to stroke the path.
    ffi::sp_stroke_path(canvas, path);

    // Clean up the created objects.
    ffi::sp_destroy_pen(pen);
    ffi::sp_destroy_path(path);
}

/// Helper function to render a `TextArtist`.
unsafe fn draw_text_artist(canvas: *mut ffi::sp_canvas_t, text: &data::TextArtist) {
    // This assumes a font has been loaded and is available.
    // For this example, we'll just set the color.
    ffi::sp_set_color(canvas, to_c_color(&text.config.color));
    
    // In a full implementation, you would load/set the font here.
    // let font = ffi::sp_load_font(canvas, "path/to/font.ttf");
    // ffi::sp_set_font(canvas, font, text.config.size);
    
    ffi::sp_draw_text(
        canvas,
        text.config.text.as_ptr() as *const i8,
        text.position.x,
        text.position.y,
    );

    // ffi::sp_destroy_font(font);
}

// --- Test/Placeholder Function ---

/// A simple function to confirm the Rust module is callable from Python.
#[pyfunction]
fn create_plot_from_rust() -> PyResult<String> {
    Ok("Plot object created successfully (message from Rust)".to_string())
}

// --- Python Module Definition ---

/// This block defines the Python module `spirographicals`.
/// It registers all the functions and classes that will be accessible from Python.
#[pymodule]
fn spirographicals(py: Python<'_>, m: &Bound<'_, PyModule>) -> PyResult<()> {
    // --- Register Functions ---
    m.add_function(wrap_pyfunction!(render_figure, m)?)?;
    m.add_function(wrap_pyfunction!(create_plot_from_rust, m)?)?;

    // --- Register Enums ---
    m.add_class::<data::HorizontalAlign>()?;
    m.add_class::<data::VerticalAlign>()?;
    m.add_class::<data::LineStyle>()?;
    m.add_class::<data::MarkerStyle>()?;

    // --- Register Core Data Structs ---
    m.add_class::<data::Vec2>()?;
    m.add_class::<data::Color>()?;

    // --- Register Configuration Structs ---
    m.add_class::<data::TextConfig>()?;
    m.add_class::<data::GridConfig>()?;
    m.add_class::<data::AxisConfig>()?;

    // --- Register Artist Structs ---
    m.add_class::<data::LineArtist>()?;
    m.add_class::<data::TextArtist>()?;
    
    // --- Register High-Level Container Structs ---
    m.add_class::<data::PlotAxes>()?;
    m.add_class::<data::Figure>()?;

    Ok(())
}
