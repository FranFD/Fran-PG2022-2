#include "Window.hpp"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include "World.hpp"

static GLFWwindow* window;

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Window::Create() {
	glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    // versão da opengl
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // cria a janela
    window = glfwCreateWindow(800, 640, "Gato vs Ratos", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    // carrega a opengl
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);
}

void Window::Destroy() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::ShouldClose() {
    return glfwWindowShouldClose(window);
}


void Window::PreRender() {
    float ratio;
    int width, height;

    // calcula o viewport
    glfwGetFramebufferSize(window, &width, &height);
    float cWidth = width * WORLD_RATIO;

    float x = width / 2.0f - cWidth / 2.0f;

    glViewport(x, 0, cWidth, height);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::PostRender() {
    // troca de buffers e processa eventos
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool Window::IsKeyDown(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Window::IsMouseDown(int button) {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void Window::GetMousePos(int& x, int& y) {
    double _x, _y;
    glfwGetCursorPos(window, &_x, &_y);

    x = (int)_x;
    y = (int)_y;
}

void Window::GetDimensions(int& x, int& y) {
    glfwGetFramebufferSize(window, &x, &y);
}