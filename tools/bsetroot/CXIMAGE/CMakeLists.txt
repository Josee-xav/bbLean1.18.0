add_subdirectory(zlib)
add_subdirectory(jpeg)
add_subdirectory(png)
add_subdirectory(CxImage)

add_dependencies(png zlib)
