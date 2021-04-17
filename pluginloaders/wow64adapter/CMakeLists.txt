set(WOW64Adapter_SOURCES
	PipeConnect/PipeClient.cpp
	PipeConnect/PipeServer.cpp
	PipeConnect/Debug.cpp
	
	Wow64Adapter.cpp
)

set(WOW64Adapter_HEADERS
	PipeConnect/PipeClient.hpp
	PipeConnect/PipeServer.hpp
	PipeConnect/PipeConnect.h
	PipeConnect/Serializer.hpp
	PipeConnect/Messages.hpp
	PipeConnect/Debug.h
	
	Wow64Adapter.h
)

include_directories(${CMAKE_SOURCE_DIR}/blackbox)
include_directories(${CMAKE_SOURCE_DIR}/lib)
add_library(WOW64Adapter SHARED 
            ${WOW64Adapter_SOURCES}
			${WOW64Adapter_HEADERS})

target_link_libraries(WOW64Adapter blackbox bblib version comctl32 gdiplus winmm msimg32 shlwapi comdlg32)

set_property(TARGET WOW64Adapter PROPERTY FOLDER "pluginloaders/WOW64Adapter")

install(TARGETS WOW64Adapter RUNTIME DESTINATION pluginloaders/WOW64Adapter)
