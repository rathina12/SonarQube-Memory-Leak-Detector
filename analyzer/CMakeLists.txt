cmake_minimum_required(VERSION 3.20)
project(mlpca_analyzer LANGUAGES CXX)

option(MLPCA_WITH_CLANG "Build optional Clang LibTooling frontend" OFF)
option(MLPCA_BUILD_TESTS "Build analyzer tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mlpca_core
  src/Analyzer.cpp
  src/Lexer.cpp
  src/JsonWriter.cpp
  src/Config.cpp
)
target_include_directories(mlpca_core PUBLIC include)
if (MSVC)
  target_compile_options(mlpca_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(mlpca_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

add_executable(mlpca-analyzer src/main.cpp)
target_link_libraries(mlpca-analyzer PRIVATE mlpca_core)

if(MLPCA_WITH_CLANG)
  find_package(LLVM REQUIRED CONFIG)
  find_package(Clang REQUIRED CONFIG)
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  target_compile_definitions(mlpca_core PUBLIC MLPCA_WITH_CLANG=1)
  target_sources(mlpca_core PRIVATE clang/ClangFrontend.cpp)
  target_include_directories(mlpca_core PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS} clang)
  target_link_libraries(mlpca_core PRIVATE
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangLex
    clangTooling
  )
endif()

if(MLPCA_BUILD_TESTS)
  enable_testing()
  add_executable(mlpca-tests tests/test_main.cpp)
  target_link_libraries(mlpca-tests PRIVATE mlpca_core)
  add_test(NAME mlpca-tests COMMAND mlpca-tests)
endif()
