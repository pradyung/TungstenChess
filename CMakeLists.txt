cmake_minimum_required(VERSION 3.16)
project(TungstenChess LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

find_package(SFML 2.6 COMPONENTS graphics REQUIRED)

if (NOT SFML_FOUND)
  include(FetchContent)
  FetchContent_Declare(SFML 
    GIT_REPOSITORY https://github.com/SFML/SFML.git)
  FetchContent_MakeAvailable(SFML)
endif()

file(GLOB_RECURSE RESOURCES "resources/*")
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*\\.main.cpp")

if (APPLE)
  list(FILTER RESOURCES EXCLUDE REGEX ".*\\.DS_Store")
  set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  
  add_executable(TungstenChess MACOSX_BUNDLE ${SOURCES} ${RESOURCES})
  set_target_properties(TungstenChess PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
  target_include_directories(TungstenChess PRIVATE include)
  target_link_libraries(TungstenChess PRIVATE sfml-graphics pthread "-framework CoreFoundation")
  target_compile_features(TungstenChess PRIVATE cxx_std_23)
  
  install(TARGETS TungstenChess BUNDLE DESTINATION .)
else()
  if (WIN32)
    add_executable(TungstenChess WIN32 ${SOURCES} ${RESOURCES})
  else()
    add_executable(TungstenChess ${SOURCES} ${RESOURCES})
  endif()

  set(BINARY_DIR "${CMAKE_BINARY_DIR}/TungstenChess")
  set_target_properties(TungstenChess PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
  file(MAKE_DIRECTORY ${BINARY_DIR})
  
  target_include_directories(TungstenChess PRIVATE include)
  target_link_libraries(TungstenChess PRIVATE sfml-graphics pthread)
  set(RESOURCES_DIR "${BINARY_DIR}/Resources")
  file(MAKE_DIRECTORY ${RESOURCES_DIR})
  file(COPY ${RESOURCES} DESTINATION ${RESOURCES_DIR})
  add_compile_definitions(TUNGSTENCHESS_RESOURCES_DIR="${RESOURCES_DIR}")
  target_compile_features(TungstenChess PRIVATE cxx_std_23)
  
  install(TARGETS TungstenChess RUNTIME DESTINATION .)
endif()