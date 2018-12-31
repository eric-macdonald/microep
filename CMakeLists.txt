CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

FIND_PACKAGE(OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIRS} )

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../../build)

SET(CMAKE_BUILD_TYPE Release)

SET(CMAKE_C_FLAGS "-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")

SET(CMAKE_CXX_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=0 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -O2")

INCLUDE_DIRECTORIES(/usr/include/libirimager)
LINK_DIRECTORIES(/usr/lib)
LINK_LIBRARIES(irdirectsdk udev pthread usb-1.0)

ADD_EXECUTABLE(direct_binding_tcp_show  direct_binding_tcp_show.cpp )
ADD_EXECUTABLE(direct_binding_usb_show  direct_binding_usb_show.cpp )
ADD_EXECUTABLE(serializeRaw  serializeRaw.cpp )
ADD_EXECUTABLE(convertRaw2PPM  convertRaw2PPM.cpp )
ADD_EXECUTABLE(checkRawFramerate  checkRawFramerate.cpp )


#linking
TARGET_LINK_LIBRARIES(direct_binding_tcp_show  irdirectsdk
                                               ${OpenCV_LIBRARIES})
                                               
TARGET_LINK_LIBRARIES(direct_binding_usb_show  irdirectsdk udev
				   	       ${OpenCV_LIBRARIES})
