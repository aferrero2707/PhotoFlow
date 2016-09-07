#include "pftypes.hh"
#include "photoflow.hh"

template<PF::colorspace_t CS>
int PF::ColorspaceInfo<CS>::NCH = 1;


template<>
int PF::ColorspaceInfo<PF::PF_COLORSPACE_RAW>::NCH = 1;

template<>
int PF::ColorspaceInfo<PF::PF_COLORSPACE_GRAYSCALE>::NCH = 1;

template<>
int PF::ColorspaceInfo<PF::PF_COLORSPACE_RGB>::NCH = 3;

template<>
int PF::ColorspaceInfo<PF::PF_COLORSPACE_LAB>::NCH = 3;

template<>
int PF::ColorspaceInfo<PF::PF_COLORSPACE_CMYK>::NCH = 4;



PF::colorspace_t PF::convert_colorspace(VipsInterpretation interpretation) 
{
  switch(interpretation) {
  case VIPS_INTERPRETATION_B_W:
  case VIPS_INTERPRETATION_GREY16:
    return PF::PF_COLORSPACE_GRAYSCALE;
    break;
  case VIPS_INTERPRETATION_MULTIBAND:
  case VIPS_INTERPRETATION_RGB:
  case VIPS_INTERPRETATION_sRGB:
  case VIPS_INTERPRETATION_RGB16:
  case VIPS_INTERPRETATION_scRGB:
    return PF::PF_COLORSPACE_RGB;
    break;
  case VIPS_INTERPRETATION_LAB:
    return PF::PF_COLORSPACE_LAB;
    break;
  case VIPS_INTERPRETATION_CMYK:
    return PF::PF_COLORSPACE_CMYK;
    break;
  default:
    return PF::PF_COLORSPACE_UNKNOWN;
  }
}




PF::Property<PF::blendmode_t>::Property(std::string name, PF::OpParBase* par): 
PF::PropertyBase(name, par)
{
  add_enum_value(PF_BLEND_PASSTHROUGH,"PF_BLEND_PASSTHROUGH",_("Passthrough"));
  add_enum_value(PF_BLEND_NORMAL,"PF_BLEND_NORMAL",_("Normal"));
  //add_enum_value(PF_BLEND_SEP1,"PF_BLEND_SEP1","Separator");
  add_enum_value(PF_BLEND_GRAIN_EXTRACT,"PF_BLEND_GRAIN_EXTRACT",_("Grain extract"));
  add_enum_value(PF_BLEND_GRAIN_MERGE,"PF_BLEND_GRAIN_MERGE",_("Grain merge"));
  add_enum_value(PF_BLEND_OVERLAY,"PF_BLEND_OVERLAY",_("Overlay"));
  //add_enum_value(PF_BLEND_OVERLAY_GIMP,"PF_BLEND_OVERLAY_GIMP",_("Overlay (Gimp)"));
  add_enum_value(PF_BLEND_SOFT_LIGHT,"PF_BLEND_SOFT_LIGHT",_("Soft light"));
  add_enum_value(PF_BLEND_HARD_LIGHT,"PF_BLEND_HARD_LIGHT",_("Hard light"));
  add_enum_value(PF_BLEND_VIVID_LIGHT,"PF_BLEND_VIVID_LIGHT",_("Vivid light"));
  add_enum_value(PF_BLEND_MULTIPLY,"PF_BLEND_MULTIPLY",_("Multiply"));
  add_enum_value(PF_BLEND_SCREEN,"PF_BLEND_SCREEN",_("Screen"));
  add_enum_value(PF_BLEND_LIGHTEN,"PF_BLEND_LIGHTEN",_("Lighten"));
  add_enum_value(PF_BLEND_DARKEN,"PF_BLEND_DARKEN",_("Darken"));
  add_enum_value(PF_BLEND_LUMI,"PF_BLEND_LUMI",_("Luminosity"));
  add_enum_value(PF_BLEND_COLOR,"PF_BLEND_COLOR",_("Color"));
  //set_enum_value(PF_BLEND_OV,"PF_BLEND_OV","Ov");
  //set_enum_value(PF_BLEND_OV,"PF_BLEND_OV","Ov");
  //set_enum_value(PF_BLEND_OV,"PF_BLEND_OV","Ov");
};


