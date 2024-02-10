cmake_minimum_required(VERSION 3.16)
project(Chess LANGUAGES CXX)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

file(GLOB_RECURSE RESOURCES "resources/*")
set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

add_executable(Chess MACOSX_BUNDLE src/GUI.cpp src/board.cpp ${RESOURCES})
set_target_properties(Chess PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
target_link_libraries(Chess PRIVATE sfml-graphics pthread "-framework CoreFoundation")
target_compile_features(Chess PRIVATE cxx_std_17)

install(TARGETS Chess BUNDLE DESTINATION .)