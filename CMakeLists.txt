cmake_minimum_required(VERSION 3.31)

list(APPEND CMAKE_PREFIX_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/node_modules
  ${CMAKE_CURRENT_SOURCE_DIR}/../../node_modules
)

find_package(cmake-bare REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(bare_svg C)

if(DEFINED ENV{Rust_CARGO_TARGET})
  set(Rust_CARGO_TARGET $ENV{Rust_CARGO_TARGET})
endif()

include(FetchContent)

FetchContent_Declare(
  Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.5
)
FetchContent_MakeAvailable(Corrosion)

FetchContent_Declare(
  resvg
  GIT_REPOSITORY https://github.com/linebender/resvg.git
  GIT_TAG v0.46.0
)
FetchContent_MakeAvailable(resvg)

corrosion_import_crate(
  MANIFEST_PATH ${resvg_SOURCE_DIR}/crates/c-api/Cargo.toml
  CRATE_TYPES staticlib
)
corrosion_set_features(resvg FEATURES text system-fonts raster-images)

add_bare_module(bare_svg)

target_sources(
  ${bare_svg}
  PRIVATE
    binding.c
)

target_include_directories(
  ${bare_svg}
  PRIVATE
    ${resvg_SOURCE_DIR}/crates/c-api
)

target_link_libraries(${bare_svg} PRIVATE resvg)

if(APPLE)
  target_link_libraries(${bare_svg} PRIVATE
    "-framework CoreFoundation"
    "-framework CoreText"
    "-framework CoreGraphics"
  )
elseif(UNIX)
  target_link_libraries(${bare_svg} PRIVATE m pthread dl)
elseif(WIN32)
  target_link_libraries(${bare_svg} PRIVATE ws2_32 userenv ntdll gdi32 user32)
endif()
