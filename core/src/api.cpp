#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <spirographicals/spirographicals.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <stack>
#include <fstream>
#include <algorithm>

namespace spiro::internal {

// --- Shader Sources ---
const char* BATCH_VERTEX_SHADER = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 a_Pos;
    layout (location = 1) in vec4 a_Color;
    layout (location = 2) in vec2 a_TexCoord;
    layout (location = 3) in float a_TexId;

    out vec4 v_Color;
    out vec2 v_TexCoord;
    out float v_TexId;

    uniform mat4 u_ViewProjection;

    void main() {
        v_Color = a_Color;
        v_TexCoord = a_TexCoord;
        v_TexId = a_TexId;
        gl_Position = u_ViewProjection * vec4(a_Pos.x, a_Pos.y, 0.0, 1.0);
    }
)glsl";

const char* BATCH_FRAGMENT_SHADER = R"glsl(
    #version 330 core
    out vec4 FragColor;

    in vec4 v_Color;
    in vec2 v_TexCoord;
    in float v_TexId;

    uniform sampler2D u_Textures[16];

    void main() {
        if (v_TexId > -0.5) {
            int tid = int(round(v_TexId));
            vec4 texColor = texture(u_Textures[tid], v_TexCoord);
            FragColor = v_Color * texColor;
        } else {
            FragColor = v_Color;
        }
    }
)glsl";

// --- Data Structures ---
struct Vertex {
    glm::vec2 position;
    glm::vec4 color;
    glm::vec2 texCoord;
    float texId;
};

struct Path {
    std::vector<sp_vec2_t> points;
};

struct Pen {
    sp_pen_config_t config;
};

struct Image {
    GLuint textureId = 0;
    int width = 0;
    int height = 0;
};

struct Font {
    std::vector<unsigned char> ttf_buffer;
    stbtt_fontinfo info;
    stbtt_bakedchar cdata[96]; // ASCII 32..126
};

struct State {
    glm::mat4 transform;
    sp_color_rgba_t color;
    sp_pen_t* pen;
    sp_font_t* font;
    float font_size;
};

class Renderer {
private:
    GLuint m_vao = 0, m_vbo = 0;
    GLuint m_shaderProgram = 0;
    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_textureSlots;
    
    static const size_t MAX_VERTICES = 60000;
    static const size_t MAX_TEXTURES = 16;
    
public:
    std::stack<State> stateStack;

    Renderer() {
        m_vertices.reserve(MAX_VERTICES);
        m_textureSlots.reserve(MAX_TEXTURES);

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &BATCH_VERTEX_SHADER, nullptr);
        glCompileShader(vs);
        
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &BATCH_FRAGMENT_SHADER, nullptr);
        glCompileShader(fs);

        m_shaderProgram = glCreateProgram();
        glAttachShader(m_shaderProgram, vs);
        glAttachShader(m_shaderProgram, fs);
        glLinkProgram(m_shaderProgram);
        glDeleteShader(vs);
        glDeleteShader(fs);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoord));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texId));
        glEnableVertexAttribArray(3);
        
        State initialState;
        initialState.transform = glm::mat4(1.0f);
        initialState.color = {1.0f, 1.0f, 1.0f, 1.0f};
        initialState.pen = nullptr;
        initialState.font = nullptr;
        initialState.font_size = 16.0f;
        stateStack.push(initialState);
    }

    ~Renderer() {
        glDeleteProgram(m_shaderProgram);
        glDeleteBuffers(1, &m_vbo);
        glDeleteVertexArrays(1, &m_vao);
    }
    
    void beginFrame(int width, int height) {
        m_vertices.clear();
        m_textureSlots.clear();
        
        glUseProgram(m_shaderProgram);
        glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
        glm::mat4 view = stateStack.top().transform;
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "u_ViewProjection"), 1, GL_FALSE, glm::value_ptr(projection * view));
        
        int samplers[MAX_TEXTURES];
        for(int i = 0; i < MAX_TEXTURES; i++) samplers[i] = i;
        glUniform1iv(glGetUniformLocation(m_shaderProgram, "u_Textures"), MAX_TEXTURES, samplers);
    }

    void flush() {
        if (m_vertices.empty()) return;

        for (uint32_t i = 0; i < m_textureSlots.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_textureSlots[i]);
        }
        
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(Vertex), m_vertices.data());
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
        glDisable(GL_BLEND);
        
        m_vertices.clear();
        m_textureSlots.clear();
    }
    
    float getTextureSlot(GLuint textureId) {
        for (size_t i = 0; i < m_textureSlots.size(); ++i) {
            if (m_textureSlots[i] == textureId) return (float)i;
        }
        if (m_textureSlots.size() >= MAX_TEXTURES) {
            flush();
            return getTextureSlot(textureId);
        }
        m_textureSlots.push_back(textureId);
        return (float)m_textureSlots.size() - 1.0f;
    }

    void addQuad(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4, const glm::vec4& color, float texId, const glm::vec4& texCoords) {
        if (m_vertices.size() + 6 > MAX_VERTICES) flush();
        m_vertices.push_back({p1, color, {texCoords.x, texCoords.y}, texId});
        m_vertices.push_back({p2, color, {texCoords.z, texCoords.y}, texId});
        m_vertices.push_back({p3, color, {texCoords.z, texCoords.w}, texId});
        m_vertices.push_back({p1, color, {texCoords.x, texCoords.y}, texId});
        m_vertices.push_back({p3, color, {texCoords.z, texCoords.w}, texId});
        m_vertices.push_back({p4, color, {texCoords.x, texCoords.w}, texId});
    }
};

class Canvas {
public:
    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Renderer> m_renderer;

    sp_key_callback_t key_callback = nullptr;
    sp_mouse_button_callback_t mouse_button_callback = nullptr;
    sp_cursor_pos_callback_t cursor_pos_callback = nullptr;
    
    Canvas(const sp_window_config_t& config) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        m_window = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
        if (!m_window) throw std::runtime_error("glfwCreateWindow failed");
        
        glfwMakeContextCurrent(m_window);
        if (config.vsync) glfwSwapInterval(1);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) throw std::runtime_error("gladLoadGLLoader failed");
        
        m_renderer = std::make_unique<Renderer>();
        glfwSetWindowUserPointer(m_window, this);
    }
    ~Canvas() { if (m_window) glfwDestroyWindow(m_window); }
};
}

using namespace spiro::internal;

static Canvas* as_canvas(sp_canvas_t* c) { return reinterpret_cast<Canvas*>(c); }
static Pen* as_pen(sp_pen_t* p) { return reinterpret_cast<Pen*>(p); }
static Path* as_path(sp_path_t* p) { return reinterpret_cast<Path*>(p); }
static Image* as_image(sp_image_t* i) { return reinterpret_cast<Image*>(i); }
static Font* as_font(sp_font_t* f) { return reinterpret_cast<Font*>(f); }

void sp_initialize() { glfwInit(); }
void sp_terminate() { glfwTerminate(); }
void sp_set_error_callback(sp_error_callback_t cb) { glfwSetErrorCallback(cb); }
void sp_set_log_level(sp_log_level_t level) {}

sp_canvas_t* sp_create_canvas(const sp_window_config_t* config) {
    try { return reinterpret_cast<sp_canvas_t*>(new Canvas(*config)); }
    catch (const std::exception& e) { std::cerr << e.what() << std::endl; return nullptr; }
}
void sp_destroy_canvas(sp_canvas_t* c) { delete as_canvas(c); }
bool sp_canvas_should_close(sp_canvas_t* c) { return c ? glfwWindowShouldClose(as_canvas(c)->m_window) : true; }
void sp_begin_frame(sp_canvas_t* c) { 
    glfwPollEvents(); 
    int w, h;
    glfwGetFramebufferSize(as_canvas(c)->m_window, &w, &h);
    glViewport(0, 0, w, h);
    as_canvas(c)->m_renderer->beginFrame(w, h);
}
void sp_end_frame(sp_canvas_t* c) {
    as_canvas(c)->m_renderer->flush();
    glfwSwapBuffers(as_canvas(c)->m_window);
}
void sp_clear(sp_canvas_t* c, sp_color_rgba_t color) { glClearColor(color.r, color.g, color.b, color.a); glClear(GL_COLOR_BUFFER_BIT); }
sp_vec2_t sp_get_canvas_size(sp_canvas_t* c) { int w, h; glfwGetWindowSize(as_canvas(c)->m_window, &w, &h); return {(float)w, (float)h}; }

void sp_save_state(sp_canvas_t* c) { as_canvas(c)->m_renderer->stateStack.push(as_canvas(c)->m_renderer->stateStack.top()); }
void sp_restore_state(sp_canvas_t* c) { if (as_canvas(c)->m_renderer->stateStack.size() > 1) as_canvas(c)->m_renderer->stateStack.pop(); }
void sp_reset_transform(sp_canvas_t* c) { as_canvas(c)->m_renderer->stateStack.top().transform = glm::mat4(1.0f); }
void sp_translate(sp_canvas_t* c, float x, float y) { auto& stack = as_canvas(c)->m_renderer->stateStack; stack.top().transform = glm::translate(stack.top().transform, glm::vec3(x,y,0)); }
void sp_rotate(sp_canvas_t* c, float angle_radians) { auto& stack = as_canvas(c)->m_renderer->stateStack; stack.top().transform = glm::rotate(stack.top().transform, angle_radians, glm::vec3(0,0,1)); }
void sp_scale(sp_canvas_t* c, float x, float y) { auto& stack = as_canvas(c)->m_renderer->stateStack; stack.top().transform = glm::scale(stack.top().transform, glm::vec3(x,y,1)); }

sp_pen_t* sp_create_pen(sp_canvas_t* c, const sp_pen_config_t* config) { return reinterpret_cast<sp_pen_t*>(new Pen{*config}); }
void sp_destroy_pen(sp_pen_t* p) { delete as_pen(p); }
void sp_set_pen(sp_canvas_t* c, sp_pen_t* p) { as_canvas(c)->m_renderer->stateStack.top().pen = p; }
void sp_set_color(sp_canvas_t* c, sp_color_rgba_t color) { as_canvas(c)->m_renderer->stateStack.top().color = color; }

sp_path_t* sp_create_path(sp_canvas_t* c) { return reinterpret_cast<sp_path_t*>(new Path()); }
void sp_destroy_path(sp_path_t* p) { delete as_path(p); }
void sp_path_move_to(sp_path_t* p, float x, float y) { as_path(p)->points.push_back({x,y}); }
void sp_path_line_to(sp_path_t* p, float x, float y) { as_path(p)->points.push_back({x,y}); }
void sp_path_arc_to(sp_path_t* p, float x1, float y1, float x2, float y2, float r) {}
void sp_path_cubic_bezier_to(sp_path_t* p, float c1x, float c1y, float c2x, float c2y, float x, float y) {}
void sp_path_close(sp_path_t* p) { if (as_path(p)->points.size() > 1) as_path(p)->points.push_back(as_path(p)->points.front()); }
void sp_stroke_path(sp_canvas_t* c, sp_path_t* p) {
    auto path = as_path(p);
    auto pen = as_pen(as_canvas(c)->m_renderer->stateStack.top().pen);
    if (path->points.size() < 2 || !pen) return;
    for (size_t i = 0; i < path->points.size() - 1; ++i) {
        glm::vec2 p1 = {path->points[i].x, path->points[i].y};
        glm::vec2 p2 = {path->points[i+1].x, path->points[i+1].y};
        glm::vec2 dir = glm::normalize(p2 - p1);
        glm::vec2 n(-dir.y, dir.x);
        n *= pen->config.line_width / 2.0f;
        
        glm::vec4 color = {1,1,1,1};
        auto& state_color = as_canvas(c)->m_renderer->stateStack.top().color;
        color = {state_color.r, state_color.g, state_color.b, state_color.a};
        
        as_canvas(c)->m_renderer->addQuad(p1 - n, p2 - n, p2 + n, p1 + n, color, -1.0f, {0,0,0,0});
    }
}
void sp_fill_path(sp_path_t* p, sp_path_t* path) {}

void sp_draw_line(sp_canvas_t* c, float x1, float y1, float x2, float y2) { sp_stroke_path(c, nullptr); }
void sp_draw_rect(sp_canvas_t* c, float x, float y, float w, float h) { auto color = as_canvas(c)->m_renderer->stateStack.top().color; as_canvas(c)->m_renderer->addQuad({x,y},{x+w,y},{x+w,y+h},{x,y+h}, {color.r,color.g,color.b,color.a}, -1.0f, {0,0,0,0}); }
void sp_fill_rect(sp_canvas_t* c, float x, float y, float w, float h) { sp_draw_rect(c,x,y,w,h); }
void sp_draw_circle(sp_canvas_t* c, float cx, float cy, float r) {}
void sp_draw_ellipse(sp_canvas_t* c, float cx, float cy, float rx, float ry) {}
void sp_fill_circle(sp_canvas_t* c, float cx, float cy, float r) {}

sp_font_t* sp_load_font(sp_canvas_t* c, const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return nullptr;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    auto font = new Font();
    font->ttf_buffer.resize(size);
    if (file.read((char*)font->ttf_buffer.data(), size)) {
        stbtt_InitFont(&font->info, font->ttf_buffer.data(), 0);
        return reinterpret_cast<sp_font_t*>(font);
    }
    delete font;
    return nullptr;
}
void sp_destroy_font(sp_font_t* f) { delete as_font(f); }
void sp_set_font(sp_canvas_t* c, sp_font_t* f, float size) { auto& stack = as_canvas(c)->m_renderer->stateStack; stack.top().font = f; stack.top().font_size = size; }
void sp_draw_text(sp_canvas_t* c, const char* text, float x, float y) {
    auto& state = as_canvas(c)->m_renderer->stateStack.top();
    auto font = as_font(state.font);
    if (!font) return;
    
    float scale = stbtt_ScaleForPixelHeight(&font->info, state.font_size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &lineGap);
    y += ascent * scale;

    const char* ptr = text;
    while (*ptr) {
        int codepoint;
        ptr += stbtt_GetCodepointSloppy(ptr, &codepoint);
        if (codepoint == '\n') {
            y += (ascent - descent + lineGap) * scale;
            x = 0; // simplistic newline
            continue;
        }
        int g = stbtt_FindGlyphIndex(&font->info, codepoint);
        if (g == 0) continue;

        int x0, y0, x1, y1;
        stbtt_GetGlyphBitmapBox(&font->info, g, scale, scale, &x0, &y0, &x1, &y1);
        
        // draw glyph here
        int advance, lsb;
        stbtt_GetGlyphHMetrics(&font->info, g, &advance, &lsb);
        x += advance * scale;
        int kern = stbtt_GetCodepointKernAdvance(&font->info, codepoint, stbtt_GetCodepointSloppy(ptr, &codepoint));
        x += kern * scale;
    }
}
sp_rect_t sp_measure_text(sp_canvas_t* c, const char* text) { return {0,0,0,0}; }

sp_image_t* sp_load_image(sp_canvas_t* c, const char* path) {
    auto image = new Image();
    int channels;
    unsigned char* data = stbi_load(path, &image->width, &image->height, &channels, 4);
    if (!data) { delete image; return nullptr; }
    glGenTextures(1, &image->textureId);
    glBindTexture(GL_TEXTURE_2D, image->textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return reinterpret_cast<sp_image_t*>(image);
}
void sp_destroy_image(sp_image_t* i) { if(i) { glDeleteTextures(1, &as_image(i)->textureId); delete as_image(i); } }
void sp_draw_image(sp_canvas_t* c, sp_image_t* i, float x, float y) {
    auto image = as_image(i);
    if (!image) return;
    float tid = as_canvas(c)->m_renderer->getTextureSlot(image->textureId);
    as_canvas(c)->m_renderer->addQuad({x, y}, {x + image->width, y}, {x + image->width, y + image->height}, {x, y + image->height}, {1,1,1,1}, tid, {0,0,1,1});
}
void sp_draw_image_rect(sp_canvas_t* c, sp_image_t* i, sp_rect_t src, sp_rect_t dest) {
    auto image = as_image(i);
    if (!image) return;
    float tid = as_canvas(c)->m_renderer->getTextureSlot(image->textureId);
    glm::vec4 texCoords = {src.x/image->width, src.y/image->height, (src.x+src.w)/image->width, (src.y+src.h)/image->height};
    as_canvas(c)->m_renderer->addQuad({dest.x, dest.y}, {dest.x + dest.w, dest.y}, {dest.x + dest.w, dest.y + dest.h}, {dest.x, dest.y + dest.h}, {1,1,1,1}, tid, texCoords);
}

static void internal_key_cb(GLFWwindow* w, int k, int s, int a, int m) { auto* c = static_cast<Canvas*>(glfwGetWindowUserPointer(w)); if(c && c->key_callback) c->key_callback(reinterpret_cast<sp_canvas_t*>(c),k,s,a,m); }
static void internal_mouse_btn_cb(GLFWwindow* w, int b, int a, int m) { auto* c = static_cast<Canvas*>(glfwGetWindowUserPointer(w)); if(c && c->mouse_button_callback) c->mouse_button_callback(reinterpret_cast<sp_canvas_t*>(c),b,a,m); }
static void internal_cursor_pos_cb(GLFWwindow* w, double x, double y) { auto* c = static_cast<Canvas*>(glfwGetWindowUserPointer(w)); if(c && c->cursor_pos_callback) c->cursor_pos_callback(reinterpret_cast<sp_canvas_t*>(c),x,y); }

void sp_set_key_callback(sp_canvas_t* c, sp_key_callback_t cb) { as_canvas(c)->key_callback = cb; glfwSetKeyCallback(as_canvas(c)->m_window, cb ? internal_key_cb : nullptr); }
void sp_set_mouse_button_callback(sp_canvas_t* c, sp_mouse_button_callback_t cb) { as_canvas(c)->mouse_button_callback = cb; glfwSetMouseButtonCallback(as_canvas(c)->m_window, cb ? internal_mouse_btn_cb : nullptr); }
void sp_set_cursor_pos_callback(sp_canvas_t* c, sp_cursor_pos_callback_t cb) { as_canvas(c)->cursor_pos_callback = cb; glfwSetCursorPosCallback(as_canvas(c)->m_window, cb ? internal_cursor_pos_cb : nullptr); }
