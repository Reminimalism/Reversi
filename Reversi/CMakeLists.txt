if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))

    add_compile_options(-O2)

endif()

set(APP_ICON_RESOURCE_WINDOWS "")
if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "Reversi.rc")
endif()

add_executable(Reversi
    AI.cpp
    Board.cpp
    BufferGeneration.cpp
    Logic.cpp
    Math.cpp
    Model.cpp
    MouseEventManager.cpp
    Reversi.cpp
    Renderer.cpp
    ShaderProgram.cpp
    Window.cpp
    ${APP_ICON_RESOURCE_WINDOWS}
)
target_link_libraries(Reversi glfw)
target_link_libraries(Reversi glad)
