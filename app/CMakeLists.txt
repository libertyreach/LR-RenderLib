# Demo executable for exercising renderlib during development.
add_executable(renderlib_demo main.cpp)

target_link_libraries(renderlib_demo PRIVATE renderlib::renderlib)
