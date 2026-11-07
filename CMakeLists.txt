cmake_minimum_required(VERSION 3.16)
project(LegionAura VERSION 2.2.0)

include(GNUInstallDirs)

add_definitions(-DLEGIONAURA_VERSION=\"${PROJECT_VERSION}\")
add_compile_definitions(PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Subdirs
add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(gui)

# ------------------------------------------------------
# Install runtime data
# ------------------------------------------------------
install(FILES devices/devices.json
        DESTINATION /usr/share/legionaura)

# ------------------------------------------------------
# Install udev rules for non-root USB access
# ------------------------------------------------------
# Packages must install to /usr/lib/udev/rules.d
install(FILES udev/60-legionaura.rules
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

# ------------------------------------------------------
# Auto-apply last settings on graphical login (XDG autostart)
# ------------------------------------------------------
install(FILES legionaura-autostart.desktop
        DESTINATION /usr/share/xdg/autostart)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Gui)
    add_executable(screen-colors-test tests/ScreenColorsTest.cpp)
    target_include_directories(screen-colors-test PRIVATE gui)
    target_link_libraries(screen-colors-test PRIVATE Qt6::Gui)
    add_test(NAME screen-colors COMMAND screen-colors-test)

    add_executable(accent-color-test
        tests/AccentColorTest.cpp
        gui/accent/AccentManager.cpp
    )
    target_include_directories(accent-color-test PRIVATE gui)
    target_link_libraries(accent-color-test PRIVATE Qt6::Gui)
    add_test(NAME accent-color COMMAND accent-color-test)
endif()
