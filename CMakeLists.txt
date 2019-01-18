CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

PROJECT(EWI)

set (CMAKE_CXX_STANDARD 11)

FIND_PACKAGE(OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIRS} )

INCLUDE_DIRECTORIES(/usr/include/libirimager)
LINK_DIRECTORIES(/usr/lib)
LINK_LIBRARIES(irdirectsdk udev pthread usb-1.0)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../../build)

SET(CMAKE_BUILD_TYPE Release)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -O2")

ADD_EXECUTABLE(direct_binding_tcp_show  direct_binding_tcp_show.cpp )

ADD_EXECUTABLE(direct_binding_usb_show  direct_binding_usb_show.cpp )

ADD_EXECUTABLE(serializeYSU  serializeYSU.cpp )

ADD_EXECUTABLE(readRaw  readRaw.cpp )
#linking

TARGET_LINK_LIBRARIES(direct_binding_tcp_show  irdirectsdk
                                               ${OpenCV_LIBRARIES})
                                               
TARGET_LINK_LIBRARIES(readRaw  irdirectsdk
                               ${OpenCV_LIBRARIES})

TARGET_LINK_LIBRARIES(serializeYSU  irdirectsdk
						   udev
				  	           ${OpenCV_LIBRARIES})

TARGET_LINK_LIBRARIES(direct_binding_usb_show  irdirectsdk
						   udev
				  	           ${OpenCV_LIBRARIES})
