include_directories(../../inc)
include_directories(effect)
include_directories(view/linux/X11)
include_directories(view/egl)
include_directories(view)
include_directories(utils)
include_directories(/usr/include)

#查找指定的库,
find_package(X11) 
find_library(M_LIB m)

# find_library( OPENGLES3_LIBRARY GLESv2 "OpenGL ES v3.0 library") 
find_library( EGL_LIBRARY EGL "EGL 1.4 library" )

#设置变量
set (x11_src 
    view/linux/X11/X11.cpp
)

set (egl_src 
    view/egl/eglCommon.cpp
)

set (view_src 
    view/view.cpp
)

set (utils_src 
    utils/GLUtil.cpp
)


#库添加源文件
add_library(common STATIC 
    ${utils_src} 
    ${egl_src} 
    ${x11_src} 
    ${view_src}
)

target_compile_options(common
                        PUBLIC
                        PRIVATE
                            -g
                            -O0
                            -std=c++14
                            -fexceptions
)

#链接库
target_link_libraries(common 
    # ${OPENGLES3_LIBRARY} 
    ${EGL_LIBRARY} 
    ${X11_LIBRARIES} 
    ${M_LIB}
    GLEW
    GL
)