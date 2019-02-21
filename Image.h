#pragma once



/**
 * @class Image
 * @brief Data container class for thermal and RGB images
 * @author Stefan May (Evocortex GmbH)
 * @date 14.9.2017
 */
template<class T>
class Image
{
public:
  /**
   * Constructor (unsigned short version)
   * @param[in] w width of image
   * @param[in] h height of image
   * @param[in] data image data
   */
  Image(unsigned int w, unsigned int h, unsigned short* data)
  {
    _width  = w;
    _height = h;
    _data = new unsigned short[w*h];
    memcpy(_data, data, w*h*sizeof(unsigned short));
  }
  /**
   * Constructor (unsigned char version for YUYV images, 2 bytes per pixel)
   * @param[in] w width of image
   * @param[in] h height of image
   * @param[in] data image data
   */
  Image(unsigned int w, unsigned int h, unsigned char* data)
  {
    _width  = w;
    _height = h;
    _data = new unsigned char[2*w*h];
    memcpy(_data, data, 2*w*h*sizeof(unsigned char));
  }
  /**
   * Destructor
   */
  ~Image()
  {
    if(_data)
    {
      delete [] _data;
      _data = NULL;
    }
  }
public:
  unsigned int _width;
  unsigned int _height;
  T*           _data;
};
