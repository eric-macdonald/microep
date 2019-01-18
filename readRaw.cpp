#include <iostream>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/imgcodecs.hpp"
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <algorithm>
#include <string.h>
#include <sys/time.h>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
  VideoCapture cap;
  int w = 642;
  int h = 480;
  int frame_start;
  int frame_end;
  int frame_size = w * h;
  char * buffer1;
  char * buffer2;
  short value;
  std::string suffix1 = ".jpg";
  std::string suffix2 = ".raw";
  std::string suffix3 = ".time";
  std::string fileout = argv[1] + suffix1;
  std::string filein1 = argv[1] + suffix3;
  std::string filein2 = argv[1] + suffix2;
  cout << "output file name = " << fileout << endl;
  cout << "input time file name = " << filein1 << endl;
  cout << "input raw file name = " << filein2 << endl;

  VideoWriter writer;
  int codec = CV_FOURCC('M', 'J', 'P', 'G'); 
  double fps = 30.0;                          
  bool   isColor = true;

  string filename = "./output.avi";            
  writer.open(filename, codec, fps, cv::Size(w,h), isColor);
  if (!writer.isOpened()) 
      {
      cerr << "Could not open the output video file for write\n";
      return -1;
      }
  else 
      {
      cerr << "Open the output video file for write\n";
      } 


// read the time file
  int number_of_frames = 0;
  std::string line;
  std::ifstream myfile(filein1);

  while (std::getline(myfile, line))
     ++number_of_frames;
  std::cout << "Number of lines in time file: " << number_of_frames << endl;

// open the raw file
  long size;
  ifstream file (filein2, ios::in|ios::binary|ios::ate);

// determine raw file size for one buffer capture of all 
  size = file.tellg();
  cout << "file size = " << size << endl;
  file.seekg (0, ios::beg);
//  short * line[642];
  buffer1 = new char [size];
  buffer2 = new char [frame_size];
  file.read (buffer1, size);

  for(int i = 0; i < number_of_frames; i++)
       {
       cout << "frame = " << i << endl;
       if(i == 0)
          {
          frame_start = 48; 
          frame_end = ((2*frame_size) + 48); 
          }
       else      
          {
          frame_start = ((2*i*frame_size) + 48 + (i) * 80); 
          frame_end = ((2*(i+1)*frame_size) + 48 + (i+1) * 80); 
          }
       cout << "frame byte start = " << frame_start << endl; 
       cout << "frame byte end = " << frame_end << endl; 
       int z = 0;
       for(int k = frame_start; k < frame_end; k = k + 2)
          {
          value = (((short(buffer1[k+1])*256 + short(buffer1[k]))-1000)/50);
          buffer2[z] = value;
          //cout << "buffer2 = " << buffer2[z] << " at address " << z << endl;
//          if(short(buffer1[k+1]) == 0)
//              {
//              cout << "new frame? = " << value << " at address " << k << endl;
//              }
          z++;
          }
       cv::Mat cv_img(cv::Size(w, h), CV_8UC1, &buffer2[0], cv::Mat::AUTO_STEP);
       cv::Mat color;
       applyColorMap(cv_img, color, cv::COLORMAP_JET);
       cv::imshow("image", color);
       cv::waitKey(0);
       writer.write(color);
       // cv::imwrite(fileout, color);
       }
  file.close();
  cout << "the complete file is in a buffer";
  delete[] buffer1;
  delete[] buffer2;
  return 0;
}
