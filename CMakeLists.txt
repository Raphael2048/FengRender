cmake_minimum_required(VERSION 3.13)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_compile_definitions(_ENABLE_EXTENDED_ALIGNED_STORAGE )


project(Feng)

##### OUTPUT DIRECTORIES #####
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB HEADERS CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
file(GLOB SOURCES CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB UTIL_HEADERS CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.hpp")
file(GLOB UTIL_SOURCES CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp")
file(GLOB DX12_HEADERS CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/dx12/*.hpp")
file(GLOB DX12_SOURCES CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/dx12/*.cpp")
file(GLOB SCENE_HEADERS CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/scene/*.hpp")
file(GLOB SCENE_SOURCES CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/scene/*.cpp")
file(GLOB RENDER_HEADERS CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/render/*.hpp")
file(GLOB RENDER_SOURCES CMAKE_CONFIGURE_DEPENDS] "${CMAKE_CURRENT_SOURCE_DIR}/src/render/*.cpp")

source_group("Feng" FILES ${HEADERS} ${SOURCES})
source_group("Utility" FILES ${UTIL_SOURCES} ${UTIL_HEADERS})
source_group("DX12" FILES ${DX12_HEADERS} ${DX12_SOURCES})
source_group("Scene" FILES ${SCENE_HEADERS} ${SCENE_SOURCES})
source_group("Render" FILES ${RENDER_HEADERS} ${RENDER_SOURCES})

set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/DirectXTK12 ${CMAKE_BINARY_DIR}/DirectXTK12)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_ERROR_MAX ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)


add_subdirectory(${CMAKE_SOURCE_DIR}/deps/assimp ${CMAKE_BINARY_DIR}/assimp)
target_compile_options(assimp PRIVATE /WX-)

set_target_properties(DirectXTK12  PROPERTIES FOLDER "ThirdParty")

set_target_properties(assimp assimp_cmd zlibstatic UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "ThirdParty/assimp")

add_library(FengRenderer STATIC ${HEADERS} ${SOURCES} ${UTIL_HEADERS} ${UTIL_SOURCES} ${DX12_HEADERS} ${DX12_SOURCES} ${SCENE_HEADERS} ${SCENE_SOURCES} ${RENDER_HEADERS} ${RENDER_SOURCES})
set_target_properties(FengRenderer PROPERTIES CXX_STANDARD 20)
set_target_properties(FengRenderer PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(FengRenderer PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
target_compile_options(FengRenderer PRIVATE /W4 /permissive- /MP /Gm-)
endif()

target_include_directories(FengRenderer PUBLIC ${CMAKE_SOURCE_DIR}/src/)
target_include_directories(FengRenderer PUBLIC ${CMAKE_SOURCE_DIR}/deps/DirectXTK12/inc/)
target_include_directories(FengRenderer PUBLIC ${CMAKE_SOURCE_DIR}/deps/DirectXTK12/Src)
target_include_directories(FengRenderer PUBLIC ${CMAKE_SOURCE_DIR}/deps/assimp/include/)

target_link_libraries(FengRenderer DirectXTK12 assimp)
target_link_libraries(FengRenderer d3d12.lib dxgi.lib dxguid.lib d3dcompiler.lib)

set_target_properties(FengRenderer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/../")

add_subdirectory(tests ${CMAKE_BINARY_DIR}/tests)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Demo)