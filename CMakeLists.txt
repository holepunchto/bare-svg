cmake_minimum_required(VERSION 3.31)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)
find_package(cmake-cargo REQUIRED PATHS node_modules/cmake-cargo)
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(bare_svg C)

fetch_package(
  "github:linebender/resvg@0.46.0"
  SOURCE_DIR resvg
  PATCHES
    patches/01-resvg-staticlib.patch
    patches/02-resvg-catch-panics.patch
)

add_crate(MANIFEST ${resvg}/crates/c-api/Cargo.toml)

add_bare_module(bare_svg)

target_sources(
  ${bare_svg}
  PRIVATE
    binding.c
)

target_include_directories(
  ${bare_svg}
  PRIVATE
    ${resvg}/crates/c-api
)

target_link_libraries(${bare_svg} PRIVATE resvg)

if(APPLE)
  target_link_libraries(${bare_svg} PRIVATE
    "-framework CoreFoundation"
    "-framework CoreText"
    "-framework CoreGraphics"
  )
elseif(ANDROID)
  target_link_libraries(${bare_svg} PRIVATE m dl log)
elseif(UNIX)
  target_link_libraries(${bare_svg} PRIVATE m pthread dl)
elseif(WIN32)
  target_link_libraries(${bare_svg} PRIVATE gdi32 user32)
endif()
