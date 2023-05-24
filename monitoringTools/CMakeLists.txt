cmake_minimum_required(VERSION 3.8)
project(monitoringTools)

# C++14 설정
set(CMAKE_CXX_STANDARD 14)

# 모든 .h, .cpp 파일 수집
file(GLOB_RECURSE headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE sources ${PROJECT_SOURCE_DIR}/*.cpp)

# 라이브러리 생성
add_library(monitoringTools ${sources} ${headers})

# 라이브러리 포함 경로 설정
target_include_directories(monitoringTools PRIVATE ${PROJECT_SOURCE_DIR})

# Windows에 종속된 경우에만 컴파일
if(WIN32)
  target_compile_definitions(monitoringTools PRIVATE -DWIN32_LEAN_AND_MEAN)
endif()
