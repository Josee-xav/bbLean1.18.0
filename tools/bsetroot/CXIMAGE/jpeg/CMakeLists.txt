set(JPEG_SOURCES
	jutils.c
	jquant2.c
	jquant1.c
	jmemnobs.c
	jmemmgr.c
	jidctred.c
	jidctint.c
	jidctfst.c
	jidctflt.c
	jcapimin.c
	jfdctfst.c
	jfdctflt.c
	jerror.c
	jdtrans.c
	jdsample.c
	jdpostct.c
	jdphuff.c
	jdmerge.c
	jdmaster.c
	jdmarker.c
	jdmainct.c
	jdinput.c
	jdhuff.c
	jddctmgr.c
	jdcolor.c
	jdcoefct.c
	jdatasrc.c
	jdatadst.c
	jdapistd.c
	jdapimin.c
	jctrans.c
	jcsample.c
	jcprepct.c
	jcphuff.c
	jcparam.c
	jcomapi.c
	jcmaster.c
	jcmarker.c
	jcmainct.c
	jcinit.c
	jchuff.c
	jcdctmgr.c
	jccolor.c
	jccoefct.c
	jcapistd.c
	jfdctint.c
)

add_library(jpeg STATIC ${JPEG_SOURCES})

set_property(TARGET jpeg PROPERTY FOLDER "tools/jpeg")

if ( CMAKE_COMPILER_IS_GNUCC )
    #set_property( TARGET main APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )
endif ( CMAKE_COMPILER_IS_GNUCC )
if ( MSVC )
    set_property( TARGET jpeg APPEND_STRING PROPERTY COMPILE_FLAGS /W1 )
endif ( MSVC )
