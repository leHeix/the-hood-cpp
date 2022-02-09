cmake_minimum_required (VERSION 3.22.1)
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0042 NEW)

project ("the-hood" LANGUAGES C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(
	$<$<BOOL:MSVC>:_CRT_SECURE_NO_WARNINGS>
	$<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
	$<$<PLATFORM_ID:Linux>:LINUX>
	SAMPGDK_AMALGAMATION
	HAVE_STDINT_H
)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
find_package(unofficial-libuv CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)
find_package(effolkronium_random CONFIG REQUIRED)

add_subdirectory(lib)

file(GLOB_RECURSE HOOD_SRC 
	"src/*.cpp"
	"src/*.hpp"
)
add_library(the-hood SHARED
	 "src/exports.def"
	 ${HOOD_SRC}
)

target_compile_features(the-hood PUBLIC cxx_std_20 c_std_11)
target_link_libraries(the-hood PUBLIC RakNet tomlplusplus::tomlplusplus effolkronium_random unofficial::libuv::libuv glm::glm fmt::fmt SQLite3 SAMPSDK SAMPGDK Botan)
set_target_properties(the-hood
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/server/plugins"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/server/plugins"
		UNITY_BUILD ON
		UNITY_BUILD_BATCH_SIZE 15
		PREFIX ""
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(the-hood PUBLIC -Wno-invalid-source-encoding)
	endif()

	target_compile_options(the-hood PUBLIC -Wno-multichar)
endif()


if(NOT MSVC)
	target_precompile_headers(the-hood PUBLIC "src/pch.h")
endif()

target_include_directories(the-hood PUBLIC 
	"./lib" 
	"./lib/robin-hood-hashing/src/include"
	"./lib/sqlite3"
	"./lib/botan/include"
)