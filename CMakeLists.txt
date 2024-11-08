cmake_minimum_required(VERSION 3.27)
project(DCMTKTEST)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_definitions(-DHAVE_CONFIG_H)

add_definitions(-DSOURCE_CODE_LOCATION="${CMAKE_SOURCE_DIR}")

set(SOURCE_FILES main.cpp)

find_package(DCMTK REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${DCMTK_INCLUDE_DIRS})
include_directories(${OPENCV_INCLUDE_DIRS})

# set output executable of our test program
add_executable(DCMTKTEST src/main.cpp)
add_executable(Compression src/Compress.cpp)
add_executable(DCMTKNET src/net/TestNet.cpp
        src/net/SCU.cpp
        src/net/SCU.h)

# link DCMTK library files
target_link_libraries(DCMTKTEST DCMTK::DCMTK)
target_link_libraries(DCMTKTEST  ${OpenCV_LIBS})

#target_link_libraries(Compression dcmjpeg dcmect dcmpmap dcmtract dcmseg dcmfg dcmiod dcmrt dcmpstat dcmqrdb dcmwlm dcmdsig cmr dcmsr dcmnet dcmtls dcmtkcharls dcmjpls ijg16 ijg12 ijg8 dcmjpeg dcmimage dcmimgle dcmxml i2d dcmdata oflog ofstd oficonv z)
target_link_libraries(Compression DCMTK::DCMTK)
target_link_libraries(Compression  ${OpenCV_LIBS})

target_link_libraries(DCMTKNET DCMTK::DCMTK)
target_link_libraries(DCMTKNET  ${OpenCV_LIBS})