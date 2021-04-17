# You need
#   CxImage - http://www.codeproject.com/bitmap/cximage.asp
#   (tested with version 5.11)

set(XIMA_SOURCES
  ximage.cpp
  ximaenc.cpp
  ximapal.cpp
  ximatran.cpp
  ximawnd.cpp
  xmemfile.cpp

  ximabmp.cpp
  ximagif.cpp
  ximajpg.cpp
  ximapng.cpp
)

set(XIMA_OTHER_SOURCES
  ximaico.cpp
  ximatif.cpp
  ximalpha.cpp
  ximasel.cpp
  ximadsp.cpp
  ximajbg.cpp
  ximaj2k.cpp
  ximawbmp.cpp
  ximaexif.cpp
  ximawmf.cpp
  ximamng.cpp
  ximapcx.cpp
  ximatga.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/tools/bsetroot/CXIMAGE/zlib)
add_library(cximage STATIC ${XIMA_SOURCES})
set_property(TARGET cximage PROPERTY FOLDER "tools/cximage")
