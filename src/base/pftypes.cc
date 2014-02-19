#include "pftypes.hh"


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


