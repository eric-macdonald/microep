#include <stdio.h>
#include <string.h>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <fstream>
#include <vector>
#include <list>
#include <signal.h>

#include <opencv2/opencv.hpp>
//#include <libirimager/direct_binding.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utility.hpp>
//#include "opencv2/videoio.hpp"
//#include "opencv2/imgcodecs.hpp"

#include <cstdio>
#include <algorithm>

// Optris device interfaces
#include "IRDevice.h"

// Optris imager interfaces
#include "IRImager.h"

// Optris logging interface
#include "IRLogger.h"

// Optris raw image file writer
#include "IRFileWriter.h"

using namespace std;
using namespace evo;

int rows = 480;
int cols = 642;
int totalsize = rows*cols;
bool _keepCapturing = true;

void sigHandler(int dummy=0)
{
    _keepCapturing = false;
}

int main (int argc, char* argv[])
{
  if(argc!=2)
  {
    cout << "usage: " << argv[0] << " <xml configuration file>" << endl;
    return -1;
  }

  signal(SIGINT, sigHandler);

  IRLogger::setVerbosity(IRLOG_ERROR, IRLOG_OFF);
  IRDeviceParams params;
  IRDeviceParamsReader::readXML(argv[1], params);
  IRDevice* dev = NULL;

  dev = IRDevice::IRCreateDevice(params);

  if(dev)
  {
    /**
     * Initialize Optris image processing chain
     */
    IRImager imager;
    if(imager.init(&params, dev->getFrequency(), dev->getWidth(), dev->getHeight(), dev->controlledViaHID()))
    {
      if(imager.getWidth()!=0 && imager.getHeight()!=0)
      {
        cout << "Thermal channel: " << imager.getWidth() << "x" << imager.getHeight() << "@" << imager.getMaxFramerate() << "Hz" << endl;
        int w = imager.getWidth();
        int h = imager.getHeight();

        // Start UVC streaming
        if(dev->startStreaming()==0)
        {
          // Enter loop in order to pass raw data to Optris image processing library.
          // Processed data are supported by the frame callback function.
          double timestamp;
          unsigned char* bufferRaw = new unsigned char[dev->getRawBufferSize()];
          RawdataHeader header;
          imager.initRawdataHeader(header);

          IRFileWriter writer(time(NULL), "/tmp", header);
          writer.open();

          char nmea[GPSBUFFERSIZE];
          memset(nmea, 0, GPSBUFFERSIZE*sizeof(*nmea));

          imager.forceFlagEvent(1000.0);
          int serializedImages = 0;
          int chunk = 1;
          unsigned short mins = -1;
          unsigned short maxs = 0;
          unsigned char minc = 255;
          unsigned char maxc = 0;
          float temp;
          while(_keepCapturing)
          {
            if(dev->getFrame(bufferRaw, &timestamp)==IRIMAGER_SUCCESS)
            {

              cout << "bufferRaw size = " << dev->getRawBufferSize() << endl;
              unsigned short *bR = new unsigned short[dev->getRawBufferSize()];
              unsigned char *temp2 = new unsigned char[308160];
              memcpy(bR, bufferRaw, dev->getRawBufferSize());
              for(int k = 0; k < totalsize; ++k)
                   {
                   cout << "bR short = " << bR[k] << endl;
                   if(bR[k] > maxs) maxs = bR[k];
                   if(bR[k] < mins) mins = bR[k];
                   temp = ((((float)bR[k])-58000.0))/3.0;
                   temp2[k] = (char)temp;
                   }
              for(int k = 0; k < totalsize; ++k)
                   {
                   cout << "temperature = " << int(temp2[k]) << endl;
                   if(temp2[k] > maxc) maxc = temp2[k];
                   if(temp2[k] < minc) minc = temp2[k];
                   }
              cout << "temperature conversion done " << endl;
              cout << "temperature conversion done max short " << maxs << endl;
              cout << "temperature conversion done min short " << mins << endl;
              cout << "temperature conversion done max char  " << int(maxc) << endl;
              cout << "temperature conversion done max char  " << int(minc) << endl;
              cv::Mat src1(rows, cols, CV_8UC1, &temp2[0]);
              cv::imshow("gray", src1);
              cv::waitKey(5000);
              cv::imwrite("gray.png",src1);
              cv::Mat src2(rows, cols, CV_8UC3, cv::Scalar(0,0,0));
	      cv::applyColorMap(src1, src2, cv::COLORMAP_JET);
              cv::imshow("palette image daemon", src2);
              cv::waitKey(5000);
              cv::imwrite("devdepth.png",src2);

              imager.process(bufferRaw, NULL);

              if(writer.canDoWriteOperations())
                    {
                    cout << "bufferRaw from imager.process = " << bufferRaw[0] << " " << bufferRaw[1] << endl;
                    writer.write(timestamp, bufferRaw, chunk, dev->getRawBufferSize(), nmea);
                    cout << "nmea = " << nmea << endl;
                    cout << "size of nmea = " << sizeof(nmea) << endl;
                    
                    }
              // In order to avoid too large files, the user can split the records into chunks of equal size.
              // Here, a fixed number of images should be recorded to each chunk.
              return(0);
              if((++serializedImages)%1000==0)
              {
                chunk++;
              }
            }
          }
          delete [] bufferRaw;
        }
        else
        {
          cout << "Error occurred in starting stream ... aborting. You may need to reconnect the camera." << endl;
        }
      }
    }
    else
    {
      cout << "Error: Image streams not available or wrongly configured. Check connection of camera and config file." << endl;
    }
    delete dev;
  }

  return 0;
}
