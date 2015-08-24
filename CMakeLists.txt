  
cmake_minimum_required(VERSION 2.8)
FIND_PACKAGE(OpenCV REQUIRED)                                                                            
add_executable( rt rt.cpp )
TARGET_LINK_LIBRARIES( rt ${OpenCV_LIBS})  

