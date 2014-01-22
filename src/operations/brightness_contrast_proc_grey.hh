  template < typename T, bool PREVIEW, class OP_PAR >
  class BrightnessContrastProc<T,PF_COLORSPACE_GRAYSCALE,PREVIEW,OP_PAR>
  {
    BrightnessContrastPar* par;
  public:
    BrightnessContrastProc(BrightnessContrastPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T*& pout)
    {
      typename FormatInfo<T>::PROMOTED val = (typename FormatInfo<T>::PROMOTED)p[first][x] - FormatInfo<T>::HALF;
      typename FormatInfo<T>::PROMOTED newval = 
	(intensity*par->get_contrast()+1.0)*val + intensity*par->get_brightness()*FormatInfo<T>::RANGE + FormatInfo<T>::HALF;
      clip(newval,pout[x]);
      x += 1;
    }
  };


