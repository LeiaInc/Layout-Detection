project(feature VERSION 1.0.0
	DESCRIPTION "feature"
        LANGUAGES CXX
)

add_library(${PROJECT_NAME} 
        STATIC
        feature.cpp
        include/feature.h
)
add_library(libs::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PRIVATE 
        opencv_imgproc
        
        INTERFACE
        opencv_core
        opencv_imgcodecs
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(${PROJECT_NAME}
	PUBLIC
        cxx_std_17
)
