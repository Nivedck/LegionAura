add_executable(legionaura
    legionaura.cpp
)

target_link_libraries(legionaura PRIVATE legionaura_lib)

install(TARGETS legionaura
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
