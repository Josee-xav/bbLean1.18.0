set(BBLIB_STATIC true)

set(bblib_SOURCES
	bbrc.c
	styleprops.c
	bbroot.c
	bools.c
	colors.c
	m_alloc.c
	moreutils.c
	numbers.c
	paths.c
	pidl.c
	strings.c
	tinylist.c
	tokenize.c
	winutils.c
)

set(bblib_HEADERS
	bblib.h
	bbrc.h
	bbroot.h
	bbshell.h
)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
add_definitions(-DBBLIB_COMPILING)
#NO_BBLIB = 1

if (BBLIB_STATIC)
	add_definitions(-DBBLIB_STATIC)
	add_library(bblib STATIC ${bblib_SOURCES})
elseif (NOT BBLIB_STATIC)
	add_library(bblib SHARED ${bblib_SOURCES})
	#BIN = bblib.dll
	#IMPLIB = 1
	#INSTALL_FILES = $(BIN)
endif (BBLIB_STATIC)

target_link_libraries(bblib version comctl32)
target_link_libraries(bblib logging)
