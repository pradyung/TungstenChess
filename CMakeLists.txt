cmake_minimum_required(VERSION 3.21)
project(Chess LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

add_executable(Chess src/Core/main.cpp src/Core/Board/board.cpp src/Core/GUI/GUI.cpp src/Core/Helpers/bitboard.cpp src/Core/Helpers/piece.cpp src/Core/Helpers/move.cpp src/Core/Helpers/openings.cpp src/Core/Helpers/zobrist.cpp src/Core/Helpers/transposition_table.cpp)
target_link_libraries(Chess PRIVATE sfml-graphics)
target_compile_features(Chess PRIVATE cxx_std_17)
if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET Chess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Chess> $<TARGET_FILE_DIR:Chess> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS Chess)