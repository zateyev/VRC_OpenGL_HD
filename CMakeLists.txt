# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (OpenGL_VRC)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

# search for pkg-config
include (FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
    message (FATAL_ERROR "pkg-config not found")
endif ()

# check for libpng
pkg_check_modules (LIBPNG libpng16 REQUIRED)
if (NOT LIBPNG_FOUND)
    message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
else ()
    include_directories (${LIBPNG_INCLUDE_DIRS})
    link_directories (${LIBPNG_LIBRARY_DIRS})
    link_libraries (${LIBPNG_LIBRARIES})
endif ()

SET(glui_DIR /usr/lib/lib/cmake/glui/)
find_package(glui CONFIG REQUIRED)

find_package(OpenGL REQUIRED)

set (CMAKE_CXX_STANDARD 11)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

add_subdirectory (include)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)

include_directories(
	include/GL/
  include/GL/glm/
  include/glew-1.13.0/include/
  lib/
  .
)

find_path(
	/usr/include
	/usr/local/include
	/opt/local/include
	${CMAKE_SOURCE_DIR}/includes
)

find_library(
	/usr/lib64
  /usr/lib
	/usr/local/lib
	/opt/local/lib
	${CMAKE_SOURCE_DIR}/lib
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
  ${GLUT_LIBRARY}
  GLEW_1130
)

add_executable(main
  main.cpp
  common/shader.cpp
	common/shader.hpp
	common/texture.cpp
	common/texture.hpp
)
target_link_libraries(main PUBLIC glui::glui ${ALL_LIBS})
