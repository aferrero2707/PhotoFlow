  template < typename T, bool PREVIEW, class OP_PAR >
  class BrightnessContrastProc<T,PF_COLORSPACE_RGB,PREVIEW,OP_PAR>
  {
    BrightnessContrastPar* par;
  public:
    BrightnessContrastProc(BrightnessContrastPar* p): par(p) {}

    void process(T**p, const int& n, const int& first, const int& nch, int& x, const double& intensity, T*& pout)
    {
      //return;
      //std::cout<<"p[first]: "<<(void*)p[first]<<std::endl;
      //std::cout<<"pout: "<<(void*)pout<<std::endl;
      typename FormatInfo<T>::PROMOTED val;
      typename FormatInfo<T>::PROMOTED newval;
      val = (typename FormatInfo<T>::PROMOTED)p[first][x] - FormatInfo<T>::HALF;
      newval = (intensity*par->get_contrast()+1.0f)*val + intensity*par->get_brightness()*FormatInfo<T>::RANGE + FormatInfo<T>::HALF;
      clip(newval,pout[x]);
      //std::cout<<"intensity="<<intensity<<"  brightness="<<par->get_brightness()<<"  contrast="<<par->get_contrast()<<std::endl;
      //std::cout<<"val="<<(int)val<<"  newval="<<(int)newval<<"  pout[0]="<<(int)pout[0]<<std::endl;
      val = (typename FormatInfo<T>::PROMOTED)p[first][x+1] - FormatInfo<T>::HALF;
      newval = (intensity*par->get_contrast()+1.0f)*val + intensity*par->get_brightness()*FormatInfo<T>::RANGE + FormatInfo<T>::HALF;
      clip(newval,pout[x+1]);
      val = (typename FormatInfo<T>::PROMOTED)p[first][x+2] - FormatInfo<T>::HALF;
      newval = (intensity*par->get_contrast()+1.0f)*val + intensity*par->get_brightness()*FormatInfo<T>::RANGE + FormatInfo<T>::HALF;
      clip(newval,pout[x+2]);
      x += 3;
    }
  };

