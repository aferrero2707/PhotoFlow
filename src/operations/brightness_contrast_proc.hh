

template < typename T, colorspace_t CS, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
  class BrightnessContrastProc
  {
    BrightnessContrastPar* par;
  public:
    BrightnessContrastProc(BrightnessContrastPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T*& pout) {}
  };

  
#include "brightness_contrast_proc_grey.hh"
#include "brightness_contrast_proc_rgb.hh"
#include "brightness_contrast_proc_lab.hh"
