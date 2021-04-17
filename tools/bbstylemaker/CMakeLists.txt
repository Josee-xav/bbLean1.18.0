project(bbstylemaker)

set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

set(bbstylemaker_SOURCES
	main.cpp
	writestyle.cpp
	${CMAKE_SOURCE_DIR}/blackbox/BImage.cpp
	${CMAKE_SOURCE_DIR}/blackbox/Settings.cpp
	#resource.res
	
	#readroot.cpp		# included manually in writestyle.cpp
	#rcfile.cpp			# included manually in writestyle.cpp
	#utils.cpp			# included manually in writestyle.cpp
	#snap.cpp			# included manually in bbdialog.cpp
	#bbdialog.cpp		# that one from main.cpp
)

set(bbstylemaker_RESOURCES
	resource.rc
)

add_definitions(-DBBSETTING_STYLEREADER_ONLY) #@FIXME
#add_definitions(-D__BBCORE__)
add_definitions(-DBBLIB_COMPILING)
add_definitions(-D__BBSM__) #used by Settings.cpp
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
add_executable(bbstylemaker WIN32 ${bbstylemaker_SOURCES} ${bbstylemaker_RESOURCES}) 
target_link_libraries(bbstylemaker bblib)
target_link_libraries(bbstylemaker version comctl32)

set_property(TARGET bbstylemaker PROPERTY FOLDER "tools/bbstylemaker")

install(TARGETS bbstylemaker
		RUNTIME DESTINATION .
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib/static)

	#install(FILES bbstylemaker.txt DESTINATION docs)
install(FILES readme.txt DESTINATION .)

#TODO: where is bbstylemaker.txt?
#INSTALL_FILES = $(BIN) -as docs/bbstylemaker.txt readme.txt
#install(FILES docs/bbstylemaker.txt DESTINATION docs)
