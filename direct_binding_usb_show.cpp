#include <iostream>
#include <memory>

//--Code for displaying image -----------------
#include <opencv2/opencv.hpp>

#include "libirimager/direct_binding.h"
//---------------------------------------------

int main(int argc, char *argv[])
{

  if(argc!=2)
  {
    std::cout << "usage: " << argv[0] << " <xml configuration file>" << std::endl;
    return -1;
  }

  if(::evo_irimager_usb_init(argv[1], 0, 0) != 0) return -1;

  int w;
  int h;
  if(::evo_irimager_get_palette_image_size(&w, &h)==0)
  {
    std::vector<unsigned char> data(w*h*3);
    do
    {
     if(::evo_irimager_get_palette_image(&w, &h, &data[0])==0)
     {
        //--Code for displaying image -----------------
        cv::Mat cv_img(cv::Size(w, h), CV_8UC3, &data[0], cv::Mat::AUTO_STEP);
        cv::cvtColor(cv_img, cv_img, CV_BGR2RGB);
        cv::imshow("palette image daemon", cv_img);
        cv::waitKey(5);
        //---------------------------------------------
     }
    } while(cvGetWindowHandle("palette image daemon"));
  }

  ::evo_irimager_terminate();

  return 0;
}
