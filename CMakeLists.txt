cmake_minimum_required(VERSION 3.16)
project(LegionAura VERSION 1.1.2)

add_definitions(-DLEGIONAURA_VERSION=\"${PROJECT_VERSION}\")
add_compile_definitions(PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Subdirs
add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(gui)

# ------------------------------------------------------
# Install udev rules for non-root USB access
# ------------------------------------------------------
# Packages must install to /usr/lib/udev/rules.d
install(FILES udev/10-legionaura.rules
        DESTINATION /usr/lib/udev/rules.d)

# Auto-reload udev rules after install
install(CODE "message(STATUS \"Reloading udev rules...\")")
install(CODE "execute_process(COMMAND udevadm control --reload-rules)")
install(CODE "execute_process(COMMAND udevadm trigger)")

# ------------------------------------------------------
# Install desktop entry + icon
# ------------------------------------------------------
install(FILES legionaura.desktop
        DESTINATION /usr/share/applications)

install(FILES icons/legionaura.png
        DESTINATION /usr/share/icons/hicolor/256x256/apps)
