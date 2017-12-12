SET(pwd ${CMAKE_CURRENT_LIST_DIR})

AUX_SOURCE_DIRECTORY(${pwd} src)
add_executable(boids ${src})
message(STATUS "boids added")

target_link_libraries(boids ${stdgl_libraries})
