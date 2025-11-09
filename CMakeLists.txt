cmake_minimum_required(VERSION 3.16)
project(LegionAura VERSION 1.0.1)

add_definitions(-DLEGIONAURA_VERSION=\"${PROJECT_VERSION}\")



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(gui)


# Install udev rules for non-root USB access
install(FILES udev/10-legionaura.rules
        DESTINATION /etc/udev/rules.d)

install(CODE "message(STATUS \"Reloading udev rules...\")")
install(CODE "execute_process(COMMAND udevadm control --reload-rules)")
install(CODE "execute_process(COMMAND udevadm trigger)")
