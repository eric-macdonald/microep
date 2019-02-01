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

Rect Rec(2, 0, 640, 480);
std::chrono::system_clock::time_point firsttime;
std::chrono::system_clock::time_point frametime;
int elapsed_time;

int main(int argc, char *argv[])
{
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    if(!((argc==2)||(argc==4))) {
      cout << "Usage: readRaw <video_filebase> <scale> <offset>" << endl << endl;
      cout << "   For a directory full of raw and time files, " << endl;
      cout << "   use the filename base as the argument.      " << endl;
      cout << "   An example is for ir_14060049_20190124_183213.time and" << endl;
      cout << "   ir_14060049_20190124_183213.raw: " << endl << endl;
      cout << "   Invoke:                          " << endl;
      cout << "   readRaw ir_14060049_20190124_183213 " << endl << endl << endl;
      cout << "   This will work for any number of files with integer suffixes " << endl << endl << endl;
      return -1;
      }
 
    float scaler = 0.33;
    float mult   = -150;
    if(argc==4) 
             {
             float scaler = stof(argv[2]);
             float mult   = stof(argv[3]);
             cout << "reading scaler and mult factor " << std::endl;
             }
     

      int              w = 642;
      int              h = 480;
      int              outw = 640;
      int              outh = 480;


      std::string      line;
      std::string      timeline;
      std::string      timestring;
      std::string      inputstring = argv[1];
      unsigned short   value;
      unsigned short   value_fake;
      unsigned short   old_value = 0;
      unsigned char    lsb;
      unsigned char    msb;
      int              base_seconds;
      int              long_frame_count = 0;
      int              frame_start;
      int              frame_end;
      int              frame_size = w * h;
   
      std::streampos   fileSize;
      int              fileindex = 1;
      char           * file_buffer;
      unsigned short * frame_buffer;
      frame_buffer = new unsigned short [frame_size];


///////////////////////////////////////////////////////////////////
      regex r("\\s*ir_(\\d+)_(\\d{4})(\\d{2})(\\d{2})_(\\d{2})(\\d{2})(\\d{2}).*"); 
      smatch m;
      if(regex_match(inputstring,m,r)) 
          {
            std::string ts = m[2] + "" + m[3] + "" + m[4] + "T" + m[5] + "" + m[6] + "" + m[7];
            firsttime =  makeTimePoint (stoi(m[2]), stoi(m[3]), stoi(m[4]), stoi(m[5]), stoi(m[6]), stoi(m[7]));
            cerr << "First timestamp from file name is " << asString(firsttime) << endl;
          } 
            else 
          {
            cerr << "File format does not contain the timestamp " <<  endl;
            return -1;
          }

///////////////////////////////////////////////////////////////////
      VideoWriter writer;
      int codec = CV_FOURCC('M', 'J', 'P', 'G'); 
      double fps = 4.0;                          
      bool   isColor = true;
      string videofile = inputstring + ".avi";            
      writer.open(videofile, codec, fps, cv::Size(outw,outh), isColor);

      if (!writer.isOpened()) 
            {
            cerr << "Could not open the output video file for write\n";
            return -1;
            }
      else 
            {
            cerr << "Opened output video file for write\n";
            } 

///////////////////////////////////////////////////////////////////
      ofstream featuresfile;
      featuresfile.open(inputstring + ".txt");

      if (!featuresfile.is_open()) 
            {
            cerr << "Could not open text feature file for write\n";
            return -1;
            }
      else 
            {
            cerr << "Opened feature file for write\n";
            }

///////////////////////////////////////////////////////////////////
      std::string    timefile_string = inputstring + std::string(".time");
      std::ifstream  timefile(timefile_string);
      if (!timefile.is_open()) 
            {
            cerr << "Could not open first time file for read\n";
            return -1;
            }
      else 
            {
            cerr << "Opened first time file" << timefile_string << endl ;
            }
 
///////////////////////////////////////////////////////////////////
      std::string    rawfile_string = inputstring  + std::string(".raw");
      std::ifstream  rawfile (rawfile_string, ios::in|ios::binary|ios::ate);
      if (!rawfile.is_open()) 
            {
            cerr << "Could not open first raw feature file\n";
            return -1;
            }
      else 
            {
            cerr << "Opened raw file" << rawfile_string << endl;
            }


///////////////////////////////////////////////////////////////////
      std::getline(timefile, timeline);
      timestring = timeline.c_str();
      regex r2("^\\s*(\\d+)\\.(\\d{3}).*"); 
      smatch m2;
      if(regex_match(timestring,m2,r2)) 
          {
          base_seconds = stoi(m2[1]);
          cerr << "Base seconds in first frame time file " <<  base_seconds << endl;   
          }
      else 
          {
          cerr << "Error reading base seconds" << endl;
          return -1;
          } 


///////////////////////////////////////////////////////////////////
      while(1) 
          {
          int number_of_frames = 0;
          std::string line;
          rawfile.seekg(0, std::ios::end);
          fileSize = rawfile.tellg();
          rawfile.seekg(0, std::ios::beg);

          cout << endl << endl << "***********************************" << endl;
          cout << to_string(fileindex) << " time file = " << timefile_string << endl;
          cout << to_string(fileindex) << " raw file  = " << rawfile_string << endl ;
          cout << "Timestamp " << asString(frametime) << endl;
          cout << "file size = " << fileSize << endl ;

          file_buffer = new char [fileSize];
          rawfile.read (file_buffer, fileSize);
          int i = 0;
          while (std::getline(timefile, timeline))
              {
              string timestring = timeline.c_str();
              regex r("^\\s*(\\d+)\\.(\\d{3}).*"); 
              smatch m;
              if(regex_match(timestring,m,r)) 
                  {
                  elapsed_time = stoi(m[1]) - base_seconds;
                  frametime = firsttime + seconds(elapsed_time);
                  }
              else 
                  {
                  cout << "Error reading time file " << endl;
                  return -1;
                  }
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
              int minval = 900;
              int maxval = 0;
              for(int k = frame_start; k < frame_end; k = k + 2)
                  {
                       
                  lsb = (unsigned char)(file_buffer[k]);
                  msb = (unsigned char)(file_buffer[k+1]);
                  value = (unsigned short)msb*256 + (unsigned short)lsb;
                  //cout << "Raw = " << std::hex << (unsigned short)msb << "  " << (unsigned short)lsb << std::dec << "  " << value;   
                  value = (value-1000)/10;
//                  if(value < 0)
//                      {
//                      value = old_value;
//                      //std::cout << " below zero " ;
//                    }
                  if(value > 900)
                      {
                      value_fake = old_value;
                      //std::cout << " above 900 ";
                      }
                  old_value = value;
                  //cout << " convert to " << value << " row " << z%642 << " col " << z/642 << std::endl; 
                  if(minval > (unsigned short)value)
                      {
                      minval = (unsigned short)value;
                      }
                  if(maxval < (unsigned short)value)
                      {
                      maxval = (unsigned short)value;
                      }
                  frame_buffer[z] = value;
                  z++;
                  }

              cv::Mat cv_img(cv::Size(w, h), CV_16UC1, &frame_buffer[0], cv::Mat::AUTO_STEP);
              cv::Mat clean = cv_img(Rec);
              cv::Size scv_img = cv_img.size();
              cv::Size sclean = clean.size();
              double high,low;
              cv::Point wherehigh, wherelow;
              cv::minMaxLoc(clean,&high,&low,&wherehigh,&wherelow);
              cv::Mat cv_img2;
              cv::Mat cv_img3;
              cv::Mat color;
              clean.convertTo(cv_img2, CV_8UC1);
              //clean.convertTo(cv_img2, CV_8UC1, 0.33, -150);
              clean.convertTo(cv_img2, CV_8UC1, scaler, mult);
              //cv::equalizeHist(cv_img2, cv_img3);
              applyColorMap(cv_img2, color, cv::COLORMAP_JET);
              string timetext;
              //left if we want an average of an roi 
              //cv::Rect roi( roiVertexXCoordinate, roiVertexYCoordinate, roiWidth, roiHeight );
              //cv::Mat image_roi = inputImage( roi );
              cv::Scalar avgPixelIntensity = cv::mean( cv_img );
              float average_pixel = avgPixelIntensity.val[0];
              timetext = "Frame " + std::to_string(long_frame_count) + " " + asString(frametime) + " average = " + std::to_string(average_pixel);
              std::cout << "Frame " << long_frame_count << " stats: " << high << " " << low << " " << wherehigh << " " << wherelow << " " << average_pixel << std::endl;   
              featuresfile <<  asString(frametime) << long_frame_count << " stats: " << high << " " << low << " " << wherehigh << " " << wherelow << " " << average_pixel << std::endl;
              putText(color, timetext, Point(25, 25), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
            
              cv::imshow("image", color);
              cv::waitKey(1);
              cv::Size scv_img2 = cv_img2.size();
              cv::Size scv_img3 = cv_img3.size();
              writer.write(color);
              long_frame_count = long_frame_count + 1;
              i = i + 1;
              }
          rawfile.close();
          timefile.close();

          delete[] file_buffer;

///////////////////////////////////////////////////////////////////
          fileindex = fileindex + 1;
          timefile_string = inputstring + std::string(".time.") + std::to_string(fileindex);
          timefile.open(timefile_string);
          if (!timefile.is_open()) 
                {
                cout << "Could not open subsequent time file for read " << timefile_string << endl; 
                high_resolution_clock::time_point t2 = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>( t2 - t1 ).count();
                std::cout << long_frame_count << " frames required " << duration/1000000.f << " seconds to process." << std::endl;
                cout << "Created text file with all extracted features" << endl;
                cout << "Deleting all buffers and files" << endl;
                return -1;
                }
          else 
                {
                cout << "Opened subsequent time file "  << timefile_string << endl;
                }
     
///////////////////////////////////////////////////////////////////////
          rawfile_string = inputstring + std::string(".raw.") + std::to_string(fileindex);
          rawfile.open(rawfile_string, ios::in|ios::binary|ios::ate);
          if (!rawfile.is_open()) 
                {
                cout << "Next subsequent raw file does not exist so completing video" << rawfile_string << endl;
                return -1;
                }
          else 
                {
                cout << "Opened subsequent raw file " << rawfile_string << endl << endl;
                }
          }
     high_resolution_clock::time_point t2 = high_resolution_clock::now();
     auto duration = duration_cast<microseconds>( t2 - t1 ).count();
     std::cout << long_frame_count << "frames required " << duration << "to process." << std::endl;
     cout << "Created text file with all extracted features" << endl;
     cout << "Deleting all buffers and files" << endl;



     delete[] frame_buffer;
     featuresfile.close();
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
//              featuresfile << cv_img.at<short>(240,240) <<"\t"<< ((unsigned int)cv_img2.at<char>(240,240)&0xFF) <<"\t"<< ((unsigned int)cv_img3.at<char>(240,240)&0xFF) <<"\t " << ((unsigned int)color.at<char>(240,240)&0xFF) << "\t" << minval << "\t" << maxval <<"\t"<< long_frame_count << "\t " << asString(frametime) << endl;
