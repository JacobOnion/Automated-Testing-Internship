#include <GLFW/glfw3.h>
#include <dlfcn.h>
#include <stdio.h>
#include <vector>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void glfwSwapBuffers(GLFWwindow* window) {
    static void (*real_glswapbuffers)(GLFWwindow*) = NULL;
    if(!real_glswapbuffers) {
        real_glswapbuffers = (void (*)(GLFWwindow*)) dlsym(RTLD_NEXT, "glfwSwapBuffers");
    }
    
    printf("glfwSwapBuffers called\n");
    real_glswapbuffers(window);
    printf("glfwSwapBuffers completed\n");
    fflush(stdout);

    int nwidth, nheight;
    glfwGetFramebufferSize(window, &nwidth, &nheight);
    std::vector<unsigned char> pixels(static_cast<std::size_t>(nwidth) * static_cast<std::size_t>(nheight) * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, nwidth, nheight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    /*if (!stbi_write_png("/autograder/source/screenshots/test_image.png", nwidth, nheight, 3, pixels.data(), nwidth * 3)) {
        fprintf(stderr, "Failed to write image\n");
    } else {
        printf("Image written successfully\n");
    }

    if (!stbi_write_png("/autograder/source/1kb_image.png", nwidth, nheight, 3, pixels.data(), nwidth * 3)) {
        fprintf(stderr, "Failed to write image\n");
    } else {
        printf("Image written successfully\n");
    }*/
}