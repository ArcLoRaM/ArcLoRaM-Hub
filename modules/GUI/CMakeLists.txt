cmake_minimum_required(VERSION 3.16)
project(ARCLORAMGUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/exec)
set(CMAKE_BUILD_TYPE Debug)

# === SFML CONFIGURATION ===
set(SFML_ROOT "C:/Users/srcla/Desktop/ArcLoRaM/ArcLoRaM-Hub/modules/libraries/SFML-3.0.0")
include_directories("${SFML_ROOT}/include")
link_directories("${SFML_ROOT}/lib")

# === INCLUDE PATHS (ton projet)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/Shared/Libraries)

# === FICHIERS À COMPILER ===

# set(SOURCES
#     main.cpp
#     Node/Clock/Clock.cpp
#     Setup/Factories/FactorySelector/FactorySelector.cpp
#     Setup/Factories/RrcUplinkNodeFactory/RrcUplinkNodeFactory.cpp
#     Setup/DeploymentManager/DeploymentManager.cpp
#     Connectivity/Logger/Logger.cpp
#     Node/Node.cpp
#     Miscellaneous/PacketTool/PacketTool.cpp
#     Setup/Seed/Seed.cpp
#     PhyLayer/PhyLayer.cpp
#     Node/C1/C1_Node.cpp
#     Node/C2/C2_Node.cpp
#     Node/C2/C2RccUplinkSlotManager.cpp
#     Node/C3/C3_Node.cpp
#     Connectivity/TCP/Client.cpp
#     Connectivity/TCP/packets.cpp
# )

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# === EXÉCUTABLE ===
add_executable(ARCLORAMGUI ${SOURCES})

# === LIENS SFML ===
target_link_libraries(ARCLORAMGUI
    sfml-system
    sfml-network
    sfml-window
    sfml-graphics
)
