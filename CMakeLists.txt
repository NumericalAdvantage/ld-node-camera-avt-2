# This file is part of project link.developers/ld-node-camera-avt-2.
# It is copyrighted by the contributors recorded in the version control history of the file,
# available from its original location https://gitlab.com/link.developers/ld-node-camera-avt-2.
#
# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.0)
project(ld-node-camera-avt-2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--unresolved-symbols=ignore-in-shared-libs")
endif()

find_package(Link2-Cpp REQUIRED)
find_package(Flatc REQUIRED)
find_package(ld-cmake-tools REQUIRED)
find_package(OpenCV REQUIRED)
find_package(VmbAPI REQUIRED)
find_package(ld-lib-image REQUIRED)
find_package(libjpeg-turbo REQUIRED)

# When adding new data formats, add them to the INPUT_FILES section.
# If you use libraries that contain Flatbuffer files, uncomment the lines with LD_FLATBUFFER_ variables.
flatc_generate_cpp(
    BFBS_OUTPUT_FOLDER ${CMAKE_SOURCE_DIR}/data
    INCLUDE_PATHS ${LD_FLATBUFFER_DATA_FOLDERS}
    INPUT_FILES
        ${LD_FLATBUFFER_FILES}
    )

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/avtcamdriver.h
    src/avtcamdriver.cpp
    ${FLATC_GENERATED_SOURCES}
    )

target_include_directories(${PROJECT_NAME} PRIVATE ${FLATC_GENERATED_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC DRAIVE::Link2-Cpp 
                                             ${OpenCV_LIBS} 
                                             AVT::VmbAPI
                                             libjpeg-turbo::libjpeg-turbo)

cmake_make_installation(
        TARGETS ${PROJECT_NAME}
        LINK2_STATIC_ASSETS
            specification.json
            data
        IS_EXECUTABLE)
