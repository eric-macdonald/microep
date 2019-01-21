#include <iostream>
#include <typeinfo>
#include <regex>
#include <type_traits>
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
#include <chrono>
#include <ctime>
#include <sys/time.h>
#include <opencv2/opencv.hpp>
                       

using namespace std;
using namespace cv;
using namespace std::chrono;

std::chrono::system_clock::time_point makeTimePoint (int year, int mon, int day, int hour, int min, int sec=0);
std::string asString (const std::chrono::system_clock::time_point& tp);

std::chrono::system_clock::time_point frametime;

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
      std::string timeline;
      std::ifstream filelist(argv[1]);
      if (filelist.is_open()) {

           while (getline(filelist, line)) {
                 string timestring = line.c_str();
                 regex r("\\s*ir_(\\d+)_(\\d{4})(\\d{2})(\\d{2})_(\\d{2})(\\d{2})(\\d{2}).*"); 
                 smatch m;
                 if(regex_match(timestring,m,r)) 
                     {
                       std::string ts = m[2] + "" + m[3] + "" + m[4] + "T" + m[5] + "" + m[6] + "" + m[7];
                       frametime =  makeTimePoint (stoi(m[2]), stoi(m[3]), stoi(m[4]), stoi(m[5]), stoi(m[6]), stoi(m[7]));
                    } 
                       else 
                     {
                       cout << "File list is corrupted " <<  endl;
                       return -1;
                     }
                 std::string suffixraw = ".raw";
                 std::string suffixtime = ".time";
                 std::string fileraw  = line.c_str() + suffixraw;
                 std::string filetime = line.c_str() + suffixtime;
                 std::string timestamp = line.c_str();
               
                 int number_of_frames = 0;
                 std::string line;
                 std::ifstream timefile(filetime);
               
                 long size;
                 ifstream rawfile (fileraw, ios::in|ios::binary|ios::ate);

                 size = rawfile.tellg();
                 rawfile.seekg (0, ios::beg);

                 cout << endl << endl << "***********************************" << endl;
                 cout << "input time file name =  " << filetime << endl;
                 cout << "input raw file name =   " << fileraw << endl ;
                 cout << "Timestamp " << asString(frametime) << endl;
                 cout << "file size = " << size << endl ;

                 file_buffer = new char [size];
                 rawfile.read (file_buffer, size);
                 int i = 0;
                 while (std::getline(timefile, timeline))
                      {
                      string timestring = timeline.c_str();
                      regex r("^\\s*(\\d+)\\.(\\d{3}).*"); 
                      smatch m;
                      if(regex_match(timestring,m,r)) 
                          {
                          cout << m[1] << "." << m[2] << endl;   
                          }
                      else 
                         cout << "Error reading time file " << endl;

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
                      string timetext;
                      timetext = "Frame " + std::to_string(long_frame_count) + " " + asString(frametime);
                      putText(color, timetext, Point(25, 25), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1);
              
                      cv::imshow("image", color);
                      cv::waitKey(1);
                      writer.write(color);
                      long_frame_count = long_frame_count + 1;
                      i = i + 1;
                      }
                 rawfile.close();
                 timefile.close();

                 delete[] file_buffer;
                 }
        }
     cout << "Created csv file with all extracted features" << endl;
     cout << "Deleting all buffers and files" << endl;
     delete[] frame_buffer;
     featuresfile.close();
     filelist.close();
     return 0;
}


std::string asString (const std::chrono::system_clock::time_point& tp)
{
   // convert to system time:
   std::time_t t = std::chrono::system_clock::to_time_t(tp);
   std::string ts = ctime(&t);   // convert to calendar time
   ts.resize(ts.size()-1);       // skip trailing newline
   return ts;
}

std::chrono::system_clock::time_point makeTimePoint (int year, int mon, int day, int hour, int min, int sec)
{
   struct std::tm t;
   t.tm_sec = sec;        // second of minute (0 .. 59 and 60 for leap seconds)
   t.tm_min = min;        // minute of hour (0 .. 59)
   t.tm_hour = hour;      // hour of day (0 .. 23)
   t.tm_mday = day;       // day of month (0 .. 31)
   t.tm_mon = mon-1;      // month of year (0 .. 11)
   t.tm_year = year-1900; // year since 1900
   t.tm_isdst = -1;       // determine whether daylight saving time
   std::time_t tt = std::mktime(&t);
   if (tt == -1) {
       throw "no valid system time";
   }
   return std::chrono::system_clock::from_time_t(tt);
}
