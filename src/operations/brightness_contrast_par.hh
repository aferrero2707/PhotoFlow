  class BrightnessContrastPar: public OpParBase
  {
    Property<float> brightness, contrast;
  public:
    BrightnessContrastPar(): 
      OpParBase(), 
      brightness("brightness",this,0), 
      contrast("contrast",this,0) 
    {
      set_type( "brightness_contrast" );
    }
    float get_brightness() { return brightness.get(); }
    float get_contrast() { return contrast.get(); }
    void set_brightness(float val) { brightness.set(val); }
    void set_contrast(float val) { contrast.set(val); }
  };

  
