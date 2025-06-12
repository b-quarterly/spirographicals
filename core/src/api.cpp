#include <spirographicals/spirographicals.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// --- Internal C++ Implementation ---

namespace spiro::internal {

class Path {
public:
    void moveTo(float x, float y) { /* TODO */ }
    void lineTo(float x, float y) { /* TODO */ }
    void arcTo(float x1, float y1, float x2, float y2, float r) { /* TODO */ }
    void cubicBezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y) { /* TODO */ }
    void close() { /* TODO */ }
};

class Pen {
public:
    explicit Pen(const sp_pen_config_t& config) : m_config(config) {}
private:
    sp_pen_config_t m_config;
};

class Canvas {
public:
    Canvas(const sp_window_config_t& config) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

        m_window = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("GLFW window creation failed.");
        }
        glfwMakeContextCurrent(m_window);
        if (config.vsync) {
            glfwSwapInterval(1);
        }
        
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("GLAD initialization failed.");
        }
        
        glViewport(0, 0, config.width, config.height);
        glfwSetWindowUserPointer(m_window, this);
    }

    ~Canvas() {
        if (m_window) {
            glfwDestroyWindow(m_window);
        }
    }

    GLFWwindow* getWindowHandle() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;

public:
    sp_key_callback_t key_callback = nullptr;
    sp_mouse_button_callback_t mouse_button_callback = nullptr;
    sp_cursor_pos_callback_t cursor_pos_callback = nullptr;
};

}

// --- C API Implementation ---

static spiro::internal::Canvas* as_canvas(sp_canvas_t* canvas) { return reinterpret_cast<spiro::internal::Canvas*>(canvas); }
static spiro::internal::Pen* as_pen(sp_pen_t* pen) { return reinterpret_cast<spiro::internal::Pen*>(pen); }
static spiro::internal::Path* as_path(sp_path_t* path) { return reinterpret_cast<spiro::internal::Path*>(path); }

void sp_initialize() {
    if (!glfwInit()) {
        std::cerr << "FATAL: sp_initialize failed." << std::endl;
    }
}

void sp_terminate() {
    glfwTerminate();
}

void sp_set_error_callback(sp_error_callback_t callback) {
    glfwSetErrorCallback(callback);
}

void sp_set_log_level(sp_log_level_t level) {
    // TODO: Implement internal logger
}

sp_canvas_t* sp_create_canvas(const sp_window_config_t* config) {
    try {
        return reinterpret_cast<sp_canvas_t*>(new spiro::internal::Canvas(*config));
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Canvas creation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

void sp_destroy_canvas(sp_canvas_t* canvas) {
    delete as_canvas(canvas);
}

bool sp_canvas_should_close(sp_canvas_t* canvas) {
    return canvas ? glfwWindowShouldClose(as_canvas(canvas)->getWindowHandle()) : true;
}

void sp_begin_frame(sp_canvas_t* canvas) {
    glfwPollEvents();
}

void sp_end_frame(sp_canvas_t* canvas) {
    glfwSwapBuffers(as_canvas(canvas)->getWindowHandle());
}

void sp_clear(sp_canvas_t* canvas, sp_color_rgba_t color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

sp_vec2_t sp_get_canvas_size(sp_canvas_t* canvas) {
    int width, height;
    glfwGetFramebufferSize(as_canvas(canvas)->getWindowHandle(), &width, &height);
    return {(float)width, (float)height};
}

void sp_save_state(sp_canvas_t* canvas) { /* TODO: Graphics state stack push */ }
void sp_restore_state(sp_canvas_t* canvas) { /* TODO: Graphics state stack pop */ }
void sp_reset_transform(sp_canvas_t* canvas) { /* TODO: Reset transformation matrix */ }
void sp_translate(sp_canvas_t* canvas, float x, float y) { /* TODO: Apply translation */ }
void sp_rotate(sp_canvas_t* canvas, float angle_radians) { /* TODO: Apply rotation */ }
void sp_scale(sp_canvas_t* canvas, float x, float y) { /* TODO: Apply scale */ }

sp_pen_t* sp_create_pen(sp_canvas_t* canvas, const sp_pen_config_t* config) {
    return reinterpret_cast<sp_pen_t*>(new spiro::internal::Pen(*config));
}

void sp_destroy_pen(sp_pen_t* pen) {
    delete as_pen(pen);
}

void sp_set_pen(sp_canvas_t* canvas, sp_pen_t* pen) { /* TODO: Set current pen */ }
void sp_set_color(sp_canvas_t* canvas, sp_color_rgba_t color) { /* TODO: Set current color */ }

sp_path_t* sp_create_path(sp_canvas_t* canvas) {
    return reinterpret_cast<sp_path_t*>(new spiro::internal::Path());
}

void sp_destroy_path(sp_path_t* path) {
    delete as_path(path);
}

void sp_path_move_to(sp_path_t* path, float x, float y) { as_path(path)->moveTo(x, y); }
void sp_path_line_to(sp_path_t* path, float x, float y) { as_path(path)->lineTo(x, y); }
void sp_path_arc_to(sp_path_t* path, float x1, float y1, float x2, float y2, float radius) { as_path(path)->arcTo(x1, y1, x2, y2, radius); }
void sp_path_cubic_bezier_to(sp_path_t* path, float c1x, float c1y, float c2x, float c2y, float x, float y) { as_path(path)->cubicBezierTo(c1x, c1y, c2x, c2y, x, y); }
void sp_path_close(sp_path_t* path) { as_path(path)->close(); }
void sp_stroke_path(sp_canvas_t* canvas, sp_path_t* path) { /* TODO: Render path stroke */ }
void sp_fill_path(sp_canvas_t* canvas, sp_path_t* path) { /* TODO: Render path fill */ }

void sp_draw_line(sp_canvas_t* canvas, float x1, float y1, float x2, float y2) { /* TODO: Immediate mode line */ }
void sp_draw_rect(sp_canvas_t* canvas, float x, float y, float w, float h) { /* TODO: Immediate mode rect */ }
void sp_draw_circle(sp_canvas_t* canvas, float cx, float cy, float radius) { /* TODO: Immediate mode circle */ }
void sp_draw_ellipse(sp_canvas_t* canvas, float cx, float cy, float rx, float ry) { /* TODO: Immediate mode ellipse */ }
void sp_fill_rect(sp_canvas_t* canvas, float x, float y, float w, float h) { /* TODO: Immediate mode fill rect */ }
void sp_fill_circle(sp_canvas_t* canvas, float cx, float cy, float radius) { /* TODO: Immediate mode fill circle */ }

sp_font_t* sp_load_font(sp_canvas_t* canvas, const char* path_to_ttf) { return nullptr; }
void sp_destroy_font(sp_font_t* font) {}
void sp_set_font(sp_canvas_t* canvas, sp_font_t* font, float size) {}
void sp_draw_text(sp_canvas_t* canvas, const char* utf8_text, float x, float y) {}
sp_rect_t sp_measure_text(sp_canvas_t* canvas, const char* utf8_text) { return {0,0,0,0}; }

sp_image_t* sp_load_image(sp_canvas_t* canvas, const char* path_to_image) { return nullptr; }
void sp_destroy_image(sp_image_t* image) {}
void sp_draw_image(sp_canvas_t* canvas, sp_image_t* image, float x, float y) {}
void sp_draw_image_rect(sp_canvas_t* canvas, sp_image_t* image, sp_rect_t source_rect, sp_rect_t dest_rect) {}

static void internal_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* internal_canvas = static_cast<spiro::internal::Canvas*>(glfwGetWindowUserPointer(window));
    if (internal_canvas && internal_canvas->key_callback) {
        internal_canvas->key_callback(reinterpret_cast<sp_canvas_t*>(internal_canvas), key, scancode, action, mods);
    }
}

static void internal_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    auto* internal_canvas = static_cast<spiro::internal::Canvas*>(glfwGetWindowUserPointer(window));
    if (internal_canvas && internal_canvas->mouse_button_callback) {
        internal_canvas->mouse_button_callback(reinterpret_cast<sp_canvas_t*>(internal_canvas), button, action, mods);
    }
}

static void internal_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    auto* internal_canvas = static_cast<spiro::internal::Canvas*>(glfwGetWindowUserPointer(window));
    if (internal_canvas && internal_canvas->cursor_pos_callback) {
        internal_canvas->cursor_pos_callback(reinterpret_cast<sp_canvas_t*>(internal_canvas), xpos, ypos);
    }
}

void sp_set_key_callback(sp_canvas_t* canvas, sp_key_callback_t callback) {
    auto* internal_canvas = as_canvas(canvas);
    internal_canvas->key_callback = callback;
    glfwSetKeyCallback(internal_canvas->getWindowHandle(), callback ? internal_key_callback : nullptr);
}

void sp_set_mouse_button_callback(sp_canvas_t* canvas, sp_mouse_button_callback_t callback) {
    auto* internal_canvas = as_canvas(canvas);
    internal_canvas->mouse_button_callback = callback;
    glfwSetMouseButtonCallback(internal_canvas->getWindowHandle(), callback ? internal_mouse_button_callback : nullptr);
}

void sp_set_cursor_pos_callback(sp_canvas_t* canvas, sp_cursor_pos_callback_t callback) {
    auto* internal_canvas = as_canvas(canvas);
    internal_canvas->cursor_pos_callback = callback;
    glfwSetCursorPosCallback(internal_canvas->getWindowHandle(), callback ? internal_cursor_pos_callback : nullptr);
}
