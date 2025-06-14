// logic/spiro-logic/src/lib.rs
// Author: Aitzaz Imtiaz
// Date: June 13, 2025

use pyo3::prelude::*;

mod data;
use spiro_core_sys as ffi;

fn to_c_color(color: &data::Color) -> ffi::sp_color_rgba_t {
    ffi::sp_color_rgba_t { r: color.r, g: color.g, b: color.b, a: color.a }
}

#[pyfunction]
fn render_figure(py: Python<'_>, figure: &data::Figure) -> PyResult<()> {
    let window_config = ffi::sp_window_config_t {
        width: figure.size_pixels.0 as i32,
        height: figure.size_pixels.1 as i32,
        title: "Spirographicals".as_ptr() as *const i8,
        resizable: true,
        vsync: true,
    };

    unsafe {
        ffi::sp_initialize();
        let canvas = ffi::sp_create_canvas(&window_config);
        if canvas.is_null() {
            return Err(pyo3::exceptions::PyRuntimeError::new_err("Failed to create canvas"));
        }

        while !ffi::sp_canvas_should_close(canvas) {
            ffi::sp_begin_frame(canvas);
            ffi::sp_clear(canvas, to_c_color(&figure.face_color));

            for axes_obj in &figure.axes {
                let axes_data = axes_obj.downcast_bound::<data::PlotAxes>(py)?;
                for artist_obj in &axes_data.artists {
                    if let Ok(line) = artist_obj.extract::<data::LineArtist>(py) {
                        draw_line_artist(canvas, &line);
                    }
                }
            }
            ffi::sp_end_frame(canvas);
        }

        ffi::sp_destroy_canvas(canvas);
        ffi::sp_terminate();
    }
    Ok(())
}

unsafe fn draw_line_artist(canvas: *mut ffi::sp_canvas_t, line: &data::LineArtist) {
    if line.points.len() < 2 { return; }

    let path = ffi::sp_create_path(canvas);
    if path.is_null() { return; }
    ffi::sp_path_move_to(path, line.points[0].x, line.points[0].y);
    for point in line.points.iter().skip(1) {
        ffi::sp_path_line_to(path, point.x, point.y);
    }
    
    let pen_config = ffi::sp_pen_config_t {
        line_width: line.linewidth,
        line_cap: ffi::sp_line_cap_t::SP_LINE_CAP_ROUND,
        line_join: ffi::sp_line_join_t::SP_LINE_JOIN_ROUND,
        miter_limit: 10.0,
    };
    let pen = ffi::sp_create_pen(canvas, &pen_config);
    if pen.is_null() { ffi::sp_destroy_path(path); return; }

    ffi::sp_set_pen(canvas, pen);
    ffi::sp_set_color(canvas, to_c_color(&line.color));
    ffi::sp_stroke_path(canvas, path);

    ffi::sp_destroy_pen(pen);
    ffi::sp_destroy_path(path);
}

#[pymodule]
fn spirographicals(_py: Python<'_>, m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(render_figure, m)?)?;
    m.add_class::<data::HorizontalAlign>()?;
    m.add_class::<data::VerticalAlign>()?;
    m.add_class::<data::LineStyle>()?;
    m.add_class::<data::MarkerStyle>()?;
    m.add_class::<data::Vec2>()?;
    m.add_class::<data::Color>()?;
    m.add_class::<data::TextConfig>()?;
    m.add_class::<data::GridConfig>()?;
    m.add_class::<data::AxisConfig>()?;
    m.add_class::<data::LineArtist>()?;
    m.add_class::<data::TextArtist>()?;
    m.add_class::<data::PlotAxes>()?;
    m.add_class::<data::Figure>()?;
    Ok(())
}
