project(bbnote)

set(bbnote_exe_SOURCES
    bbnote.cpp
    edfiles.cpp
    edfunc.cpp
    match3.cpp
    edprint.cpp
    edproc.cpp
    bbapi-small.cpp
    readstyle.cpp
	${CMAKE_SOURCE_DIR}/blackbox/BImage.cpp
	${CMAKE_SOURCE_DIR}/blackbox/Settings.cpp
#   resource.res
)

#INSTALL_FILES = $(BIN) -to docs bbnote.txt bbnote.htm

set(bbnote_exe_RESOURCES
  resource.rc
)

set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
add_executable(bbnote WIN32 ${bbnote_exe_SOURCES} ${bbnote_exe_RESOURCES}) 
target_link_libraries(bbnote bblib blackbox)
target_link_libraries(bbnote version comctl32)

set_property(TARGET bbnote PROPERTY FOLDER "tools/bbnote")

add_library(bbnote-proxy SHARED bbnote-proxy.cpp)
target_link_libraries(bbnote-proxy bblib blackbox)
set_property(TARGET bbnote-proxy PROPERTY FOLDER "tools/bbnote-proxy")

#TODO: only for Settings?
#Settings.obj : DEFINES += -D BBSETTING_STYLEREADER_ONLY -D BBSETTING_NOWINDOW
add_definitions(-DBBSETTING_STYLEREADER_ONLY)
add_definitions(-DBBSETTING_NOWINDOW)

install(TARGETS bbnote bbnote-proxy
		RUNTIME DESTINATION .
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib/static)

install(FILES bbnote.txt DESTINATION docs)
#bbnote.obj: bbstyle.h
#resource.res: bbnote.ico
