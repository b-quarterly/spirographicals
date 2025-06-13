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

struct Vertex { glm::vec2 p; glm::vec4 c; glm::vec2 t; float tid; };
struct Path { std::vector<sp_vec2_t> points; };
struct Pen { sp_pen_config_t config; };
struct Image { GLuint textureId=0; int w=0; int h=0; };
struct Font { std::vector<unsigned char> ttf_buffer; GLuint tex=0; stbtt_bakedchar cdata[96]; };
struct State { glm::mat4 transform; sp_color_rgba_t color; sp_pen_t* pen; sp_font_t* font; float font_size; };

class Renderer {
private:
    GLuint m_vao=0, m_vbo=0, m_prog=0;
    std::vector<Vertex> m_verts;
    std::vector<GLuint> m_tex_slots;
    static const size_t MAX_VERTS = 60000;
    static const size_t MAX_TEX = 16;
public:
    std::stack<State> stateStack;
    Renderer() {
        m_verts.reserve(MAX_VERTS);
        m_tex_slots.reserve(MAX_TEX);
        const char* vs_src = "#version 330 core\nlayout (location = 0) in vec2 p; layout (location = 1) in vec4 c; layout (location = 2) in vec2 t; layout (location = 3) in float tid; out vec4 vc; out vec2 vt; out float vtid; uniform mat4 u_vp; void main() { vc=c; vt=t; vtid=tid; gl_Position = u_vp * vec4(p, 0.0, 1.0); }";
        const char* fs_src = "#version 330 core\nout vec4 fc; in vec4 vc; in vec2 vt; in float vtid; uniform sampler2D u_tex[16]; void main() { if (vtid > -0.5) { int tid=int(round(vtid)); fc = vc * texture(u_tex[tid], vt).r; } else { fc = vc; } }";
        GLuint vs = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vs, 1, &vs_src, 0); glCompileShader(vs);
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fs, 1, &fs_src, 0); glCompileShader(fs);
        m_prog = glCreateProgram(); glAttachShader(m_prog, vs); glAttachShader(m_prog, fs); glLinkProgram(m_prog);
        glDeleteShader(vs); glDeleteShader(fs);
        glGenVertexArrays(1, &m_vao); glBindVertexArray(m_vao);
        glGenBuffers(1, &m_vbo); glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTS * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, p));
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, c));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, t));
        glEnableVertexAttribArray(3); glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tid));
        State init; init.transform = glm::mat4(1.0f); init.color = {1,1,1,1}; stateStack.push(init);
    }
    ~Renderer() { glDeleteProgram(m_prog); glDeleteBuffers(1, &m_vbo); glDeleteVertexArrays(1, &m_vao); }
    void beginFrame(int w, int h) {
        m_verts.clear(); m_tex_slots.clear();
        glUseProgram(m_prog);
        glm::mat4 proj = glm::ortho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);
        glm::mat4 view = stateStack.top().transform;
        glUniformMatrix4fv(glGetUniformLocation(m_prog, "u_vp"), 1, GL_FALSE, glm::value_ptr(proj * view));
        int samplers[MAX_TEX]; for(int i=0; i<MAX_TEX; ++i) samplers[i]=i;
        glUniform1iv(glGetUniformLocation(m_prog, "u_tex"), MAX_TEX, samplers);
    }
    void flush() {
        if (m_verts.empty()) return;
        for (uint32_t i=0; i<m_tex_slots.size(); ++i) { glActiveTexture(GL_TEXTURE0+i); glBindTexture(GL_TEXTURE_2D, m_tex_slots[i]); }
        glBindVertexArray(m_vao); glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_verts.size() * sizeof(Vertex), m_verts.data());
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLES, 0, m_verts.size());
        glDisable(GL_BLEND);
    }
    float getTexSlot(GLuint id) {
        for (size_t i=0; i<m_tex_slots.size(); ++i) if (m_tex_slots[i]==id) return (float)i;
        if (m_tex_slots.size() >= MAX_TEX) flush();
        m_tex_slots.push_back(id); return (float)m_tex_slots.size() - 1.0f;
    }
    void addQuad(const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& p3, const glm::vec4& p4, const glm::vec4& c, float tid, const glm::vec4& tc) {
        if (m_verts.size() + 6 > MAX_VERTS) flush();
        m_verts.push_back({{p1.x,p1.y}, c, {tc.x,tc.y}, tid}); m_verts.push_back({{p2.x,p2.y}, c, {tc.z,tc.y}, tid}); m_verts.push_back({{p3.x,p3.y}, c, {tc.z,tc.w}, tid});
        m_verts.push_back({{p1.x,p1.y}, c, {tc.x,tc.y}, tid}); m_verts.push_back({{p3.x,p3.y}, c, {tc.z,tc.w}, tid}); m_verts.push_back({{p4.x,p4.y}, c, {tc.x,tc.w}, tid});
    }
};

class Canvas {
public:
    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Renderer> m_renderer;
    sp_key_callback_t key_cb=nullptr; sp_mouse_button_callback_t mouse_btn_cb=nullptr; sp_cursor_pos_callback_t cursor_pos_cb=nullptr;
    Canvas(const sp_window_config_t& config) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // This hint allows window creation in headless CI
        m_window = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
        if (!m_window) { throw std::runtime_error("glfwCreateWindow failed"); }
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
    if (!config) return nullptr;
    try { return reinterpret_cast<sp_canvas_t*>(new Canvas(*config)); }
    catch (const std::exception& e) { std::cerr << "Canvas Creation Failed: " << e.what() << std::endl; return nullptr; }
}
void sp_destroy_canvas(sp_canvas_t* c) { delete as_canvas(c); }
bool sp_canvas_should_close(sp_canvas_t* c) { return c ? glfwWindowShouldClose(as_canvas(c)->m_window) : true; }
void sp_begin_frame(sp_canvas_t* c) { if (!c) return; glfwPollEvents(); int w,h; glfwGetFramebufferSize(as_canvas(c)->m_window,&w,&h); glViewport(0,0,w,h); as_canvas(c)->m_renderer->beginFrame(w,h); }
void sp_end_frame(sp_canvas_t* c) { if (!c) return; as_canvas(c)->m_renderer->flush(); glfwSwapBuffers(as_canvas(c)->m_window); }
void sp_clear(sp_canvas_t* c, sp_color_rgba_t color) { if (!c) return; glClearColor(color.r,color.g,color.b,color.a); glClear(GL_COLOR_BUFFER_BIT); }
sp_vec2_t sp_get_canvas_size(sp_canvas_t* c) { if (!c) return {0,0}; int w,h; glfwGetWindowSize(as_canvas(c)->m_window,&w,&h); return {(float)w,(float)h}; }

void sp_save_state(sp_canvas_t* c) { if (!c) return; as_canvas(c)->m_renderer->stateStack.push(as_canvas(c)->m_renderer->stateStack.top()); }
void sp_restore_state(sp_canvas_t* c) { if (!c) return; if (as_canvas(c)->m_renderer->stateStack.size() > 1) as_canvas(c)->m_renderer->stateStack.pop(); }
void sp_reset_transform(sp_canvas_t* c) { if (!c) return; as_canvas(c)->m_renderer->stateStack.top().transform = glm::mat4(1.0f); }
void sp_translate(sp_canvas_t* c, float x, float y) { if (!c) return; auto& s = as_canvas(c)->m_renderer->stateStack; s.top().transform=glm::translate(s.top().transform,glm::vec3(x,y,0)); }
void sp_rotate(sp_canvas_t* c, float angle_radians) { if (!c) return; auto& s = as_canvas(c)->m_renderer->stateStack; s.top().transform=glm::rotate(s.top().transform,angle_radians,glm::vec3(0,0,1)); }
void sp_scale(sp_canvas_t* c, float x, float y) { if (!c) return; auto& s = as_canvas(c)->m_renderer->stateStack; s.top().transform=glm::scale(s.top().transform,glm::vec3(x,y,1)); }

sp_pen_t* sp_create_pen(sp_canvas_t* c, const sp_pen_config_t* config) { if (!c || !config) return nullptr; return reinterpret_cast<sp_pen_t*>(new Pen{*config}); }
void sp_destroy_pen(sp_pen_t* p) { delete as_pen(p); }
void sp_set_pen(sp_canvas_t* c, sp_pen_t* p) { if (!c || !p) return; as_canvas(c)->m_renderer->stateStack.top().pen = p; }
void sp_set_color(sp_canvas_t* c, sp_color_rgba_t color) { if (!c) return; as_canvas(c)->m_renderer->stateStack.top().color = color; }

sp_path_t* sp_create_path(sp_canvas_t* c) { if (!c) return nullptr; return reinterpret_cast<sp_path_t*>(new Path()); }
void sp_destroy_path(sp_path_t* p) { delete as_path(p); }
void sp_path_move_to(sp_path_t* p, float x, float y) { if (!p) return; as_path(p)->points.clear(); as_path(p)->points.push_back({x,y}); }
void sp_path_line_to(sp_path_t* p, float x, float y) { if (!p) return; as_path(p)->points.push_back({x,y}); }
void sp_path_arc_to(sp_path_t* p, float x1, float y1, float x2, float y2, float r) {}
void sp_path_cubic_bezier_to(sp_path_t* p, float c1x, float c1y, float c2x, float c2y, float x, float y) {}
void sp_path_close(sp_path_t* p) { if (!p || as_path(p)->points.size() < 2) return; as_path(p)->points.push_back(as_path(p)->points.front()); }
void sp_stroke_path(sp_canvas_t* c, sp_path_t* p) {
    if (!c || !p) return;
    auto path = as_path(p); auto renderer = as_canvas(c)->m_renderer.get();
    auto pen = as_pen(renderer->stateStack.top().pen);
    if (path->points.size() < 2 || !pen) return;
    auto& color_s = renderer->stateStack.top().color;
    glm::vec4 color = {color_s.r, color_s.g, color_s.b, color_s.a};
    for (size_t i=0; i<path->points.size()-1; ++i) {
        glm::vec4 p1={path->points[i].x, path->points[i].y,0,1}, p2={path->points[i+1].x, path->points[i+1].y,0,1};
        glm::vec2 dir = p2-p1; if(glm::length(dir)>0.0f) dir=glm::normalize(dir);
        glm::vec2 n(-dir.y, dir.x); n *= pen->config.line_width/2.0f;
        renderer->addQuad(p1-glm::vec4(n,0,0), p2-glm::vec4(n,0,0), p2+glm::vec4(n,0,0), p1+glm::vec4(n,0,0), color, -1.0f, {});
    }
}
void sp_fill_path(sp_path_t* p, sp_path_t* path) {}

void sp_draw_line(sp_canvas_t* c, float x1, float y1, float x2, float y2) { if (!c) return; auto renderer=as_canvas(c)->m_renderer.get(); auto& cs=renderer->stateStack.top().color; glm::vec4 color={cs.r,cs.g,cs.b,cs.a}; glm::vec4 p1={x1,y1,0,1}, p2={x2,y2,0,1}; glm::vec2 dir=p2-p1; if(glm::length(dir)>0.0f) dir=glm::normalize(dir); glm::vec2 n(-dir.y,dir.x); renderer->addQuad(p1-glm::vec4(n,0,0), p2-glm::vec4(n,0,0), p2+glm::vec4(n,0,0), p1+glm::vec4(n,0,0), color, -1.0f, {}); }
void sp_fill_rect(sp_canvas_t* c, float x, float y, float w, float h) { if (!c) return; auto renderer=as_canvas(c)->m_renderer.get(); auto& cs=renderer->stateStack.top().color; glm::vec4 color={cs.r,cs.g,cs.b,cs.a}; renderer->addQuad({x,y,0,1},{x+w,y,0,1},{x+w,y+h,0,1},{x,y+h,0,1},color,-1.0f,{0,0,1,1}); }
void sp_draw_rect(sp_canvas_t* c, float x, float y, float w, float h) {}
void sp_draw_circle(sp_canvas_t* c, float cx, float cy, float r) {}
void sp_fill_circle(sp_canvas_t* c, float cx, float cy, float r) {}
void sp_draw_ellipse(sp_canvas_t* c, float cx, float cy, float rx, float ry) {}

sp_font_t* sp_load_font(sp_canvas_t* c, const char* path) {
    if (!c || !path) return nullptr;
    std::ifstream file(path, std::ios::binary | std::ios::ate); if (!file) return nullptr;
    auto font = new Font(); font->ttf_buffer.resize(file.tellg()); file.seekg(0, std::ios::beg);
    if (!file.read((char*)font->ttf_buffer.data(), font->ttf_buffer.size())) { delete font; return nullptr; }
    unsigned char temp_bitmap[512*512];
    stbtt_BakeFontBitmap(font->ttf_buffer.data(), 0, 32.0, temp_bitmap, 512, 512, 32, 96, font->cdata);
    glGenTextures(1, &font->font_texture); glBindTexture(GL_TEXTURE_2D, font->font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return reinterpret_cast<sp_font_t*>(font);
}
void sp_destroy_font(sp_font_t* f) { if(f) { glDeleteTextures(1, &as_font(f)->font_texture); delete as_font(f); } }
void sp_set_font(sp_canvas_t* c, sp_font_t* f, float size) { if (!c || !f) return; auto& state=as_canvas(c)->m_renderer->stateStack.top(); state.font=f; state.font_size=size; }
void sp_draw_text(sp_canvas_t* c, const char* text, float x, float y) {
    if (!c || !text) return;
    auto& state = as_canvas(c)->m_renderer->stateStack.top(); auto font = as_font(state.font); if (!font) return;
    auto color_s = state.color; glm::vec4 color = {color_s.r, color_s.g, color_s.b, color_s.a};
    float tid = as_canvas(c)->m_renderer->getTexSlot(font->font_texture);
    for (const char* ptr=text; *ptr; ptr++) {
        if (*ptr >= 32 && *ptr < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->cdata, 512, 512, *ptr - 32, &x, &y, &q, 1);
            as_canvas(c)->m_renderer->addQuad({q.x0,q.y0,0,1},{q.x1,q.y0,0,1},{q.x1,q.y1,0,1},{q.x0,q.y1,0,1}, color, tid, {q.s0,q.t0,q.s1,q.t1});
        }
    }
}
sp_rect_t sp_measure_text(sp_canvas_t* c, const char* text) { return {0,0,0,0}; }

sp_image_t* sp_load_image(sp_canvas_t* c, const char* path) {
    if (!c || !path) return nullptr;
    auto image=new Image(); int channels; unsigned char* data = stbi_load(path, &image->w, &image->h, &channels, 4);
    if (!data) { delete image; return nullptr; }
    glGenTextures(1, &image->textureId); glBindTexture(GL_TEXTURE_2D, image->textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data); return reinterpret_cast<sp_image_t*>(image);
}
void sp_destroy_image(sp_image_t* i) { if(i) { glDeleteTextures(1, &as_image(i)->textureId); delete as_image(i); } }
void sp_draw_image(sp_canvas_t* c, sp_image_t* i, float x, float y) {
    if (!c || !i) return; auto image = as_image(i);
    float tid = as_canvas(c)->m_renderer->getTexSlot(image->textureId);
    as_canvas(c)->m_renderer->addQuad({x,y,0,1},{x+image->w,y,0,1},{x+image->w,y+image->h,0,1},{x,y+image->h,0,1},{1,1,1,1},tid,{0,0,1,1});
}
void sp_draw_image_rect(sp_canvas_t* c, sp_image_t* i, sp_rect_t src, sp_rect_t dest) {
    if (!c || !i) return; auto image = as_image(i);
    float tid = as_canvas(c)->m_renderer->getTexSlot(image->textureId);
    glm::vec4 tc={src.x/image->w, src.y/image->h, (src.x+src.w)/image->w, (src.y+src.h)/image->h};
    as_canvas(c)->m_renderer->addQuad({dest.x,dest.y,0,1},{dest.x+dest.w,dest.y,0,1},{dest.x+dest.w,dest.y+dest.h,0,1},{dest.x,dest.y+dest.h,0,1},{1,1,1,1},tid,tc);
}

static void internal_key_cb(GLFWwindow* w, int k, int s, int a, int m) { auto* c=static_cast<Canvas*>(glfwGetWindowUserPointer(w)); if(c&&c->key_cb) c->key_cb(reinterpret_cast<sp_canvas_t*>(c),k,s,a,m); }
static void internal_mouse_btn_cb(GLFWwindow* w, int b, int a, int m) { auto* c=static_cast<Canvas*>(glfwGetWindowUserPointer(w)); if(c&&c->mouse_btn_cb) c->mouse_btn_cb(reinterpret_cast<sp_canvas_t*>(c),b,a,m); }
static void internal_cursor_pos_cb(GLFWwindow* w, double x, double y) { auto* c=static_cast<Canvas*>(glfwGetWindowUserPointer(w)); if(c&&c->cursor_pos_cb) c->cursor_pos_cb(reinterpret_cast<sp_canvas_t*>(c),x,y); }

void sp_set_key_callback(sp_canvas_t* c, sp_key_callback_t cb) { if(!c)return; auto* cv=as_canvas(c); cv->key_cb=cb; glfwSetKeyCallback(cv->m_window, cb?internal_key_cb:nullptr); }
void sp_set_mouse_button_callback(sp_canvas_t* c, sp_mouse_button_callback_t cb) { if(!c)return; auto* cv=as_canvas(c); cv->mouse_btn_cb=cb; glfwSetMouseButtonCallback(cv->m_window, cb?internal_mouse_btn_cb:nullptr); }
void sp_set_cursor_pos_callback(sp_canvas_t* c, sp_cursor_pos_callback_t cb) { if(!c)return; auto* cv=as_canvas(c); cv->cursor_pos_cb=cb; glfwSetCursorPosCallback(cv->m_window, cb?internal_cursor_pos_cb:nullptr); }
