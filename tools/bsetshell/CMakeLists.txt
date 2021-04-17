project(bsetshell)

add_definitions(-DUNICODE -D_UNICODE)

set(bsetshell_SOURCES bsetshell.cpp)
set(bsetshell_RESOURCES resource.rc)

set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

add_executable(bsetshell ${bsetshell_SOURCES} ${bsetshell_RESOURCES})
set_target_properties(bsetshell PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
target_link_libraries(bsetshell bblib)
target_link_libraries(bsetshell version comctl32)

set_property(TARGET bsetshell PROPERTY FOLDER "tools/bsetshell")

#if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/${bsetshell_RESOURCES}")
#	install(FILES ${bsetshell_RESOURCES} DESTINATION .)
#endif()

install(TARGETS bsetshell RUNTIME DESTINATION .)


