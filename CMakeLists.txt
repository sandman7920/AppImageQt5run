cmake_minimum_required(VERSION 2.8)
project(AppImageQt5run)

set(APP_NAME "appimage.qt5run")
set(CMAKE_SKIP_BUILD_RPATH  TRUE)

find_program (PATCH_ELF "patchelf")
if (NOT PATCH_ELF)
    message(FATAL_ERROR "patchelf not found")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fPIC -DQT_CORE_LIB")
if(LINK_RUNTIME)
    message("LINK_RUNTIME ON")
    set(APP_NAME "appimage_wrt.qt5run")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

set(QT_ROOT "${CMAKE_CURRENT_LIST_DIR}/Qt_5.1.1")

include_directories(SYSTEM ${QT_ROOT}/include)
include_directories(SYSTEM ${QT_ROOT}/include/QtCore)
link_directories(${QT_ROOT}/lib)

add_executable(${APP_NAME} "main.cpp")
target_link_libraries(${APP_NAME} -Wl,--rpath-link=${QT_ROOT}/lib -lQt5Core -ldl)

add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${PATCH_ELF} --remove-needed libQt5Core.so.5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${APP_NAME}"
)
