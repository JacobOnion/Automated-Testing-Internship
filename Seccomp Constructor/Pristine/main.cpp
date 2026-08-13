#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/socket.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace {
std::string read_text_file(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open file: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string resolve_shader_path(const std::string& filename) {
    const std::vector<std::string> candidates = {
        filename,
        "./assets/" + filename,
        "./OpenGL/assets/" + filename,
        "/LocalCont/source/OpenGL/assets/" + filename,
        "/LocalCont/source/OpenGL/" + filename,
    };

    for (const auto& candidate : candidates) {
        std::ifstream probe(candidate);
        if (probe.good()) {
            return candidate;
        }
    }

    return filename;
}

GLuint compile_shader(GLenum type, const std::string& filename) {
    const std::string path = resolve_shader_path(filename);
    const std::string source = read_text_file(path);
    const GLchar* source_ptr = source.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source_ptr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar info_log[1024];
        glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log);
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Shader compilation failed for ") + path + ": " + info_log);
    }

    return shader;
}

GLuint build_program(const std::string& vertex_shader, const std::string& fragment_shader) {
    GLuint vertex_shader_id = compile_shader(GL_VERTEX_SHADER, vertex_shader);
    GLuint fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader_id);
    glAttachShader(program, fragment_shader_id);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar info_log[1024];
        glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
        glDeleteProgram(program);
        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);
        throw std::runtime_error(std::string("Shader program linking failed: ") + info_log);
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    return program;
}
}  // namespace

int main() {
    
    fflush(stdout);
    glfwSetErrorCallback([](int code, const char* desc){
        fprintf(stderr, "GLFW error %d: %s\n", code, desc);
    });
        
    std::cout << "Initializing GLFW..." << std::endl;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "Creating GLFW window..." << std::endl;
    GLFWwindow* window = glfwCreateWindow(400, 300, "Minimal GLFW Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return 2;
    }

    std::cout << "GLFW window created successfully." << std::endl;
    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 3;
    }


    // Testing ptrace filter
    //FILE *testfile = fopen("/autograder/source/tests/bad.txt", "r");
    //fclose(testfile);
    
    //int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int nwidth = 400;
    int nheight = 300;
    glfwGetFramebufferSize(window, &nwidth, &nheight);
    glViewport(0, 0, nwidth, nheight);

    std::cout << "Window is size " << nwidth << "x" << nheight << std::endl;

    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vertexVBO = 0;
    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    const float vertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.0f,  0.5f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint colorVBO = 0;
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    const float colors[] = {
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    GLuint program = 0;
    try {
        program = build_program("base.vert", "base.frag");
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vertexVBO);
        glDeleteBuffers(1, &colorVBO);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 4;
    }

    glUseProgram(program);
    GLint uniform_loc = glGetUniformLocation(program, "uBaseColor");
    if (uniform_loc != -1) {
        glUniform3f(uniform_loc, 1.0f, 1.0f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);

    glDeleteProgram(program);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexVBO);
    glDeleteBuffers(1, &colorVBO);

    printf("Cleaning up and exiting...\n");

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
