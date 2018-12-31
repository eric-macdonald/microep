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

// evo Frame Rate Counter
#include "FramerateCounter.h"

using namespace std;
using namespace evo;

bool _keepCapturing = true;
double _elapsed            = 0.0;
int    _cntElapsed         = 0;
int    _cntFrames          = 0;
FramerateCounter _fpsCounter(1000.0, 250);
FramerateCounter _fpsCounterRaw(1000.0, 250);

void sigHandler(int dummy=0)
{
    _keepCapturing = false;
}

void onThermalFrame(unsigned short* thermal, unsigned int w, unsigned int h, IRFrameMetadata meta, void* arg)
{
  // output of current frame rate
  if(_fpsCounter.trigger())
  {
    cout << "Current Thermal FPS: " << _fpsCounter.getFps() << endl;
  }
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
  IRDevice* dev = IRDevice::IRCreateDevice(params);

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

        if(!dev)
        {
          cout << "NO DEV" << endl;
          exit(1);
        }

        // Start UVC streaming
        if(dev->isOpen())
        {
          // Enter loop in order to pass raw data to Optris image processing library.
          // Processed data are supported by the frame callback function.
          double timestamp;
          unsigned char* bufferRaw = new unsigned char[dev->getRawBufferSize()];
          RawdataHeader header;
          imager.initRawdataHeader(header);

          char nmea[GPSBUFFERSIZE];
          memset(nmea, 0, GPSBUFFERSIZE*sizeof(*nmea));

          imager.forceFlagEvent(1000.0);
          imager.setThermalFrameCallback(onThermalFrame);
          dev->startStreaming();

          while(_keepCapturing)
          {
            dev->getFrame(bufferRaw);
            imager.process(bufferRaw, NULL);
            // output of current raw frame rate
            if(_fpsCounterRaw.trigger())
            {
              cout << "Current Raw FPS: " << _fpsCounterRaw.getFps() << endl;
            }
          }
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
