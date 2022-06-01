cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(layoutDectection VERSION 1.0.0
        DESCRIPTION "layoutDectection"
        LANGUAGES CXX
)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


#### Find dependencies
# OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "Use OpenCV ${OpenCV_VERSION}")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../layoutdetection/src/main/cpp/libs libs)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    libs::feature
    libs::svm    	
)

target_include_directories(${PROJECT_NAME}
    PUBLIC        
        ${PROJECT_BINARY_DIR}
		$<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cxxopts>
)

add_executable(detection detection.cpp)

target_link_libraries(detection
    PUBLIC
    libs::feature
    libs::svm
    opencv_videoio
    opencv_imgproc
)

target_include_directories(detection
    PUBLIC        
        ${PROJECT_BINARY_DIR}
		$<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cxxopts>
)
