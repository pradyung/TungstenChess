cmake_minimum_required(VERSION 3.16)
project(Chess LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

add_executable(Chess src/GUI.cpp src/board.cpp)
target_link_libraries(Chess PRIVATE sfml-graphics pthread)
target_compile_features(Chess PRIVATE cxx_std_17)

add_executable(ChessUCI src/UCI.cpp src/board.cpp)
target_compile_features(ChessUCI PRIVATE cxx_std_17)

install(TARGETS Chess)