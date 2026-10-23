cmake_minimum_required(VERSION 3.16)
# Only pin Generic toolchain for Emscripten; native needs a real host system.
if(WASM_SERVER)
    set(CMAKE_SYSTEM_NAME Generic)
endif()
project(gardn-server)

include_directories(..)

set(SOURCES
    EntityFunctions/Damage.cc
    EntityFunctions/Death.cc
    EntityFunctions/Detection.cc
    EntityFunctions/Misc.cc
    Process/Ai.cc
    Process/Camera.cc
    Process/Collision.cc
    Process/Culling.cc
    Process/Curse.cc
    Process/Drop.cc
    Process/Flower.cc
    Process/FlowerAi.cc
    Process/Health.cc
    Process/Motion.cc
    Process/Petal.cc
    Process/Score.cc
    Process/Segment.cc
    AccountStore.cc
    AdminGiveLog.cc
    Chat.cc
    Client.cc
    DevCheats.cc
    Game.cc
    LeaderboardStore.cc
    Main.cc
    Guild.cc
    Party.cc
    PetalTracker.cc
    RateLimit.cc
    Server.cc
    Simulation.cc
    Spawn.cc
    TeamManager.cc
    Treasure.cc
    WorldEvent.cc
    WorldExplore.cc
    XpMigration.cc
    ../Helpers/Math.cc
    ../Helpers/UTF8.cc
    ../Helpers/Vector.cc
    ../Shared/Arena.cc
    ../Shared/Binary.cc
    ../Shared/Config.cc
    ../Shared/Craft.cc
    ../Shared/Recipe.cc
    ../Shared/Entity.cc
    ../Shared/EntityDef.cc
    ../Shared/Map.cc
    ../Shared/MapGen.cc
    ../Shared/Simulation.cc
    ../Shared/StaticData.cc
)

if(WASM_SERVER)
    set(SOURCES ${SOURCES} Wasm.cc)
else()
    set(SOURCES ${SOURCES} Native.cc)
endif()
# Large bosses / wide magnet pulls — default ON for ashflor
option(GENERAL_SPATIAL_HASH "Canonical spatial hash (large entities)" ON)
if(GENERAL_SPATIAL_HASH)
    set(SOURCES ${SOURCES} SpatialHashCanonical.cc)
else()
    set(SOURCES ${SOURCES} SpatialHashUniform.cc)
endif()
set(CMAKE_CXX_FLAGS "-std=c++20 -DSERVERSIDE=1")
# Petal attribute tables use 1.0 for uint8_t flags in places; don't fail Release.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")

if (TDM)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGAMEMODE_TDM=1")
endif()
option(USE_CODEPOINT_LEN "Measure strings by Unicode codepoints" ON)
if (USE_CODEPOINT_LEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_CODEPOINT_LEN=1")
endif()
if (VERSION_HASH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERSION_HASH=${VERSION_HASH}ull")
endif()
if (SERVER_PORT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSERVER_PORT=${SERVER_PORT}")
endif()
if(DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-4 -DDEBUG=1")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
endif()

if(WASM_SERVER)
    set(CMAKE_CXX_COMPILER "em++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWASM_SERVER=1")
    add_link_options(
        -sEXIT_RUNTIME=0
        # 8 maps × Simulation(ENTITY_CAP≈8192) needs a large static BSS + growth headroom
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=536870912
        -sMAXIMUM_MEMORY=2147483648
        -sSTACK_SIZE=2097152
        -sEXPORTED_FUNCTIONS=_main,_on_connect,_on_disconnect,_tick,_on_message,_sigusr2,_malloc,_free
        --js-library=${CMAKE_CURRENT_SOURCE_DIR}/node_fs_library.js
    )
    if (NOT DEBUG) 
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --closure=1")
    endif()
    add_executable(gardn-server ${SOURCES})
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
else()
    # Native builds should use clang++ (-DCMAKE_CXX_COMPILER=clang++): g++ rejects
    # out-of-order designated initializers in StaticData as hard C++20 errors.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder-init-list")
    add_executable(gardn-server ${SOURCES})
    target_include_directories(gardn-server PRIVATE ${CMAKE_SOURCE_DIR}/uWebSockets/src)
    target_include_directories(gardn-server PRIVATE ${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/src)
    # Link archive by path (avoids -l: name quirks); uSockets must be built WITHOUT -flto
    # when using clang++ (GCC LTO bitcode will not resolve).
    set(USOCKETS_A "${CMAKE_SOURCE_DIR}/uWebSockets/uSockets/uSockets.a")
    target_link_libraries(gardn-server "${USOCKETS_A}" uv z pthread dl)
    if(CMAKE_HOST_WIN32)
        target_link_libraries(gardn-server ws2_32)
    endif()
endif()