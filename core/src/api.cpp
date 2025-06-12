#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <spirographicals/spirographicals.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpphpp>

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
    GLuint font_texture = 0;
    stbtt_bakedchar cdata[96]; // Character data for ASCII 32-127
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

        const char* vs_src = R"glsl(#version 330 core
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
                gl_Position = u_ViewProjection * vec4(a_Pos, 0.0, 1.0);
            })glsl";

        const char* fs_src = R"glsl(#version 330 core
            out vec4 FragColor;
            in vec4 v_Color;
            in vec2 v_TexCoord;
            in float v_TexId;
            uniform sampler2D u_Textures[16];
            void main() {
                if (v_TexId > -0.5) {
                    int tid = int(round(v_TexId));
                    vec4 texColor = texture(u_Textures[tid], v_TexCoord);
                    FragColor = v_Color * texColor.r; // Use red channel for monochrome font atlas
                } else {
                    FragColor = v_Color;
                }
            })glsl";

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vs_src, nullptr);
        glCompileShader(vs);
        
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fs_src, nullptr);
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

    void addQuad(const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& p3, const glm::vec4& p4, const glm::vec4& color, float texId, const glm::vec4& texCoords) {
        if (m_vertices.size() + 6 > MAX_VERTICES) flush();
        m_vertices.push_back({ {p1.x, p1.y}, color, {texCoords.x, texCoords.y}, texId });
        m_vertices.push_back({ {p2.x, p2.y}, color, {texCoords.z, texCoords.y}, texId });
        m_vertices.push_back({ {p3.x, p3.y}, color, {texCoords.z, texCoords.w}, texId });
        m_vertices.push_back({ {p1.x, p1.y}, color, {texCoords.x, texCoords.y}, texId });
        m_vertices.push_back({ {p3.x, p3.y}, color, {texCoords.z, texCoords.w}, texId });
        m_vertices.push_back({ {p4.x, p4.y}, color, {texCoords.x, texCoords.w}, texId });
    }
};

class Canvas {
public:
    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Renderer> m_renderer;

    sp_key_callback_t key_callback = nullptr;
