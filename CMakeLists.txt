cmake_minimum_required(VERSION 3.16)
project(dragdropwrapper)

ADD_LIBRARY( dragdropwrapper STATIC
             ./dragdropwrapper/dragdropwrapper.h)

if(APPLE)
    target_sources(dragdropwrapper
    PRIVATE
        ./dragdropwrapper/dragdropwrapper_mac.mm
    )

    find_library(SCK ScreenCaptureKit)
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
    find_library(COREMEDIA_FRAMEWORK CoreMedia)

    target_link_libraries(dragdropwrapper
        ${SCKLIB_STATIC_LIB_PATH}
        ${SCK} 
        ${AVFOUNDATION_FRAMEWORK} 
        ${COREMEDIA_FRAMEWORK}
    )

endif()

target_include_directories(dragdropwrapper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dragdropwrapper)

target_compile_features(dragdropwrapper PUBLIC cxx_std_17)

