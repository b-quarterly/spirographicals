#ifndef SPIROGRAPHICALS_H
#define SPIROGRAPHICALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct sp_canvas_t sp_canvas_t;
typedef struct sp_pen_t sp_pen_t;
typedef struct sp_path_t sp_path_t;
typedef struct sp_image_t sp_image_t;
typedef struct sp_font_t sp_font_t;
typedef struct sp_gradient_t sp_gradient_t;
typedef struct sp_shader_t sp_shader_t;

typedef enum {
    SP_LOG_LEVEL_DEBUG,
    SP_LOG_LEVEL_INFO,
    SP_LOG_LEVEL_WARN,
    SP_LOG_LEVEL_ERROR,
    SP_LOG_LEVEL_FATAL
} sp_log_level_t;

typedef enum {
    SP_PEN_STYLE_SOLID,
    SP_PEN_STYLE_DASHED,
    SP_PEN_STYLE_DOTTED,
    SP_PEN_STYLE_DASH_DOT
} sp_pen_style_t;

typedef enum {
    SP_LINE_CAP_BUTT,
    SP_LINE_CAP_ROUND,
    SP_LINE_CAP_SQUARE
} sp_line_cap_t;

typedef enum {
    SP_LINE_JOIN_MITER,
    SP_LINE_JOIN_ROUND,
    SP_LINE_JOIN_BEVEL
} sp_line_join_t;

typedef enum {
    SP_BLEND_MODE_NORMAL,
    SP_BLEND_MODE_ADD,
    SP_BLEND_MODE_MULTIPLY,
    SP_BLEND_MODE_SCREEN,
    SP_BLEND_MODE_OVERLAY
} sp_blend_mode_t;

typedef enum {
    SP_IMAGE_FILTER_NEAREST,
    SP_IMAGE_FILTER_LINEAR
} sp_image_filter_t;

typedef enum {
    SP_TEXT_ALIGN_LEFT,
    SP_TEXT_ALIGN_CENTER,
    SP_TEXT_ALIGN_RIGHT
} sp_text_align_t;

typedef enum {
    SP_TEXT_BASELINE_TOP,
    SP_TEXT_BASELINE_MIDDLE,
    SP_TEXT_BASELINE_BOTTOM
} sp_text_baseline_t;

typedef struct { float x; float y; } sp_vec2_t;
typedef struct { float r; float g; float b; float a; } sp_color_rgba_t;
typedef struct { float x; float y; float w; float h; } sp_rect_t;
typedef struct { sp_color_rgba_t color; float position; } sp_gradient_stop_t;

typedef struct {
    int width;
    int height;
    const char* title;
    bool resizable;
    bool vsync;
} sp_window_config_t;

typedef struct {
    float line_width;
    sp_line_cap_t line_cap;
    sp_line_join_t line_join;
    float miter_limit;
} sp_pen_config_t;

typedef void (*sp_error_callback_t)(int error_code, const char* description);
typedef void (*sp_key_callback_t)(sp_canvas_t* canvas, int key, int scancode, int action, int mods);
typedef void (*sp_mouse_button_callback_t)(sp_canvas_t* canvas, int button, int action, int mods);
typedef void (*sp_cursor_pos_callback_t)(sp_canvas_t* canvas, double xpos, double ypos);

void sp_initialize();
void sp_terminate();
void sp_set_error_callback(sp_error_callback_t callback);
void sp_set_log_level(sp_log_level_t level);

sp_canvas_t* sp_create_canvas(const sp_window_config_t* config);
void sp_destroy_canvas(sp_canvas_t* canvas);
bool sp_canvas_should_close(sp_canvas_t* canvas);
void sp_begin_frame(sp_canvas_t* canvas);
void sp_end_frame(sp_canvas_t* canvas);
void sp_clear(sp_canvas_t* canvas, sp_color_rgba_t color);
sp_vec2_t sp_get_canvas_size(sp_canvas_t* canvas);

void sp_save_state(sp_canvas_t* canvas);
void sp_restore_state(sp_canvas_t* canvas);
void sp_reset_transform(sp_canvas_t* canvas);
void sp_translate(sp_canvas_t* canvas, float x, float y);
void sp_rotate(sp_canvas_t* canvas, float angle_radians);
void sp_scale(sp_canvas_t* canvas, float x, float y);

sp_pen_t* sp_create_pen(sp_canvas_t* canvas, const sp_pen_config_t* config);
void sp_destroy_pen(sp_pen_t* pen);
void sp_set_pen(sp_canvas_t* canvas, sp_pen_t* pen);
void sp_set_color(sp_canvas_t* canvas, sp_color_rgba_t color);

sp_path_t* sp_create_path(sp_canvas_t* canvas);
void sp_destroy_path(sp_path_t* path);
void sp_path_move_to(sp_path_t* path, float x, float y);
void sp_path_line_to(sp_path_t* path, float x, float y);
void sp_path_arc_to(sp_path_t* path, float x1, float y1, float x2, float y2, float radius);
void sp_path_cubic_bezier_to(sp_path_t* path, float c1x, float c1y, float c2x, float c2y, float x, float y);
void sp_path_close(sp_path_t* path);
void sp_stroke_path(sp_canvas_t* canvas, sp_path_t* path);
void sp_fill_path(sp_canvas_t* canvas, sp_path_t* path);

void sp_draw_line(sp_canvas_t* canvas, float x1, float y1, float x2, float y2);
void sp_draw_rect(sp_canvas_t* canvas, float x, float y, float w, float h);
void sp_draw_circle(sp_canvas_t* canvas, float cx, float cy, float radius);
void sp_draw_ellipse(sp_canvas_t* canvas, float cx, float cy, float rx, float ry);
void sp_fill_rect(sp_canvas_t* canvas, float x, float y, float w, float h);
void sp_fill_circle(sp_canvas_t* canvas, float cx, float cy, float radius);

sp_font_t* sp_load_font(sp_canvas_t* canvas, const char* path_to_ttf);
void sp_destroy_font(sp_font_t* font);
void sp_set_font(sp_canvas_t* canvas, sp_font_t* font, float size);
void sp_draw_text(sp_canvas_t* canvas, const char* utf8_text, float x, float y);
sp_rect_t sp_measure_text(sp_canvas_t* canvas, const char* utf8_text);

sp_image_t* sp_load_image(sp_canvas_t* canvas, const char* path_to_image);
void sp_destroy_image(sp_image_t* image);
void sp_draw_image(sp_canvas_t* canvas, sp_image_t* image, float x, float y);
void sp_draw_image_rect(sp_canvas_t* canvas, sp_image_t* image, sp_rect_t source_rect, sp_rect_t dest_rect);

void sp_set_key_callback(sp_canvas_t* canvas, sp_key_callback_t callback);
void sp_set_mouse_button_callback(sp_canvas_t* canvas, sp_mouse_button_callback_t callback);
void sp_set_cursor_pos_callback(sp_canvas_t* canvas, sp_cursor_pos_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // SPIROGRAPHICALS_H
