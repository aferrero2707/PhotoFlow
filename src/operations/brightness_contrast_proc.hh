  template < typename T > 
  void clip(const typename FormatInfo<T>::PROMOTED& val, T& clipped)
  {
    if(val > FormatInfo<T>::MAX) clipped = FormatInfo<T>::MAX;
    else if(val < FormatInfo<T>::MIN) clipped = FormatInfo<T>::MIN;
    else clipped = (T)val;
    //std::cout<<"val="<<(int)val<<"  max="<<(int)FormatInfo<T>::MAX<<"  min="<<(int)FormatInfo<T>::MIN<<"  clipped="<<(int)clipped<<std::endl;
  }



  template < typename T, colorspace_t CS, bool PREVIEW, class OP_PAR >
  class BrightnessContrastProc
  {
    BrightnessContrastPar* par;
  public:
    BrightnessContrastProc(BrightnessContrastPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T*& pout) {}
  };

  
#include "brightness_contrast_proc_grey.hh"
#include "brightness_contrast_proc_rgb.hh"
#include "brightness_contrast_proc_lab.hh"
