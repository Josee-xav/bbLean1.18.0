set(PNG_SOURCES
	pngwutil.c
	pngwtran.c
	pngwrite.c
	pngwio.c
	pngvcrd.c
	pngtrans.c
	pngset.c
	pngrutil.c
	pngrtran.c
	pngrio.c
	pngread.c
	pngpread.c
	pngmem.c
	pngget.c
	pnggccrd.c
	pngerror.c
	png.c
)

include_directories(${CMAKE_SOURCE_DIR}/tools/bsetroot/CXIMAGE/zlib)
add_library(png STATIC ${PNG_SOURCES})
target_link_libraries(png zlib)

set_property(TARGET png PROPERTY FOLDER "tools/png")

if ( CMAKE_COMPILER_IS_GNUCC )
    #set_property( TARGET main APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )
endif ( CMAKE_COMPILER_IS_GNUCC )
if ( MSVC )
    set_property( TARGET png APPEND_STRING PROPERTY COMPILE_FLAGS /W1 )
endif ( MSVC )
