#INSTALL_FILES = $(BIN) -as bbLeanBar+.txt readme.txt
set(BlackboxBar_SOURCES
	BlackboxBar.cpp
)

set(BlackboxBar_CONFIGS BlackboxBar.rc)
set(BlackboxBar_RESOURCES BlackboxBar_private.rc)

set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

add_definitions(-DNO_BBLIB)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(BlackboxBar SHARED ${BlackboxBar_SOURCES} ${BlackboxBar_RESOURCES})
target_link_libraries(BlackboxBar blackbox bblib bbPlugin)
target_link_libraries(BlackboxBar version comctl32)

set_property(TARGET BlackboxBar PROPERTY FOLDER "plugins/BlackboxBar")

install(TARGETS BlackboxBar RUNTIME DESTINATION plugins/BlackboxBar)
if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/plugins/BlackboxBar/${BlackboxBar_CONFIGS}")
	install(FILES ${BlackboxBar_CONFIGS} DESTINATION plugins/BlackboxBar)
endif()
