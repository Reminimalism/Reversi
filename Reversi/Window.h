#pragma once

#include "Reversi.dec.h"
#include "MouseEventManager.h"

#include "../glad/include/glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "../glfw/include/GLFW/glfw3.h"

#include <functional>
#include <map>
#include <string>

namespace Reversi
{
    class Window final
    {
    public:
        Window(const std::string& title = "Reversi");
        ~Window();

        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = delete;

        void SetResizeCallback(std::function<void(int width, int height)>);
        void GetSize(int& width, int& height);
        MouseEventManager * GetMouseEventManager();
        void MakeCurrent();
        void Update();
        void SwapBuffers();
        bool ShouldClose();
        void Close();
    private:
        static int ObjectsCount;
        static std::map<GLFWwindow*, Window*> ObjectsMap;
        GLFWwindow * _GLFWWindow;
        MouseEventManager _MouseEventManager;
        std::function<void(int width, int height)> ResizeCallback;
    };
}
