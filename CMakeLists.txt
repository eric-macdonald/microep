CMAKE_MINIMUM_REQUIRED (VERSION 2.6)


FIND_PACKAGE(OpenCV REQUIRED )
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIRS} )

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../../build)

SET(CMAKE_BUILD_TYPE Release)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -O2")

ADD_EXECUTABLE(direct_binding_tcp_show  direct_binding_tcp_show.cpp )

ADD_EXECUTABLE(direct_binding_usb_show  direct_binding_usb_show.cpp )


#linking
TARGET_LINK_LIBRARIES(direct_binding_tcp_show  irdirectsdk
                                               ${OpenCV_LIBRARIES})
                                               
TARGET_LINK_LIBRARIES(direct_binding_usb_show  irdirectsdk
											   udev
											   ${OpenCV_LIBRARIES})
