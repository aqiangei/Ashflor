set(CMAKE_SYSTEM_NAME Generic)

cmake_minimum_required(VERSION 3.16)

project(gardn-client)
include_directories(..)

set(SRCS
    Assets/Flower.cc
    Assets/Mob.cc
    Assets/Petal.cc
    Assets/Web.cc
    Render/RenderDrop.cc
    Render/RenderFlower.cc
    Render/RenderHealth.cc
    Render/RenderMob.cc
    Render/RenderName.cc
    Render/RenderPetal.cc
    Render/RenderWeb.cc
    Render/Renderer.cc
    Debug.cc
    DOM.cc
    Game.cc
    I18n.cc
    Input.cc
    MapChunkCache.cc
    Main.cc
    Network.cc
    Particle.cc
    Rendering.cc
    Setup.cc
    Simulation.cc
    Socket.cc
    StaticData.cc
    Storage.cc
    Ui/InGame/InputFreeze.cc
    Ui/InGame/Leaderboard.cc
    Ui/InGame/LevelBar.cc
    Ui/InGame/LoadoutPetal.cc
    Ui/InGame/LoadoutSlot.cc
    Ui/InGame/Map.cc
    Ui/InGame/Mobile.cc
    Ui/InGame/OverlevelTimer.cc
    Ui/InGame/PauseMenu.cc
    Ui/InGame/Chat.cc
    Ui/InGame/Social.cc
    Ui/InGame/GamePanels.cc
    Ui/InGame/Tooltip.cc
    Ui/TitleScreen/Changelog.cc
    Ui/TitleScreen/DeathScreen.cc
    Ui/TitleScreen/MainScreen.cc
    Ui/TitleScreen/MobGallery.cc
    Ui/TitleScreen/PetalGallery.cc
    Ui/TitleScreen/Settings.cc
    Ui/TitleScreen/StatScreen.cc
    Ui/Button.cc
    Ui/Chrome.cc
    Ui/Choose.cc
    Ui/Container.cc
    Ui/DynamicText.cc
    Ui/Element.cc
    Ui/Extern.cc
    Ui/ScrollContainer.cc
    Ui/StaticIcon.cc
    Ui/StaticText.cc
    Ui/TextInput.cc
    Ui/Window.cc
    ../Helpers/Math.cc
    ../Helpers/UTF8.cc
    ../Helpers/Vector.cc
    ../Shared/Arena.cc
    ../Shared/Binary.cc
    ../Shared/Config.cc
    ../Shared/Craft.cc
    ../Shared/Entity.cc
    ../Shared/EntityDef.cc
    ../Shared/Map.cc
    ../Shared/MapGen.cc
    ../Shared/Simulation.cc
    ../Shared/StaticData.cc
)

set(CMAKE_CXX_COMPILER "em++")
set(CMAKE_CXX_FLAGS "-DCLIENTSIDE=1 -std=c++20")

# Chinese / multi-byte names & chat — default ON for ashflor
option(USE_CODEPOINT_LEN "Measure strings by Unicode codepoints" ON)
if (USE_CODEPOINT_LEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_CODEPOINT_LEN=1")
endif()
if (VERSION_HASH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERSION_HASH=${VERSION_HASH}ull")
endif()
if(DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG=1 -gdwarf-4 -sNO_DISABLE_EXCEPTION_CATCHING")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math --closure=1")
endif()

add_link_options(-sEXPORTED_RUNTIME_METHODS=stringToNewUTF8)
add_link_options(-sEXPORTED_FUNCTIONS=_main,_key_event,_mouse_event,_touch_event,_wheel_event,_clipboard_event,_loop,_on_message,_ashflor_cancel_petal_drags)
# Map/UI statics already ~19MB; without growth, Game::init malloc hits the ceiling and abort()s.
add_link_options(-sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=33554432 -sSTACK_SIZE=5242880)

add_executable(gardn-client ${SRCS})
set(CMAKE_EXECUTABLE_SUFFIX ".js")

if (WS_URL)
    target_compile_definitions(gardn-client PRIVATE WS_URL="${WS_URL}")
endif()

find_program(NODE_EXECUTABLE node)
if(NODE_EXECUTABLE AND NOT DEBUG)
    add_custom_command(TARGET gardn-client POST_BUILD
        COMMAND ${NODE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../Scripts/compress_client.js $<TARGET_FILE_DIR:gardn-client>
        COMMENT "Compressing client JS and WASM for web delivery")
endif()
