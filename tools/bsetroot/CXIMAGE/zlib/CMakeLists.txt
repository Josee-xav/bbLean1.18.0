set(ZLIB_SOURCES
	adler32.c
	compress.c
	crc32.c
	deflate.c
	gzio.c
	infblock.c
	infcodes.c
	inffast.c
	inflate.c
	inftrees.c
	infutil.c
	maketree.c
#	minigzip.c # example program
	trees.c
	uncompr.c
	zutil.c
)

add_library(zlib STATIC ${ZLIB_SOURCES})

set_property(TARGET zlib PROPERTY FOLDER "tools/zlib")
if ( CMAKE_COMPILER_IS_GNUCC )
    #set_property( TARGET main APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )
endif ( CMAKE_COMPILER_IS_GNUCC )
if ( MSVC )
    set_property( TARGET zlib APPEND_STRING PROPERTY COMPILE_FLAGS /W1 )
endif ( MSVC )
