include_directories(./)
include_directories(../inc)
include_directories(/usr/include)
include_directories(opengl/view/linux/X11)
include_directories(opengl/view)
include_directories(opengl/utils)
include_directories(opengl/view/egl)

find_library( OPENGLES3_LIBRARY GLESv2 "OpenGL ES v3.0 library") 
find_library( EGL_LIBRARY EGL "EGL 1.4 library" )

set (app_src 
    App.cpp
)

add_executable(opengl_test ${app_src}) 

target_compile_options(opengl_test
                        PUBLIC
                        PRIVATE
                            -g
                            -O0
                            -std=c++14
                            -fexceptions
)

target_link_libraries(opengl_test 
    ${OPENGLES3_LIBRARY}
    ${EGL_LIBRARY} 
    GLEW
    GL
    common
    gbm)