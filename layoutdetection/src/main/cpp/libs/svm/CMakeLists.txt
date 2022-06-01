project(svm VERSION 1.0.0
	DESCRIPTION "svm"
        LANGUAGES CXX
)

add_library(${PROJECT_NAME} 
        STATIC
        svm.cpp
        include/svm.h
        svm_util.cpp
        include/svm_util.h
)
add_library(libs::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

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
