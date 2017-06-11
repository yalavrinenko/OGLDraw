cmake_minimum_required(VERSION 3.0)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set (CMAKE_CXX_STANDARD 11)
set (PLOT_SRC GLPlot.cpp)
set (LOG_SRC Logger.cpp)

add_library(3DDraw SHARED ${PLOT_SRC} ${LOG_SRC})
target_link_libraries(3DDraw ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} -lpthread)

add_executable(3DTest main.cpp)
target_link_libraries(3DTest 3DDraw -lX11)
