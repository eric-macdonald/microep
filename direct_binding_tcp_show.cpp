#include <iostream>
#include <memory>
#include <unistd.h>

//--Code for displaying image -----------------
#include <opencv2/opencv.hpp>

#include "libirimager/direct_binding.h"
//---------------------------------------------



int main(int argc, char *argv[])
{
  bool hasLaunchedDaemon = false;
  char default_arg[] = "localhost";
  char* arg = default_arg;
  if(argc==2)
    arg = argv[1];
  else
  {
    if(::evo_irimager_daemon_is_running()!=0)
    {
        evo_irimager_daemon_launch();
        hasLaunchedDaemon = true;
    }
    // Wait some time for server
    usleep(2000000);
  }

  int ret = ::evo_irimager_tcp_init(arg, 1337);

  if(ret < 0)
  {
    std::cout << "error at init" << std::endl;
    ::exit(-1);
  }

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

  if(hasLaunchedDaemon) ::evo_irimager_daemon_kill();

  return 0;
}
