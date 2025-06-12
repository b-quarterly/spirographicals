#pragma once

#include <spirographicals/spirographicals.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>

namespace spiro {

enum class LogLevel {
    Debug = SP_LOG_LEVEL_DEBUG,
    Info = SP_LOG_LEVEL_INFO,
    Warn = SP_LOG_LEVEL_WARN,
    Error = SP_LOG_LEVEL_ERROR,
    Fatal = SP_LOG_LEVEL_FATAL
};

enum class PenStyle {
    Solid = SP_PEN_STYLE_SOLID,
    Dashed = SP_PEN_STYLE_DASHED,
    Dotted = SP_PEN_STYLE_DOTTED,
    DashDot = SP_PEN_STYLE_DASH_DOT
};

enum class LineCap {
    Butt = SP_LINE_CAP_BUTT,
    Round = SP_LINE_CAP_ROUND,
    Square = SP_LINE_CAP_SQUARE
};

enum class LineJoin {
    Miter = SP_LINE_JOIN_MITER,
    Round = SP_LINE_JOIN_ROUND,
    Bevel = SP_LINE_JOIN_BEVEL
};

enum class BlendMode {
    Normal = SP_BLEND_MODE_NORMAL,
    Add = SP_BLEND_MODE_ADD,
    Multiply = SP_BLEND_MODE_MULTIPLY,
    Screen = SP_BLEND_MODE_SCREEN,
    Overlay = SP_BLEND_MODE_OVERLAY
};

enum class TextAlign {
    Left = SP_TEXT_ALIGN_LEFT,
    Center = SP_TEXT_ALIGN_CENTER,
    Right = SP_TEXT_ALIGN_RIGHT
};

struct Vec2 { float x, y; };
struct Color { float r, g, b, a; };
struct Rect { float x, y, w, h; };

using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
using CursorPosCallback = std::function<void(double xpos, double ypos)>;

class Pen;
class Path;
class Image;
class Font;
class Gradient;
class Shader;

class Canvas {
public:
    explicit Canvas(const sp_window_config_t& config) {
        handle_ = sp_create_canvas(&config);
        if (!handle_) { throw std::runtime_error("Failed to create Spirographicals canvas"); }
    }
    Canvas(int width, int height, const std::string& title, bool resizable = true, bool vsync = true) {
        sp_window_config_t config = {width, height, title.c_str(), resizable, vsync};
        handle_ = sp_create_canvas(&config);
        if (!handle_) { throw std::runtime_error("Failed to create Spirographicals canvas"); }
    }
    ~Canvas() { sp_destroy_canvas(handle_); }

    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    Canvas(Canvas&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
    Canvas& operator=(Canvas&& other) noexcept {
        if (this != &other) {
            sp_destroy_canvas(handle_);
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    [[nodiscard]] bool shouldClose() const { return sp_canvas_should_close(handle_); }
    void beginFrame() { sp_begin_frame(handle_); }
    void endFrame() { sp_end_frame(handle_); }
    void clear(const Color& color) { sp_clear(handle_, {color.r, color.g, color.b, color.a}); }
    [[nodiscard]] Vec2 getSize() const { 
        sp_vec2_t s = sp_get_canvas_size(handle_);
        return {s.x, s.y};
    }

    void saveState() { sp_save_state(handle_); }
    void restoreState() { sp_restore_state(handle_); }

    void resetTransform() { sp_reset_transform(handle_); }
    void translate(float x, float y) { sp_translate(handle_, x, y); }
    void rotate(float angleRadians) { sp_rotate(handle_, angleRadians); }
    void scale(float x, float y) { sp_scale(handle_, x, y); }

    void setPen(const Pen& pen);
    void setColor(const Color& color) { sp_set_color(handle_, {color.r, color.g, color.b, color.a}); }

    void strokePath(const Path& path);
    void fillPath(const Path& path);

    void drawLine(float x1, float y1, float x2, float y2) { sp_draw_line(handle_, x1, y1, x2, y2); }
    void drawRect(float x, float y, float w, float h) { sp_draw_rect(handle_, x, y, w, h); }
    void drawCircle(float cx, float cy, float radius) { sp_draw_circle(handle_, cx, cy, radius); }
    void drawEllipse(float cx, float cy, float rx, float ry) { sp_draw_ellipse(handle_, cx, cy, rx, ry); }
    void fillRect(float x, float y, float w, float h) { sp_fill_rect(handle_, x, y, w, h); }
    void fillCircle(float cx, float cy, float radius) { sp_fill_circle(handle_, cx, cy, radius); }

    void setFont(const Font& font, float size);
    void drawText(const std::string& text, float x, float y) { sp_draw_text(handle_, text.c_str(), x, y); }
    [[nodiscard]] Rect measureText(const std::string& text) {
        sp_rect_t r = sp_measure_text(handle_, text.c_str());
        return {r.x, r.y, r.w, r.h};
    }

    void drawImage(const Image& image, float x, float y);
    void drawImageRect(const Image& image, const Rect& source, const Rect& dest);

    void setKeyCallback(KeyCallback cb);
    void setMouseButtonCallback(MouseButtonCallback cb);
    void setCursorPosCallback(CursorPosCallback cb);

    sp_canvas_t* getHandle() const { return handle_; }

private:
    sp_canvas_t* handle_ = nullptr;
};

class Path {
public:
    explicit Path(const Canvas& canvas) { handle_ = sp_create_path(canvas.getHandle()); }
    ~Path() { sp_destroy_path(handle_); }

    Path(const Path&) = delete;
    Path& operator=(const Path&) = delete;
    Path(Path&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
    Path& operator=(Path&& other) noexcept {
        if (this != &other) {
            sp_destroy_path(handle_);
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    Path& moveTo(float x, float y) { sp_path_move_to(handle_, x, y); return *this; }
    Path& lineTo(float x, float y) { sp_path_line_to(handle_, x, y); return *this; }
    Path& arcTo(float x1, float y1, float x2, float y2, float radius) { sp_path_arc_to(handle_, x1, y1, x2, y2, radius); return *this; }
    Path& cubicBezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y) { sp_path_cubic_bezier_to(handle_, c1x, c1y, c2x, c2y, x, y); return *this; }
    Path& close() { sp_path_close(handle_); return *this; }

    sp_path_t* getHandle() const { return handle_; }

private:
    sp_path_t* handle_ = nullptr;
};

class Font {
public:
    explicit Font(const Canvas& canvas, const std::string& path) {
        handle_ = sp_load_font(canvas.getHandle(), path.c_str());
        if (!handle_) { throw std::runtime_error("Failed to load font from: " + path); }
    }
    ~Font() { sp_destroy_font(handle_); }
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    sp_font_t* getHandle() const { return handle_; }
private:
    sp_font_t* handle_ = nullptr;
};

class Image {
public:
    explicit Image(const Canvas& canvas, const std::string& path) {
        handle_ = sp_load_image(canvas.getHandle(), path.c_str());
        if (!handle_) { throw std::runtime_error("Failed to load image from: " + path); }
    }
    ~Image() { sp_destroy_image(handle_); }
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    sp_image_t* getHandle() const { return handle_; }
private:
    sp_image_t* handle_ = nullptr;
};

inline void Canvas::strokePath(const Path& path) { sp_stroke_path(handle_, path.getHandle()); }
inline void Canvas::fillPath(const Path& path) { sp_fill_path(handle_, path.getHandle()); }
inline void Canvas::setFont(const Font& font, float size) { sp_set_font(handle_, font.getHandle(), size); }
inline void Canvas::drawImage(const Image& image, float x, float y) { sp_draw_image(handle_, image.getHandle(), x, y); }
inline void Canvas::drawImageRect(const Image& image, const Rect& source, const Rect& dest) {
    sp_draw_image_rect(handle_, image.getHandle(), {source.x, source.y, source.w, source.h}, {dest.x, dest.y, dest.w, dest.h});
}

}
