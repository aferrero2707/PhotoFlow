  class BrightnessContrastPar: public OpParBase
  {
    double brightness, contrast;
  public:
    BrightnessContrastPar(): OpParBase(), brightness(0), contrast(0) {}
    double get_brightness() { return brightness; }
    double get_contrast() { return contrast; }
    void set_brightness(double val) { brightness = val; }
    void set_contrast(double val) { contrast = val; }
  };

  
