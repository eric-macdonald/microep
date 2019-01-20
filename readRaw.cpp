#include <iostream>
#include <fstream>
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
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
      int              w = 642;
      int              h = 480;
      short            value;
      int              long_frame_count = 0;
      int              frame_start;
      int              frame_end;
      int              frame_size = w * h;
      char           * file_buffer;
      unsigned short * frame_buffer;
      frame_buffer = new unsigned short [frame_size];
      unsigned short   kvalue;

      ofstream featuresfile;
      featuresfile.open("./featuresfile.txt");

      VideoWriter writer;
      int codec = CV_FOURCC('M', 'J', 'P', 'G'); 
      double fps = 32.0;                          
      bool   isColor = true;
      string videofile = "./output.avi";            
      writer.open(videofile, codec, fps, cv::Size(w,h), isColor);

      if (!writer.isOpened()) 
          {
          cerr << "Could not open the output video file for write\n";
          return -1;
          }
      else 
          {
          cerr << "Opened output video file for write\n";
          } 

      std::string line;
      std::ifstream filelist(argv[1]);
      if (filelist.is_open()) {

           while (getline(filelist, line)) {
                 printf("%s", line.c_str());
               
                 std::string suffixraw = ".raw";
                 std::string suffixtime = ".time";
                 std::string fileraw  = line.c_str() + suffixraw;
                 std::string filetime = line.c_str() + suffixtime;
                 std::string timestamp = line.c_str();
                 cout << "***********************************" << endl;
                 cout << "input time file name =  " << filetime << endl;
                 cout << "input raw file name =   " << fileraw << endl ;
               
                 int number_of_frames = 0;
                 std::string line;
                 std::ifstream timefile(filetime);
               
///////////////// open the raw file
                 long size;
                 ifstream rawfile (fileraw, ios::in|ios::binary|ios::ate);

///////////////// determine raw file size for one buffer capture of all 
                 size = rawfile.tellg();
                 rawfile.seekg (0, ios::beg);
                 cout << "file size = " << size << endl << endl << endl;

                 file_buffer = new char [size];
                 cout << "new file_buffer of size " << size << endl;
                 rawfile.read (file_buffer, size);
                 int i = 0;
                 while (std::getline(timefile, line))
                      {
                      cout << "frame = " << long_frame_count << endl;
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
                      int z = 0;
                      int minval = 10000;
                      int maxval = 0;
                      for(int k = frame_start; k < frame_end; k = k + 2)
                         {
                         value = (((short(file_buffer[k+1])*256 + short(file_buffer[k]))-1000)/50);
                         if(value > 3000)
                               value = 90;
                         if(minval > value)
                               minval = value;
                         if(maxval < value)
                               maxval = value;
                         frame_buffer[z] = value;
                         z++;
                         }
                      cv::Mat cv_img(cv::Size(w, h), CV_16UC1, &frame_buffer[0], cv::Mat::AUTO_STEP);
                      cv::Mat cv_img2;
                      featuresfile << cv_img.at<short>(240,240) <<","<<minval<<","<<maxval<<","<<long_frame_count<<", " << timestamp << "," << line << endl; 
                      cv::Mat color;
                      cv_img.convertTo(cv_img, CV_8UC1);
                      cv::equalizeHist(cv_img, cv_img2);
                      applyColorMap(cv_img, color, cv::COLORMAP_JET);
              
//////////            cout << "cv_img r, c = " << cv_img2.rows << " " << cv_img2.cols <<endl;
//////////            cout << "cv_img channels and size =" << cv_img2.channels() << " " << cv_img2.size() <<endl;
//////////            cout << "Matrix 3 type = " << cv_img2.type() << endl;
               
                      cv::imshow("image", color);
                      cv::waitKey(1);
                      writer.write(color);
                      long_frame_count = long_frame_count + 1;
                      i = i + 1;
                      }
                 rawfile.close();
                 timefile.close();

                 cout << "about to delete file buffer " << endl;
                 delete[] file_buffer;
                 cout << "deleted file buffer " << endl;
                 }
        }
     cout << "started to delete with frame_buffer" << endl;
     delete[] frame_buffer;
     cout << "ended deletion of frame_buffer" << endl;
     featuresfile.close();
     filelist.close();
     return 0;
}
