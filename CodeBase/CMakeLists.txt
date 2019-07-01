cmake_minimum_required(VERSION 3.5)

if(EMSCRIPTEN)
    set (CMAKE_C_COMPILER   $ENV{EMSCRIPTEN}/emcc CACHE PATH "C compiler")
    set (CMAKE_CXX_COMPILER $ENV{EMSCRIPTEN}/em++ CACHE PATH "C++ compiler")
    set (CMAKE_AR           $ENV{EMSCRIPTEN}/emar CACHE PATH "archive")
    set (CMAKE_RANLIB       $ENV{EMSCRIPTEN}/emranlib CACHE PATH "ranlib")
    set (CMAKE_LINKER       $ENV{EMSCRIPTEN}/emlink.py CACHE PATH "linker")
endif(EMSCRIPTEN)

project(simavr_js)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

include_directories(../simavr_atmega_1280/simavr/sim/)
set(SOURCE_FILES main.c simulator.h parts/wdg0151.c parts/wdg0151.h parts/hd44780.c parts/hd44780.h)

if(EMSCRIPTEN)
    add_executable(simavr.js ${SOURCE_FILES})
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
else(EMSCRIPTEN)
    add_executable(simavr_js ${SOURCE_FILES})
endif(EMSCRIPTEN)

if(EMSCRIPTEN)
    add_library(libsimavr SHARED IMPORTED)
    set_target_properties(libsimavr PROPERTIES IMPORTED_LOCATION /home/sarm/project/Port_Of_SimAvr/simavr_atmega_1280/simavr/obj-asmjs-unknown-emscripten/libsimavr.a)
    target_link_libraries(simavr.js libsimavr)
else(EMSCRIPTEN)
    target_link_libraries(simavr_js /home/sarm/project/Port_Of_SimAvr/simavr_atmega_1280/simavr/obj-x86_64-linux-gnu/libsimavr.so.1)
endif(EMSCRIPTEN)
