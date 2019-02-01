#include <stdio.h>
#include <string.h>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <fstream>
#include <vector>
#include <list>
#include <signal.h>

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

bool _keepCapturing = true;

void sigHandler(int dummy=0)
{
    _keepCapturing = false;
}

int main (int argc, char* argv[])
{
  if(argc!=3)
  {
    cout << "usage: " << argv[0] << " <xml configuration file> <save directory>" << endl;
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

        // Start UVC streaming
        if(dev->startStreaming()==0)
        {
          // Enter loop in order to pass raw data to Optris image processing library.
          // Processed data are supported by the frame callback function.
          double timestamp;
          unsigned char* bufferRaw = new unsigned char[dev->getRawBufferSize()];
          RawdataHeader header;
          imager.initRawdataHeader(header);

          IRFileWriter writer(time(NULL), argv[2], header);
          writer.open();

          char nmea[GPSBUFFERSIZE];
          memset(nmea, 0, GPSBUFFERSIZE*sizeof(*nmea));

          imager.forceFlagEvent(1000.0);
          int serializedImages = 0;
          int chunk = 1;
          while(_keepCapturing)
          {
            if(dev->getFrame(bufferRaw, &timestamp)==IRIMAGER_SUCCESS)
            {
              imager.process(bufferRaw, NULL);

              if(writer.canDoWriteOperations())
                writer.write(timestamp, bufferRaw, chunk, dev->getRawBufferSize(), nmea);
              std::cout << "frames = " << serializedImages << std::endl; 
              // In order to avoid too large files, the user can split the records into chunks of equal size.
              // Here, a fixed number of images should be recorded to each chunk.
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
