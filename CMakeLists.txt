cmake_minimum_required (VERSION 3.8)
project ("borno")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_subdirectory(libs/raylib)
set(LIBRARIES ${LIBRARIES} raylib)

set(INCLUDES ${INCLUDES} libs/raygui/src)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	list(APPEND _CMAKE_CXX_FLAGS ${_CMAKE_CXX_FLAGS} "-Wall" "-pedantic")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# using GCC
	list(APPEND _CMAKE_CXX_FLAGS ${_CMAKE_CXX_FLAGS} "-fdiagnostics-color=always" "-Wall" "-pedantic" "-Wconversion")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/permissive-")
	add_compile_options("/Zc:preprocessor")
	# Remove the default warning level.
	# https://stackoverflow.com/questions/58708772/cmake-project-in-visual-studio-gives-flag-override-warnings-command-line-warnin
	string(REGEX REPLACE "/W[3|4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	 # The above preprocessor warns about certain things in windows headers.
	 # Turn those off: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c5105?view=vs-2019
	add_compile_options("/wd5105")

	set(_CMAKE_CXX_FLAGS ${_CMAKE_CXX_FLAGS} /W3)
endif()

if(APPLE)
    set(LIBRARIES ${LIBRARIES} pthread dl)
elseif(UNIX)
    set(LIBRARIES ${LIBRARIES} pthread GL dl)
elseif(WIN32)
endif()

file(GLOB SOURCES
    src/*
)

# TODO: Add tests and install targets if needed.
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${DEFINITIONS})
target_compile_options(${PROJECT_NAME} PRIVATE ${_CMAKE_CXX_FLAGS})
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "./" BUILD_RPATH "./")