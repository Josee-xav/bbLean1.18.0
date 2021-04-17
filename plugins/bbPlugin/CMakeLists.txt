include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)

set(bbPlugin_RESOURCES bbLeanBar.rc)
set(bbPlugin_SOURCES
	bbPlugin.cpp
	drawico.cpp
	sysmenu.cpp
	tooltips.cpp
  MessageBox.cpp
)

SET(bbPlugin_HEADERS
	bbPlugin.h
	drawico.h
	MessageBox.h
	Tooltips.h
)

add_library(bbPlugin STATIC ${bbPlugin_SOURCES} ${bbPlugin_HEADERS})
target_link_libraries(bbPlugin blackbox bblib)

set_property(TARGET bbPlugin PROPERTY FOLDER "plugins/bbPlugin")

