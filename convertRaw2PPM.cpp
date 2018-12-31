#include <stdio.h>
#include <string.h>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <fstream>
#include <sys/stat.h>
#include <stdlib.h>
#include <iomanip>
#include <unistd.h>

#include "IRFileReader.h"

/**
 * Optris logging interface
 */
#include "IRLogger.h"

/**
 * Visualization
 */
#include "ImageBuilder.h"

/**
 * Helper class (frame rate calculator)
 */
#include "FramerateCounter.h"

#define SERIALIZE_COLOR_BAR 0
#define PRINT_THERMAL_BOUNDS 0

using namespace std;
using namespace evo;

char*            _fileIn = NULL;
unsigned char*   _image  = NULL;
unsigned int     _cnt    = 0;

void onFlagStateChange(EnumFlagState fs, void* arg)
{
  if(fs==irFlagClosing)
    cout << endl << "Flag is closing -> image freezes" << endl;
  else if(fs==irFlagOpen)
    cout << endl << "Flag is open    -> image defreezes" << endl;
}

// Function called within process call of IRImager instance with thermal image as parameter.
// Keep this function free of heavy processing load. Otherwise the frame rate will drop down significantly for the control loop.
void onThermalFrame(unsigned short* thermal, unsigned int w, unsigned int h, IRFrameMetadata meta, void* arg)
{
  // output file name = input filename with appendix {counter} and ending "ppm"
  int len = strlen(_fileIn);
  char* fileOut = new char[len+7];
  memcpy(fileOut, _fileIn, len);
  sprintf(&(fileOut[len-4]), "_%05d.ppm", _cnt);
  fileOut[len+6] = '\0';

  // configuration of image conversion
  ImageBuilder iBuilder;
  iBuilder.setPaletteScalingMethod(eSigma1);
  iBuilder.setData(w, h, thermal);
  if(_image==NULL)
    _image = new unsigned char[iBuilder.getStride() * h * 3];
  iBuilder.convertTemperatureToPaletteImage(_image, true);
  iBuilder.serializePPM(fileOut, _image, w, h);
  delete [] fileOut;

  // Additionally, one can serialize the concrete color bar
#if SERIALIZE_COLOR_BAR
  char* fileBar = new char[len+11];
  memcpy(fileBar, _fileIn, len);
  sprintf(&(fileBar[len-4]), "%05d_bar.ppm", _cnt);
  unsigned char* bar;
  iBuilder.getPaletteBar(30, h, bar);
  iBuilder.serializePPM(fileBar, bar, 30, h);
  delete [] fileBar;
  delete [] bar;
#endif

  // If the user needs to know the temperature bounds, the following commands can be used to output those values.
#if PRINT_THERMAL_BOUNDS
  cout << "conversion temperature range: " << setprecision(1) << fixed << iBuilder.getIsothermalMin() << " " << iBuilder.getIsothermalMax() << endl;
#endif

  cout << "." << flush;
  _cnt++;
}

int main (int argc, char* argv[])
{
  if(argc<3)
  {
    cout << "usage: " << argv[0] << " <xmlConfig> <filename>" << endl;
    return 1;
  }

  IRLogger::setVerbosity(IRLOG_ERROR, IRLOG_OFF);

  char* xmlConfig = argv[1];
  _fileIn         = argv[2];

  // We need to provide camera independent parameters, e.g., calibration paths
  IRDeviceParams params;
  IRDeviceParamsReader::readXML(xmlConfig, params);

  IRFileReader reader(_fileIn, params);
  reader.setThermalFrameCallback(onThermalFrame);
  reader.setFlagStateCallback(onFlagStateChange);
  if(reader.isReady())
  {
    FramerateCounter fr;
    while(reader.nextFrame());
  }

  return 0;
}

