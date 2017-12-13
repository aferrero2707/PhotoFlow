template < typename T, int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
class BrightnessContrastProc<T,PF_COLORSPACE_LAB,CHMIN,CHMAX,PREVIEW,OP_PAR>
  {
    BrightnessContrastPar* par;
  public:
    BrightnessContrastProc(BrightnessContrastPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, T*& pout)
    {
      typename FormatInfo<T>::SIGNED val = (typename FormatInfo<T>::SIGNED)p[first][x] - FormatInfo<T>::HALF;
      typename FormatInfo<T>::SIGNED newval = 
  (intensity*par->get_contrast()+1.0f)*val + intensity*par->get_brightness()*FormatInfo<T>::RANGE + FormatInfo<T>::HALF;
      clip(newval,pout[x]);
      pout[x+1] = p[first][x+1];
      pout[x+2] = p[first][x+2];
    }
  };


template < int CHMIN, int CHMAX, bool PREVIEW, class OP_PAR >
class BrightnessContrastProc<float,PF_COLORSPACE_LAB,CHMIN,CHMAX,PREVIEW,OP_PAR>
  {
    BrightnessContrastPar* par;
  public:
    BrightnessContrastProc(BrightnessContrastPar* p): par(p) {}

    void process(float**p, const int& n, const int& first, const int& nch, const int& x, const double& intensity, float*& pout)
    {
      float val = p[first][x] - 0.5f;
      float newval = (intensity*par->get_contrast()+1.0f)*val + intensity*par->get_brightness() + 0.5f;
      if( par->get_gamma() != 1 ) {
        newval = powf( newval, par->get_gamma() );
      }
      clip(newval,pout[x]);
    }
  };


