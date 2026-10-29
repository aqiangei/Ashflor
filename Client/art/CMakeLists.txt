set(CMAKE_SYSTEM_NAME Generic)
cmake_minimum_required(VERSION 3.16)
project(gardn-art)
include_directories(../..)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(SRCS
    ${ROOT}/ArtExport.cc
    ${ROOT}/ArtI18nStub.cc
    ${ROOT}/Assets/Flower.cc
    ${ROOT}/Assets/FlorrArt.cc
    ${ROOT}/Assets/FlorrTerrain.cc
    ${ROOT}/Assets/Mob.cc
    ${ROOT}/Assets/Petal.cc
    ${ROOT}/Assets/ThemeArt.cc
    ${ROOT}/Assets/Web.cc
    ${ROOT}/Render/Renderer.cc
    ${ROOT}/StaticData.cc
    ${ROOT}/../Helpers/Math.cc
    ${ROOT}/../Helpers/UTF8.cc
    ${ROOT}/../Helpers/Vector.cc
    ${ROOT}/../Shared/Arena.cc
    ${ROOT}/../Shared/Binary.cc
    ${ROOT}/../Shared/Config.cc
    ${ROOT}/../Shared/Craft.cc
    ${ROOT}/../Shared/Recipe.cc
    ${ROOT}/../Shared/Entity.cc
    ${ROOT}/../Shared/EntityDef.cc
    ${ROOT}/../Shared/Map.cc
    ${ROOT}/../Shared/MapGen.cc
    ${ROOT}/../Shared/Simulation.cc
    ${ROOT}/../Shared/StaticData.cc
)

set(CMAKE_CXX_COMPILER "em++")
set(CMAKE_CXX_FLAGS "-DCLIENTSIDE=1 -DART_STANDALONE=1 -std=c++20 -O3 -ffast-math -DUSE_CODEPOINT_LEN=1")

add_link_options(-sEXPORTED_RUNTIME_METHODS=stringToUTF8,UTF8ToString,lengthBytesUTF8)
add_link_options(-sEXPORTED_FUNCTIONS=_main,_ashflor_art_boot,_ashflor_art_ctx_id,_ashflor_art_draw_petal,_ashflor_art_draw_mob,_ashflor_art_draw_theme,_malloc,_free)
add_link_options(-sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=67108864 -sSTACK_SIZE=2097152)
add_link_options(-sENVIRONMENT=web)
add_link_options(-sMODULARIZE=1 -sEXPORT_NAME=createGardnArt)
add_link_options(-sFILESYSTEM=0)

add_executable(gardn-art ${SRCS})
set(CMAKE_EXECUTABLE_SUFFIX ".js")
