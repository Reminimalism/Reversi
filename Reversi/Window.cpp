#include "Window.h"

#include <iostream>
#include <stdexcept>

namespace Reversi
{
    int Window::ObjectsCount = 0;
    std::map<GLFWwindow*, Window*> Window::ObjectsMap;

    Window::Window(const std::string& title) : _MouseEventManager(true, 100), ResizeCallback(nullptr)
    {
        if (ObjectsCount == 0)
        {
            if (!glfwInit())
                throw std::logic_error("GLFW initialization failed.");
            glfwSetErrorCallback([](int error, const char * description) {
                std::cerr << "Error " << error << ": " << description << '\n';
            });
        }
        ObjectsCount++;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_SAMPLES, 8);
        _GLFWWindow = glfwCreateWindow(640, 480, title.c_str(), NULL, NULL);
        //_GLFWWindow = glfwCreateWindow(640, 480, title.c_str(), glfwGetPrimaryMonitor(), NULL); // fullscreen
        if (!_GLFWWindow)
        {
            ObjectsCount--;
            throw std::logic_error("Window or OpenGL context creation failed.");
        }
        //GLFWmonitor * monitor = glfwGetPrimaryMonitor();
        //const GLFWvidmode * mode = glfwGetVideoMode(monitor);
        //glfwSetWindowMonitor(_GLFWWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate); // fullscreen
        glfwMakeContextCurrent(_GLFWWindow);
        gladLoadGL();
        glfwSwapInterval(1);
        ObjectsMap[_GLFWWindow] = this;
        glfwSetMouseButtonCallback(_GLFWWindow, [](GLFWwindow * _GLFWWindow, int button, int action, int mods) {
            auto window = Window::ObjectsMap[_GLFWWindow];
            double x, y;
            glfwGetCursorPos(_GLFWWindow, &x, &y);
            if (action == GLFW_PRESS)
                window->_MouseEventManager.MouseDown(x, y);
            else if (action == GLFW_RELEASE)
                window->_MouseEventManager.MouseUp(x, y);
        });
        glfwSetFramebufferSizeCallback(_GLFWWindow, [](GLFWwindow * _GLFWWindow, int width, int height) {
            glViewport(0, 0, width, height);
            auto window = Window::ObjectsMap[_GLFWWindow];
            if (window->ResizeCallback != nullptr)
                window->ResizeCallback(width, height);
        });
        glEnable(GL_MULTISAMPLE);
    }

    void Window::SetResizeCallback(std::function<void(int width, int height)> callback)
    {
        ResizeCallback = callback;
    }

    void Window::GetSize(int& width, int& height)
    {
        glfwGetFramebufferSize(_GLFWWindow, &width, &height);
    }

    MouseEventManager * Window::GetMouseEventManager()
    {
        return &_MouseEventManager;
    }

    void Window::MakeCurrent()
    {
        glfwMakeContextCurrent(_GLFWWindow);
    }

    void Window::Update()
    {
        double x, y;
        glfwGetCursorPos(_GLFWWindow, &x, &y);
        _MouseEventManager.UpdateMousePosition(x, y);
        glfwPollEvents();
    }

    void Window::SwapBuffers()
    {
        glfwSwapBuffers(_GLFWWindow);
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(_GLFWWindow);
    }

    void Window::Close()
    {
        return glfwSetWindowShouldClose(_GLFWWindow, GLFW_TRUE);
    }

    Window::~Window()
    {
        glfwDestroyWindow(_GLFWWindow);
        ObjectsMap.erase(_GLFWWindow);

        ObjectsCount--;
        if (ObjectsCount == 0)
            glfwTerminate();
    }
}
