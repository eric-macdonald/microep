# microep

#to build 
cmake .
make 


#Calibration files with serial number from bottom of camera
#ir_find_camera will find attached cameras and provide the serial number
#ir_generate_calibration will create one 
14060049.xml
Cali-14060049.xml
Cali-18072089.xml

#executables
checkRawFramerate       #provides frame rate
convertRaw2PPM          #should convert raw to readable but deprecated in 4.0
direct_binding_tcp_show #connect to camera through ethernet
direct_binding_usb_show #connect to camera through usb 2.0
readRaw                 #YSU developed program to read a raw file
serializeRaw            #program to generate a raw file from the camera
serializeYSU            #YSU generate of raw file with extra debug to understand raw files
