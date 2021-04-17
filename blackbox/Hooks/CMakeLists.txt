#NO_BBLIB = 1
# INSTALL_FILES = $(BIN) -as docs/bbhooks.txt readme.txt

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/plugins/bbPlugin)
add_library(deskhook SHARED deskhook.cpp)
target_link_libraries(deskhook blackbox bblib bbPlugin)
target_link_libraries(deskhook version comctl32)
set_property(TARGET deskhook PROPERTY FOLDER "hooks/deskhook")

add_library(trayhook SHARED trayhook.cpp)
target_link_libraries(trayhook blackbox bblib bbPlugin)
target_link_libraries(trayhook version comctl32)
set_property(TARGET trayhook PROPERTY FOLDER "hooks/trayhook")

#install(FILES readme.txt DESTINATION plugins/bbAnalog)
install(TARGETS deskhook RUNTIME DESTINATION .)
#install(TARGETS trayhook RUNTIME DESTINATION .)

