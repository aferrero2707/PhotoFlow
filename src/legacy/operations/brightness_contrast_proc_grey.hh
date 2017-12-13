template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
class BrightnessContrastProc<T,PF_COLORSPACE_GRAYSCALE,CHMIN,CHMAX,PREVIEW,OP_PAR>
  {
    BrightnessContrastPar* par;
  public:
    BrightnessContrastProc(BrightnessContrastPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T*& pout)
    {
      typename FormatInfo<T>::SIGNED val = (typename FormatInfo<T>::SIGNED)p[first][x] - FormatInfo<T>::HALF;
      typename FormatInfo<T>::SIGNED newval = 
	(intensity*par->get_contrast()+1.0)*val + intensity*par->get_brightness()*FormatInfo<T>::RANGE + FormatInfo<T>::HALF;
      clip(newval,pout[x]);
    }
  };


