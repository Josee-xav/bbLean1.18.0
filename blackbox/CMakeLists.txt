project(blackbox)

include("${CMAKE_SOURCE_DIR}/build/version.cmake")

add_custom_target(bbversion_h ALL)
add_custom_command(TARGET bbversion_h COMMAND ${CMAKE_COMMAND}
         -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
         -P ${CMAKE_SOURCE_DIR}/build/version.cmake)

SOURCE_GROUP("Source Files\\Menu" REGULAR_EXPRESSION "Menu/.*\\.cpp")
SOURCE_GROUP("Header Files\\Menu" REGULAR_EXPRESSION "Menu/.*\\.h(pp)?")

SOURCE_GROUP("Source Files\\PluginManager" REGULAR_EXPRESSION "PluginManager/.*\\.cpp")
SOURCE_GROUP("Header Files\\PluginManager" REGULAR_EXPRESSION "PluginManager/.*\\.h(pp)?")

set(blackbox_SOURCES
	Menu/CommandItem.cpp
	Menu/Contextmenu.cpp
	Menu/Dragsource.cpp
	Menu/Droptarget.cpp
	Menu/FolderItem.cpp
	Menu/Menu.cpp
	Menu/MenuItem.cpp
	Menu/MenuMaker.cpp
	Menu/RecentItem.cpp
	Menu/SpecialFolder.cpp
	Menu/TitleItem.cpp
	Menu/SearchItem.cpp
  Search/index.cpp
  Search/history.cpp
  Search/lookup.cpp
  Search/complete.cpp
  Search/rc.cpp
	
	BBApi.cpp
	BBRoot.cpp
	BBVWM.cpp
	BImage.cpp
	Blackbox.cpp
	ConfigMenu.cpp
	Desk.cpp
	DesktopMenu.cpp
  DrawText.cpp
	MessageManager.cpp
	Settings.cpp
	Toolbar.cpp
	Tray.cpp
	Utils.cpp
	Workspaces.cpp
)

set(PluginManager_SOURCES
    PluginManager/PluginManager.cpp
    PluginManager/PluginLoaderNative.cpp
)

set (blackbox_HEADERS
	BB.h
	BBApi.h
	BBApiPluginLoader.h
	BBSendData.h
	bbversion.h
	BBVWM.h
	BImage.h
	DataTypes.h
	Desk.h
	MessageManager.h
	Settings.h
	Stylestruct.h
	Toolbar.h
	Tray.h
	win0x500.h
	Workspaces.h
	Menu/Menu.h
	Menu/MenuMaker.h
	Menu/RecentItem.h
	Menu/SearchItem.h
)

set(PluginManager_HEADERS
    PluginManager/PluginManager.h
    PluginManager/PluginLoaderNative.h
    PluginManager/Types.h
)

if (NOT MINGW)
  set(blackbox_SOURCES ${blackbox_SOURCES} ${PluginManager_SOURCES})
  set(blackbox_HEADERS ${blackbox_HEADERS} ${PluginManager_HEADERS})
endif (NOT MINGW)

set(blackbox_RESOURCES resource.rc)
set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

if(MINGW)
 SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif(MINGW)

add_definitions(-D__BBCORE__)
include_directories(${CMAKE_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

add_executable(blackbox WIN32
	${blackbox_SOURCES}
	${blackbox_HEADERS}
	${blackbox_RESOURCES}
)

set_target_properties(blackbox PROPERTIES ENABLE_EXPORTS ON)
target_link_libraries(blackbox bblib)
target_link_libraries(blackbox version comctl32)

install(TARGETS blackbox
		RUNTIME DESTINATION .
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib/static)

if(MSVC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/blackbox.pdb
	DESTINATION .
    CONFIGURATIONS Debug
    )
endif(MSVC)

# old stuff from makefile
#INSTALL_FILES = $(BIN) -to docs Menu/menu-bullets.bmp nls-c.txt

