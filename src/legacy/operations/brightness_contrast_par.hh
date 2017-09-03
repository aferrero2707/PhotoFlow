  class BrightnessContrastPar: public PixelProcessorPar
  {
    Property<float> brightness, contrast, gamma;
    float exponent;
  public:
    BrightnessContrastPar(): 
      PixelProcessorPar(), 
      brightness("brightness",this,0), 
      contrast("contrast",this,0),
      gamma("gamma",this,0)
    {
      set_type( "brightness_contrast" );

      set_default_name( _("brightness contrast") );
    }
    float get_brightness() { return brightness.get(); }
    float get_contrast() { return contrast.get(); }
    float get_gamma() { return exponent; }
    void set_brightness(float val) { brightness.set(val); }
    void set_contrast(float val) { contrast.set(val); }
  };

  
