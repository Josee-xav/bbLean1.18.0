project(bsetroot)
#TODO: image_tiny.obj bsetroot.htm

add_subdirectory(CXIMAGE)

set(CMAKE_RC_COMPILER_INIT windres)
ENABLE_LANGUAGE(RC)

# bsetroot
set(bsetroot_SOURCES
	bsetroot.cpp
	image_cx.cpp
	${CMAKE_SOURCE_DIR}/blackbox/BImage.cpp
)
set(bsetroot_CONFIGS
	bsetroot.rc
)
set(bsetroot_RESOURCES
	bsrt-rsc.rc
)

add_executable(bsetroot WIN32 ${bsetroot_SOURCES} ${bsetroot_RESOURCES})
include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(bsetroot cximage zlib jpeg png)
target_link_libraries(bsetroot bblib blackbox)
target_link_libraries(bsetroot version comctl32 )
set_property(TARGET bsetroot PROPERTY FOLDER "tools/bsetroot")

if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/${bsetroot_CONFIGS}")
	install(FILES ${bsetroot_CONFIGS} DESTINATION .)
endif()

# bsetbg
set(bsetbg_SOURCES
	bsetbg.cpp
)
set(bsetbg_RESOURCES
	bsbg-rsc.rc
)

add_executable(bsetbg WIN32 ${bsetbg_SOURCES} ${bsetbg_RESOURCES}) 
target_link_libraries(bsetbg bblib)
target_link_libraries(bsetbg version comctl32)
set_property(TARGET bsetbg PROPERTY FOLDER "tools/bsetbg")

# common install
install(TARGETS bsetroot bsetbg
		RUNTIME DESTINATION .
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib/static)


