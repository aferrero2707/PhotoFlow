/* 
 */

/*

    Copyright (C) 2014 Ferrero Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.


 */

/*

    These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/

 */

/* We need C linkage for this.
 */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "../external/darktable/src/common/colorspaces.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#include "../external/darktable/src/external/wb_presets.c"
#include "../external/darktable/src/external/adobe_coeff.c"
#include "../external/darktable/src/external/cie_colorimetric_tables.c"

#include "../base/exif_data.hh"
#include "../../operations/raw_image.hh"
#include "../../operations/raw_preprocessor.hh"
#include "../../operations/raw_output.hh"
#include "../../operations/raw_developer.hh"
//#include "../../operations/hotpixels.hh"

#include "raw_developer_config.hh"

/* Darktable code starts here
 */

#define generate_mat3inv_body(c_type, A, B)                                                                  \
    static int mat3inv_##c_type(c_type *const dst, const c_type *const src)                                           \
    {                                                                                                          \
  \
  const c_type det = A(1, 1) * (A(3, 3) * A(2, 2) - A(3, 2) * A(2, 3))                                     \
  - A(2, 1) * (A(3, 3) * A(1, 2) - A(3, 2) * A(1, 3))                                   \
  + A(3, 1) * (A(2, 3) * A(1, 2) - A(2, 2) * A(1, 3));                                  \
  \
  const c_type epsilon = 1e-7f;                                                                            \
  if(fabs(det) < epsilon) return 1;                                                                        \
  \
  const c_type invDet = 1.0 / det;                                                                         \
  \
  B(1, 1) = invDet * (A(3, 3) * A(2, 2) - A(3, 2) * A(2, 3));                                              \
  B(1, 2) = -invDet * (A(3, 3) * A(1, 2) - A(3, 2) * A(1, 3));                                             \
  B(1, 3) = invDet * (A(2, 3) * A(1, 2) - A(2, 2) * A(1, 3));                                              \
  \
  B(2, 1) = -invDet * (A(3, 3) * A(2, 1) - A(3, 1) * A(2, 3));                                             \
  B(2, 2) = invDet * (A(3, 3) * A(1, 1) - A(3, 1) * A(1, 3));                                              \
  B(2, 3) = -invDet * (A(2, 3) * A(1, 1) - A(2, 1) * A(1, 3));                                             \
  \
  B(3, 1) = invDet * (A(3, 2) * A(2, 1) - A(3, 1) * A(2, 2));                                              \
  B(3, 2) = -invDet * (A(3, 2) * A(1, 1) - A(3, 1) * A(1, 2));                                             \
  B(3, 3) = invDet * (A(2, 2) * A(1, 1) - A(2, 1) * A(1, 2));                                              \
  return 0;                                                                                                \
    }

#define A(y, x) src[(y - 1) * 3 + (x - 1)]
#define B(y, x) dst[(y - 1) * 3 + (x - 1)]
/** inverts the given 3x3 matrix */
generate_mat3inv_body(float, A, B)

static int mat3inv(float *const dst, const float *const src)
{
  return mat3inv_float(dst, src);
}

generate_mat3inv_body(double, A, B)
#undef B
#undef A
#undef generate_mat3inv_body


#define INITIALBLACKBODYTEMPERATURE 4000

#define DT_IOP_LOWEST_TEMPERATURE 1901
#define DT_IOP_HIGHEST_TEMPERATURE 25000

#define DT_IOP_LOWEST_TINT 0.135
#define DT_IOP_HIGHEST_TINT 2.326


/*
 * Spectral power distribution functions
 * https://en.wikipedia.org/wiki/Spectral_power_distribution
 */
typedef double((*spd)(unsigned long int wavelength, double TempK));

/*
 * Bruce Lindbloom, "Spectral Power Distribution of a Blackbody Radiator"
 * http://www.brucelindbloom.com/Eqn_Blackbody.html
 */
static double spd_blackbody(unsigned long int wavelength, double TempK)
{
  // convert wavelength from nm to m
  const long double lambda = (double)wavelength * 1e-9;

  /*
   * these 2 constants were computed using following Sage code:
   *
   * (from http://physics.nist.gov/cgi-bin/cuu/Value?h)
   * h = 6.62606957 * 10^-34 # Planck
   * c= 299792458 # speed of light in vacuum
   * k = 1.3806488 * 10^-23 # Boltzmann
   *
   * c_1 = 2 * pi * h * c^2
   * c_2 = h * c / k
   *
   * print 'c_1 = ', c_1, ' ~= ', RealField(128)(c_1)
   * print 'c_2 = ', c_2, ' ~= ', RealField(128)(c_2)
   */

#define c1 3.7417715246641281639549488324352159753e-16L
#define c2 0.014387769599838156481252937624049081933L

  return (double)(c1 / (powl(lambda, 5) * (expl(c2 / (lambda * TempK)) - 1.0L)));

#undef c2
#undef c1
}

/*
 * Bruce Lindbloom, "Spectral Power Distribution of a CIE D-Illuminant"
 * http://www.brucelindbloom.com/Eqn_DIlluminant.html
 * and https://en.wikipedia.org/wiki/Standard_illuminant#Illuminant_series_D
 */
static double spd_daylight(unsigned long int wavelength, double TempK)
{
  cmsCIExyY WhitePoint = { 0.3127, 0.3290, 1.0 };

  /*
   * Bruce Lindbloom, "TempK to xy"
   * http://www.brucelindbloom.com/Eqn_T_to_xy.html
   */
  cmsWhitePointFromTemp(&WhitePoint, TempK);

  const double M = (0.0241 + 0.2562 * WhitePoint.x - 0.7341 * WhitePoint.y),
      m1 = (-1.3515 - 1.7703 * WhitePoint.x + 5.9114 * WhitePoint.y) / M,
      m2 = (0.0300 - 31.4424 * WhitePoint.x + 30.0717 * WhitePoint.y) / M;

  const unsigned long int j
  = ((wavelength - cie_daylight_components[0].wavelength)
      / (cie_daylight_components[1].wavelength - cie_daylight_components[0].wavelength));

  return (cie_daylight_components[j].S[0] + m1 * cie_daylight_components[j].S[1]
                                                                              + m2 * cie_daylight_components[j].S[2]);
}

/*
 * Bruce Lindbloom, "Computing XYZ From Spectral Data (Emissive Case)"
 * http://www.brucelindbloom.com/Eqn_Spect_to_XYZ.html
 */
static cmsCIEXYZ spectrum_to_XYZ(double TempK, spd I)
{
  cmsCIEXYZ Source = {.X = 0.0, .Y = 0.0, .Z = 0.0 };

  /*
   * Color matching functions
   * https://en.wikipedia.org/wiki/CIE_1931_color_space#Color_matching_functions
   */
  for(size_t i = 0; i < cie_1931_std_colorimetric_observer_count; i++)
  {
    const unsigned long int lambda = cie_1931_std_colorimetric_observer[0].wavelength
        + (cie_1931_std_colorimetric_observer[1].wavelength
            - cie_1931_std_colorimetric_observer[0].wavelength) * i;
    const double P = I(lambda, TempK);
    Source.X += P * cie_1931_std_colorimetric_observer[i].xyz.X;
    Source.Y += P * cie_1931_std_colorimetric_observer[i].xyz.Y;
    Source.Z += P * cie_1931_std_colorimetric_observer[i].xyz.Z;
  }

  // normalize so that each component is in [0.0, 1.0] range
  const double _max = MAX(MAX(Source.X, Source.Y), Source.Z);
  Source.X /= _max;
  Source.Y /= _max;
  Source.Z /= _max;

  return Source;
}

//
static cmsCIEXYZ temperature_to_XYZ(double TempK)
{
  if(TempK < DT_IOP_LOWEST_TEMPERATURE) TempK = DT_IOP_LOWEST_TEMPERATURE;
  if(TempK > DT_IOP_HIGHEST_TEMPERATURE) TempK = DT_IOP_HIGHEST_TEMPERATURE;

  if(TempK < INITIALBLACKBODYTEMPERATURE)
  {
    // if temperature is less than 4000K we use blackbody,
    // because there will be no Daylight reference below 4000K...
    return spectrum_to_XYZ(TempK, spd_blackbody);
  }
  else
  {
    return spectrum_to_XYZ(TempK, spd_daylight);
  }
}

// binary search inversion
static void XYZ_to_temperature(cmsCIEXYZ XYZ, double *TempK, double *tint)
{
  double maxtemp = DT_IOP_HIGHEST_TEMPERATURE, mintemp = DT_IOP_LOWEST_TEMPERATURE;
  cmsCIEXYZ _xyz;

  for(*TempK = (maxtemp + mintemp) / 2.0; (maxtemp - mintemp) > 1.0; *TempK = (maxtemp + mintemp) / 2.0)
  {
    _xyz = temperature_to_XYZ(*TempK);
    if(_xyz.Z / _xyz.X > XYZ.Z / XYZ.X)
      maxtemp = *TempK;
    else
      mintemp = *TempK;
  }

  *tint = (_xyz.Y / _xyz.X) / (XYZ.Y / XYZ.X);

  if(*TempK < DT_IOP_LOWEST_TEMPERATURE) *TempK = DT_IOP_LOWEST_TEMPERATURE;
  if(*TempK > DT_IOP_HIGHEST_TEMPERATURE) *TempK = DT_IOP_HIGHEST_TEMPERATURE;
  if(*tint < DT_IOP_LOWEST_TINT) *tint = DT_IOP_LOWEST_TINT;
  if(*tint > DT_IOP_HIGHEST_TINT) *tint = DT_IOP_HIGHEST_TINT;
}

void PF::RawDeveloperConfigGUI::temp2mul(double TempK, double tint, double mul[3])
{
  //dt_iop_temperature_gui_data_t *g = (dt_iop_temperature_gui_data_t *)self->gui_data;

  cmsCIEXYZ _xyz = temperature_to_XYZ(TempK);

  double XYZ[3] = { _xyz.X, _xyz.Y / tint, _xyz.Z };

  double CAM[3];
  for(int k = 0; k < 3; k++)
  {
    CAM[k] = 0.0;
    for(int i = 0; i < 3; i++)
    {
      CAM[k] += XYZ_to_CAM[k][i] * XYZ[i];
    }
  }

  for(int k = 0; k < 3; k++) mul[k] = 1.0 / CAM[k];
}

void PF::RawDeveloperConfigGUI::mul2temp(float coeffs[3], double *TempK, double *tint)
{
  //dt_iop_temperature_gui_data_t *g = (dt_iop_temperature_gui_data_t *)self->gui_data;

#ifndef NDEBUG
  //std::cout<<"mul2temp(): coeffs="<<coeffs[0]<<","<<coeffs[1]<<","<<coeffs[2]<<std::endl;
  printf("mul2temp: coeffs[]=%f,%f,%f\nCAM_to_XYZ:\n",coeffs[0],coeffs[1],coeffs[2]);
  for(int k = 0; k < 3; k++) {
    printf("    %.4f %.4f %.4f\n",CAM_to_XYZ[k][0],CAM_to_XYZ[k][1],CAM_to_XYZ[k][2]);
  }
#endif
  double CAM[3];
  for(int k = 0; k < 3; k++) CAM[k] = 1.0 / coeffs[k];

  double XYZ[3];
  for(int k = 0; k < 3; k++)
  {
    XYZ[k] = 0.0;
    for(int i = 0; i < 3; i++)
    {
      XYZ[k] += CAM_to_XYZ[k][i] * CAM[i];
    }
  }

  XYZ_to_temperature((cmsCIEXYZ){ XYZ[0], XYZ[1], XYZ[2] }, TempK, tint);
}

/* Darktable code ends here
 */



#ifndef MIN
#define MIN( a, b ) ((a<b) ? a : b)
#endif
#define MIN3( a, b, c ) MIN(a,MIN(b,c))

#ifndef MAX
#define MAX( a, b ) ((a>b) ? a : b)
#endif
#define MAX3( a, b, c ) MAX(a,MAX(b,c))


bool PF::WBSelector::check_value( int id, const std::string& name, const std::string& val )
{
  if( id < 4 ) return true;
  //std::cout<<"WBSelector::check_value(): maker="<<maker<<" model="<<model<<std::endl;
  for(int i = 0; i < wb_preset_count; i++) {
    //std::cout<<"  wb_preset[i].make="<<wb_preset[i].make<<" wb_preset[i].model="<<wb_preset[i].model<<std::endl;
    if( maker == wb_preset[i].make && model == wb_preset[i].model ) {
      //std::cout<<"    val="<<val<<" wb_preset[i].name="<<wb_preset[i].name<<std::endl;
      if( val == wb_preset[i].name ) {
        return true;
      }
    }
  }
  return false;
}




static double black_level_prop_to_slider(double& val, PF::OperationConfigGUI* dialog, void* user_data)
{
  if( !dialog ) return val;
  if( !dialog->get_layer() ) return val;
  if( !dialog->get_layer()->get_image() ) return val;
  if( !user_data ) return val;

  dcraw_data_t* raw_data = NULL;
  float result = val;

  PF::Image* image = dialog->get_layer()->get_image();
  PF::Pipeline* pipeline = image->get_pipeline(0);
  PF::PipelineNode* node = NULL;
  PF::PipelineNode* inode = NULL;
  PF::ProcessorBase* processor = NULL;
  std::string maker, model;
  if( pipeline ) node = pipeline->get_node( dialog->get_layer()->get_id() );
  if( node ) inode = pipeline->get_node( node->input_id );
  if( inode && inode->image) {
    size_t blobsz;
    if( PF_VIPS_IMAGE_GET_BLOB( inode->image, "raw_image_data", &raw_data, &blobsz ) ||
        blobsz != sizeof(dcraw_data_t) ) {
      raw_data = NULL;
    }

    if(!raw_data) return val;

    int* pc = (int*)user_data;
    int c = *pc;
    if(c<0 || c>3) return val;
    float black_level = raw_data->color.cblack[c];
    float white_level = raw_data->color.maximum;

    result = roundf(val * black_level);
  }

  return result;
}


static double black_level_slider_to_prop(double& val, PF::OperationConfigGUI* dialog, void* user_data)
{
  float result = 1;
  if( !dialog ) return result;
  if( !dialog->get_layer() ) return result;
  if( !dialog->get_layer()->get_image() ) return result;
  if( !user_data ) return result;

  dcraw_data_t* raw_data = NULL;

  PF::Image* image = dialog->get_layer()->get_image();
  PF::Pipeline* pipeline = image->get_pipeline(0);
  PF::PipelineNode* node = NULL;
  PF::PipelineNode* inode = NULL;
  PF::ProcessorBase* processor = NULL;
  std::string maker, model;
  if( pipeline ) node = pipeline->get_node( dialog->get_layer()->get_id() );
  if( node ) inode = pipeline->get_node( node->input_id );
  if( inode && inode->image) {
    size_t blobsz;
    if( PF_VIPS_IMAGE_GET_BLOB( inode->image, "raw_image_data", &raw_data, &blobsz ) ||
        blobsz != sizeof(dcraw_data_t) ) {
      raw_data = NULL;
    }

    if(!raw_data) return result;

    int* pc = (int*)user_data;
    int c = *pc;
    if(c<0 || c>3) return result;
    float black_level = raw_data->color.cblack[c];
    float white_level = raw_data->color.maximum;

    result = val / black_level;
  }

  return result;
}


static double white_level_prop_to_slider(double& val, PF::OperationConfigGUI* dialog, void*)
{
  if( !dialog ) return val;
  if( !dialog->get_layer() ) return val;
  if( !dialog->get_layer()->get_image() ) return val;

  dcraw_data_t* raw_data = NULL;
  float result = val;

  PF::Image* image = dialog->get_layer()->get_image();
  PF::Pipeline* pipeline = image->get_pipeline(0);
  PF::PipelineNode* node = NULL;
  PF::PipelineNode* inode = NULL;
  PF::ProcessorBase* processor = NULL;
  std::string maker, model;
  if( pipeline ) node = pipeline->get_node( dialog->get_layer()->get_id() );
  if( node ) inode = pipeline->get_node( node->input_id );
  if( inode && inode->image) {
    size_t blobsz;
    if( PF_VIPS_IMAGE_GET_BLOB( inode->image, "raw_image_data", &raw_data, &blobsz ) ||
        blobsz != sizeof(dcraw_data_t) ) {
      raw_data = NULL;
    }

    if(!raw_data) return val;

    float white_level = raw_data->color.maximum;

    result = roundf(val * white_level);
  }

  return result;
}


static double white_level_slider_to_prop(double& val, PF::OperationConfigGUI* dialog, void*)
{
  float result = 1;
  if( !dialog ) return result;
  if( !dialog->get_layer() ) return result;
  if( !dialog->get_layer()->get_image() ) return result;

  dcraw_data_t* raw_data = NULL;

  PF::Image* image = dialog->get_layer()->get_image();
  PF::Pipeline* pipeline = image->get_pipeline(0);
  PF::PipelineNode* node = NULL;
  PF::PipelineNode* inode = NULL;
  PF::ProcessorBase* processor = NULL;
  std::string maker, model;
  if( pipeline ) node = pipeline->get_node( dialog->get_layer()->get_id() );
  if( node ) inode = pipeline->get_node( node->input_id );
  if( inode && inode->image) {
    size_t blobsz;
    if( PF_VIPS_IMAGE_GET_BLOB( inode->image, "raw_image_data", &raw_data, &blobsz ) ||
        blobsz != sizeof(dcraw_data_t) ) {
      raw_data = NULL;
    }

    if(!raw_data) return result;

    float white_level = raw_data->color.maximum;

    result = val / white_level;
  }

  return result;
}



#ifdef GTKMM_2
bool PF::RawHistogramArea::on_expose_event (GdkEventExpose * event)
{
  std::cout<<"RawHistogramArea::on_expose_event() called"<<std::endl;

  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if( !window )
    return true;

  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

#endif
#ifdef GTKMM_3
bool PF::RawHistogramArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  std::cout<<"RawHistogramArea::on_draw() called"<<std::endl;

#endif
  Pango::FontDescription font;
  int text_width;
  int text_height;

  font.set_family("Monospace");
  font.set_weight(Pango::WEIGHT_BOLD);
  auto layout = create_pango_layout("B");
  layout->set_font_description(font);
  //get the text dimensions (it updates the variables -- by reference)
  layout->get_pixel_size(text_width, text_height);

  int hborder_size = text_width/2;
  int border_top = 2;
  int border_bottom = text_height+4;

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width() - hborder_size*2;
  const int height = allocation.get_height() - border_top - border_bottom;
  const int x0 = hborder_size;
  const int y0 = border_top;

  cr->save();
  cr->set_source_rgba(0.2, 0.2, 0.2, 1.0);
  cr->paint();
  cr->restore();

  // Draw outer rectangle
  cr->set_antialias( Cairo::ANTIALIAS_GRAY );
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  cr->set_line_width( 0.5 );
  cr->rectangle( double(hborder_size), double(0.5),
      double(width), double(height+border_top-0.5) );
  cr->stroke ();

  std::cout<<"[RawHistogramArea] hist="<<hist<<std::endl;

  if( hist == NULL ) return TRUE;

  unsigned long int* h[3];
  h[0] = hist;
  h[1] = &(hist[65536]);
  h[2] = &(hist[65536*2]);

  unsigned short int xmax = (white * 1.1) > 65535 ? 65535 : (white * 1.1);
  unsigned short int xmin = 0;
  if( zoom_black ) {
    xmax = black * 2;
  } else if( zoom_white ) {
    xmin = white * 0.9;
  }
  std::cout<<"[RawHistogramArea] xmin="<<xmin<<"  xmax="<<xmax<<std::endl;

  //for(unsigned int i = 65535; i >= 0; i--) {
  //  if(h[0][i] > 0 || h[1][i] > 0 || h[2][i] > 0) {
  //    xmax = i;
  //    break;
  //  }
  //}
  //xmax += xmax/10;

  unsigned long int* hh[3];
  hh[0] = new unsigned long int[width];
  hh[1] = new unsigned long int[width];
  hh[2] = new unsigned long int[width];

  unsigned long int max = 0;
  for( int i = 0; i < 3; i++ ) {
    for( int j = 0; j < width; j++ ) {
      hh[i][j] = 0;
    }
    for( int j = xmin; j <= xmax; j++ ) {
      float nj = j;
      nj = (nj-xmin) * width / (xmax-xmin);
      int bin = (int)nj;
      if( bin < 0 ) continue;
      if( bin >= width ) continue;
      //if(j==65535) std::cout<<"j="<<j<<"  bin="<<bin<<"  width="<<width<<std::endl;
      hh[i][bin] += h[i][j];
    }
    for( int j = 0; j < width; j++ ) {
      if( hh[i][j] > max) max = hh[i][j];
    }
  }
  max *= 1.05;

  for( int i = 0; i < 3; i++ ) {
    if( i == 0 ) cr->set_source_rgb( 0.9, 0., 0. );
    if( i == 1 ) cr->set_source_rgb( 0., 0.9, 0. );
    if( i == 2 ) cr->set_source_rgb( 0.2, 0.6, 1. );

    float ny = 0;
    if( max > 0 ) {
      ny = hh[i][0]; ny *= height; ny /= max;
    }
    float y = height; y -= ny; y -= 1;
    cr->move_to( x0, y+y0 );
    for( int j = 1; j < width; j++ ) {
      ny = 0;
      if( max > 0 ) {
        ny = hh[i][j]; ny *= height; ny /= max;
      }
      y = height; y -= ny; y -= 1;
      //y = (max>0) ? height - hh[i][j]*height/max - 1 : height-1;
      //std::cout<<"bin #"<<j<<": "<<hh[i][j]<<" ("<<max<<") -> "<<y<<std::endl;
      cr->line_to( j+x0, y+y0 );
    }
    cr->stroke();
  }

  float fullrange = xmax-xmin;
  float x_0 = (black-xmin) * width / fullrange;
  float x_1 = (white-xmin) * width / fullrange;

  cr->set_antialias( Cairo::ANTIALIAS_GRAY );
  cr->set_source_rgb( 0.9, 0.9, 0.9 );
  cr->set_line_width( 0.5 );
  std::valarray< double > dashes(2);
  dashes[0] = 2.0;
  dashes[1] = 2.0;
  cr->set_dash (dashes, 0.0);
  std::cout<<"[Histogram] black="<<black<<" white="<<white<<std::endl;
  if( xmax > white ) {
    std::cout<<"[Histogram] drawin line at "<<x_1<<std::endl;
    cr->move_to( x0+x_1, y0);
    cr->line_to( x0+x_1, y0+height );
    cr->stroke();
  }
  if( xmin < black ) {
    std::cout<<"[Histogram] drawin line at 0"<<std::endl;
    cr->move_to( x0+x_0, y0);
    cr->line_to( x0+x_0, y0+height );
    cr->stroke();
  }

  if( true ) {
  layout = create_pango_layout("B");
  layout->set_font_description(font);
  //get the text dimensions (it updates the variables -- by reference)
  layout->get_pixel_size(text_width, text_height);
  // Position the text
  cr->move_to(x0+x_0-text_width/2, y0+height+4);
  layout->show_in_cairo_context(cr);

  layout = create_pango_layout("W");
  layout->set_font_description(font);
  //get the text dimensions (it updates the variables -- by reference)
  layout->get_pixel_size(text_width, text_height);
  // Position the text
  cr->move_to(x0+x_1-text_width/2, y0+height+4);
  layout->show_in_cairo_context(cr);
  }

  if(hh[0]) delete[] hh[0];
  if(hh[1]) delete[] hh[1];
  if(hh[2]) delete[] hh[2];

  return TRUE;
}




PF::RawDeveloperConfigGUI::RawDeveloperConfigGUI( PF::Layer* layer ):
        OperationConfigGUI( layer, "Raw Developer" ),
        wbModeSelector( this, "wb_mode", "WB mode: ", 0 ),
        wbTempSlider( this, "", _("temp."), 15000, DT_IOP_LOWEST_TEMPERATURE, DT_IOP_HIGHEST_TEMPERATURE, 10, 100, 1),
        wbTintSlider( this, "", _("tint"), 1, DT_IOP_LOWEST_TINT, DT_IOP_HIGHEST_TINT, 0.01, 0.1, 1),
        //wbRedSlider( this, "wb_red", "Red mult.", 1, 0, 10, 0.05, 0.1, 1),
        //wbGreenSlider( this, "wb_green", "Green mult.", 1, 0, 10, 0.05, 0.1, 1),
        //wbBlueSlider( this, "wb_blue", "Blue mult.", 1, 0, 10, 0.05, 0.1, 1),
        wbRedCorrSlider( this, "camwb_corr_red", "R corr.", 1, 0, 10, 0.05, 0.1, 1),
        wbGreenCorrSlider( this, "camwb_corr_green", "G corr.", 1, 0, 10, 0.05, 0.1, 1),
        wbBlueCorrSlider( this, "camwb_corr_blue", "B corr.", 1, 0, 10, 0.05, 0.1, 1),
        wb_target_L_slider( this, "wb_target_L", "Target: ", 50, 0, 1000000, 0.05, 0.1, 1),
        wb_target_a_slider( this, "wb_target_a", "a: ", 10, -1000000, 1000000, 0.05, 0.1, 1),
        wb_target_b_slider( this, "wb_target_b", "b: ", 12, -1000000, 1000000, 0.05, 0.1, 1),
        enable_ca_checkbox( this, "enable_ca", _("enable CA correction"), false ),
        hotp_enable_checkbox( this, "hotp_enable", _("enable hot pixels correction"), false ),
        hotp_threshold_slider( this, "hotp_threshold", _("threshold"), 0, 0.0, 1.0, 0.01, 0.01, 1), // "lower threshold increases removal for hot pixel"
        hotp_strength_slider( this, "hotp_strength", _("strength"), 0, 0.0, 1.0, 0.005, 0.005, 1), // "strength of hot pixel correction"
        hotp_permissive_checkbox( this, "hotp_permissive", _("detect by 3 neighbors"), false ),
        hotp_markfixed_checkbox( this, "hotp_markfixed", _("mark fixed pixels"), false ),
        hotp_frame( _("hot pixels filter") ),
        ca_mode_selector( this, "tca_method", _("CA correction: "), PF::PF_TCA_CORR_AUTO ),
        auto_ca_checkbox( this, "auto_ca", _("auto"), true ),
        ca_red_slider( this, "ca_red", _("red"), 0, -4, 4, 0.1, 0.5, 1),
        ca_blue_slider( this, "ca_blue", _("blue"), 0, -4, 4, 0.1, 0.5, 1),
        ca_frame( _("CA correction") ),
        lf_auto_matching_checkbox( this, "auto_matching", _("auto matching"), true, 1 ),
        lf_auto_crop_checkbox( this, "lf_auto_crop", _("auto cropping"), true, 1 ),
        lf_cam_selector( this, "camera_maker", "camera_model", _("camera model: "), 0, 150 ),
        lf_lens_selector( this, "lens", _("lens model: "), 0, 150 ),
        lf_selector( this, "camera_maker", "camera_model", "lens" ),
        lf_enable_distortion_button( this, "lf_enable_distortion", _("distortion"), false ),
        lf_enable_tca_button( this, "lf_enable_tca", _("chromatic aberrations (CA)"), false ),
        lf_enable_vignetting_button( this, "lf_enable_vignetting", _("vignetting"), false ),
        lf_enable_all_button( this, "lf_enable_all", _("all corrections"), false ),
        lens_frame( _("lens corrections") ),
        demoMethodSelector( this, "demo_method", _("demosaicing method: "), PF::PF_DEMO_AMAZE ),
        fcsSlider( this, "fcs_steps", "FCC steps", 1, 0, 4, 1, 1, 1 ),
        exposureSlider( this, "exposure", "Exposure ", 0, -10, 10, 0.05, 0.5 ),
        saturationLevelSlider( this, "saturation_level_correction", _("white level"), 100, 0, 65535, 1, 5, 1, 250, 3 ),
        blackLevelSlider( this, "black_level_correction", _("black level %"), 100, 0, 200, 0.5, 5, 100 ),
        blackLevelRSlider( this, "black_level_correction_r", _("black level [R]"), 100, 0, 10000, 1, 5, 1, 250, 3 ),
        blackLevelG1Slider( this, "black_level_correction_g1", _("black level [G1]"), 100, 0, 10000, 1, 5, 1, 250, 3 ),
        blackLevelG2Slider( this, "black_level_correction_g2", _("black level [G2]"), 100, 0, 10000, 1, 5, 1, 250, 3 ),
        blackLevelBSlider( this, "black_level_correction_b", _("black level [B]"), 100, 0, 10000, 1, 5, 1, 250, 3 ),
        hlrecoModeSelector( this, "hlreco_mode", _("highlights reco: "), PF::HLRECO_CLIP ),
        profileModeSelector( this, "profile_mode", _("input: "), 0 ),
        camProfOpenButton(Gtk::Stock::OPEN),
        camDCPProfOpenButton(Gtk::Stock::OPEN),
        apply_hue_sat_map_checkbox( this, "apply_hue_sat_map", _("base table"), true ),
        apply_look_table_checkbox( this, "apply_look_table", _("look table"), true ),
        use_tone_curve_checkbox( this, "use_tone_curve", _("tone curve"), true ),
        apply_baseline_exposure_offset_checkbox( this, "apply_baseline_exposure_offset", _("baseline exposure"), true ),
        gammaModeSelector( this, "gamma_mode", "raw curve: ", 0 ),
        inGammaLinSlider( this, "gamma_lin", "Gamma linear", 0, 0, 100000, 0.05, 0.1, 1),
        inGammaExpSlider( this, "gamma_exp", "Gamma exponent", 2.2, 0, 100000, 0.05, 0.1, 1),
        outProfileModeSelector( this, "out_profile_mode", _("type: "), 1, 80 ),
        outProfileTypeSelector( this, "out_profile_type", _("gamut: "), 1, 80 ),
        outTRCTypeSelector( this, "out_trc_type", _("encoding: "), 1, 80 ),
        outProfOpenButton(Gtk::Stock::OPEN),
        inProfFrame( _("camera profile") ),
        outProfFrame( _("working profile") ),
        hist_range_label(_("range: ")),
        hist_range_full_label(_("full")),
        hist_range_sh_label(_("shadows")),
        hist_range_hi_label(_("highlights")),
        clip_negative_checkbox( this, "clip_negative", _("clip negative values"), true ),
        clip_overflow_checkbox( this, "clip_overflow", _("clip overflow values"), true ),
        selected_wb_area_id(-1),
        ignore_temp_tint_change( false )
{
  char tstr[100];

  wbControlsBox.pack_start( wbModeSelector, Gtk::PACK_SHRINK );

  wb_target_L_slider.set_passive( true );
  wb_target_a_slider.set_passive( true );
  wb_target_b_slider.set_passive( true );
  //wbTargetBox.pack_start( wb_target_L_slider, Gtk::PACK_SHRINK );
  wbTargetBox.pack_start( wb_target_a_slider, Gtk::PACK_SHRINK );
  wbTargetBox.pack_start( wb_target_b_slider, Gtk::PACK_SHRINK );
  wbControlsBox.pack_start( wbTargetBox );
  wbControlsBox.pack_start( wb_best_match_label, Gtk::PACK_SHRINK );

  wbControlsBox.pack_start( wbTempSlider, Gtk::PACK_SHRINK );
  wbControlsBox.pack_start( wbTintSlider, Gtk::PACK_SHRINK );
  //wbControlsBox.pack_start( wbRedSlider, Gtk::PACK_SHRINK );
  //wbControlsBox.pack_start( wbGreenSlider, Gtk::PACK_SHRINK );
  //wbControlsBox.pack_start( wbBlueSlider, Gtk::PACK_SHRINK );
  //wbControlsBox.pack_start( wbRedCorrSlider, Gtk::PACK_SHRINK );
  //wbControlsBox.pack_start( wbGreenCorrSlider, Gtk::PACK_SHRINK );
  //wbControlsBox.pack_start( wbBlueCorrSlider, Gtk::PACK_SHRINK );

  for( unsigned int i = 0; i < PF::WB_LAST; i++ ) {
    snprintf(tstr,99,"wb_red_%d", i);
    wbRedSliders[i] = new PF::Slider( this, tstr, "red", 1, 0, 10, 0.05, 0.1, 1);
    snprintf(tstr,99,"wb_green_%d", i);
    wbGreenSliders[i] = new PF::Slider( this, tstr, "green", 1, 0, 10, 0.05, 0.1, 1);
    snprintf(tstr,99,"wb_blue_%d", i);
    wbBlueSliders[i] = new PF::Slider( this, tstr, "blue", 1, 0, 10, 0.05, 0.1, 1);

    wbSliderBoxes[i].pack_start(*wbRedSliders[i]);
    wbSliderBoxes[i].pack_start(*wbGreenSliders[i]);
    wbSliderBoxes[i].pack_start(*wbBlueSliders[i]);
    wbSliderBox.pack_start( wbSliderBoxes[i], Gtk::PACK_SHRINK );
    wbSliderBoxes[i].hide();
  }
  wbControlsBox.pack_start( wbSliderBox, Gtk::PACK_SHRINK );

  wbControlsBox.pack_start( exp_separator, Gtk::PACK_SHRINK, 5 );
  wbControlsBox.pack_start( exposureSlider, Gtk::PACK_SHRINK, 2 );
  wbControlsBox.pack_start( hlrecoModeSelector, Gtk::PACK_SHRINK, 2 );

  wbControlsBox.pack_start( demo_separator, Gtk::PACK_SHRINK, 5 );
  wbControlsBox.pack_start( demoMethodSelector, Gtk::PACK_SHRINK );
  wbControlsBox.pack_start( fcsSlider, Gtk::PACK_SHRINK );


  black_level_label_align.set( 0, 0.5, 0, 0 );
  white_level_label_align.set( 0, 0.5, 0, 0 );
  black_level_label_align.add( black_level_label );
  white_level_label_align.add( white_level_label );


  int* pc = (int*)malloc(sizeof(int)); *pc = 0;
  blackLevelRSlider.set_conversion_functions(black_level_slider_to_prop, black_level_prop_to_slider, pc);
  pc = (int*)malloc(sizeof(int)); *pc = 1;
  blackLevelG1Slider.set_conversion_functions(black_level_slider_to_prop, black_level_prop_to_slider, pc);
  pc = (int*)malloc(sizeof(int)); *pc = 3;
  blackLevelG2Slider.set_conversion_functions(black_level_slider_to_prop, black_level_prop_to_slider, pc);
  pc = (int*)malloc(sizeof(int)); *pc = 2;
  blackLevelBSlider.set_conversion_functions(black_level_slider_to_prop, black_level_prop_to_slider, pc);

  saturationLevelSlider.set_conversion_functions(white_level_slider_to_prop, white_level_prop_to_slider);

  //exposureControlsBox.pack_start( black_level_label_align, Gtk::PACK_SHRINK, 2 );
  //exposureControlsBox.pack_start( white_level_label_align, Gtk::PACK_SHRINK, 2 );
  //exposureControlsBox.pack_start( blackLevelSlider, Gtk::PACK_SHRINK, 2 );
  exposureControlsBox.pack_start( blackLevelRSlider, Gtk::PACK_SHRINK, 2 );
  exposureControlsBox.pack_start( blackLevelG1Slider, Gtk::PACK_SHRINK, 2 );
  exposureControlsBox.pack_start( blackLevelG2Slider, Gtk::PACK_SHRINK, 2 );
  exposureControlsBox.pack_start( blackLevelBSlider, Gtk::PACK_SHRINK, 2 );
  exposureControlsBox.pack_start( saturationLevelSlider, Gtk::PACK_SHRINK, 10 );

  lensControlsBox.pack_start( hotp_frame, Gtk::PACK_SHRINK, 10 );
  lensControlsBox.pack_start( lens_frame, Gtk::PACK_SHRINK, 10 );
  //lensControlsBox.pack_start( ca_frame, Gtk::PACK_SHRINK, 10 );
  hotp_frame.add( hotp_box );
  hotp_box.pack_start( hotp_enable_checkbox, Gtk::PACK_SHRINK );
  hotp_box.pack_start( hotp_threshold_slider, Gtk::PACK_SHRINK );
  hotp_box.pack_start( hotp_strength_slider, Gtk::PACK_SHRINK );
  hotp_box.pack_start( hotp_permissive_checkbox, Gtk::PACK_SHRINK );
  //hotp_box.pack_start( hotp_markfixed_checkbox, Gtk::PACK_SHRINK );

  lens_frame.add( lf_box );
  lf_box.pack_start( lf_selector );
  lf_hbox.pack_start( lf_auto_matching_checkbox );
  lf_hbox.pack_start( lf_enable_all_button );
  lf_hbox2.pack_start( lf_auto_crop_checkbox );
  lf_hbox2.pack_start( lf_enable_vignetting_button );
  lf_box.pack_start( lf_hbox );
  lf_box.pack_start( lf_hbox2 );
  //lf_box.pack_start( lf_auto_crop_checkbox );
  //lf_box.pack_start( lf_enable_vignetting_button );
  lf_box.pack_start( lf_enable_distortion_button );
  lf_box.pack_start( lf_enable_tca_button );
  lf_makerEntry.set_editable( false );
  lf_modelEntry.set_editable( false );
  lf_lensEntry.set_editable( false );


  //lf_box.pack_start( enable_ca_checkbox, Gtk::PACK_SHRINK );
  lf_box.pack_start( ca_mode_selector, Gtk::PACK_SHRINK );
  //lf_box.pack_start( auto_ca_checkbox, Gtk::PACK_SHRINK );
  lf_box.pack_start( ca_box, Gtk::PACK_SHRINK );
  ca_box.pack_start( ca_red_slider, Gtk::PACK_SHRINK );
  ca_box.pack_start( ca_blue_slider, Gtk::PACK_SHRINK );

  //===================
  // Input camera profile
  outputControlsBox.pack_start( inProfFrame, Gtk::PACK_SHRINK, 10 );
  outputControlsBox.pack_start( outProfFrame, Gtk::PACK_SHRINK, 10 );
  inProfFrame.add( inProfBox );
  outProfFrame.add( outProfBox );

  inProfBox.pack_start( profileModeSelectorBox, Gtk::PACK_SHRINK, 4 );
  profileModeSelectorBox.pack_end( profileModeSelector, Gtk::PACK_SHRINK, 4 );

  gammaModeVBox.pack_start( gammaModeSelector, Gtk::PACK_SHRINK );
  //gammaModeVBox.pack_start( inGammaLinSlider );
  gammaModeVBox.pack_start( inGammaExpSlider, Gtk::PACK_SHRINK );
  gammaModeHBox.pack_end( gammaModeVBox, Gtk::PACK_SHRINK );
  inProfBox.pack_start( gammaModeHBox, Gtk::PACK_SHRINK, 4 );

  camProfLabel.set_text( "camera profile name:" );
  camProfVBox.pack_start( camProfLabel, Gtk::PACK_SHRINK );
  camProfVBox.pack_start( camProfFileEntry, Gtk::PACK_SHRINK );
  camProfHBox.pack_end( camProfOpenButton, Gtk::PACK_SHRINK, 4 );
  camProfHBox.pack_end( camProfVBox, Gtk::PACK_SHRINK, 4 );
  inProfBox.pack_start( camProfHBox, Gtk::PACK_SHRINK, 4 );

  camDCPProfLabel.set_text( "DCP profile name:" );
  camDCPProfVBox.pack_start( camDCPProfLabel, Gtk::PACK_SHRINK );
  camDCPProfVBox.pack_start( camDCPProfFileEntry, Gtk::PACK_SHRINK );
  camDCPProfHBox.pack_end( camDCPProfOpenButton, Gtk::PACK_SHRINK, 4 );
  camDCPProfHBox.pack_end( camDCPProfVBox, Gtk::PACK_SHRINK, 4 );
  inProfBox.pack_start( camDCPProfHBox, Gtk::PACK_SHRINK, 4 );

  dcp_options_box.pack_start( use_tone_curve_checkbox, Gtk::PACK_SHRINK );
  dcp_options_box.pack_start( apply_hue_sat_map_checkbox, Gtk::PACK_SHRINK );
  dcp_options_box.pack_start( apply_look_table_checkbox, Gtk::PACK_SHRINK );
  dcp_options_box.pack_start( apply_baseline_exposure_offset_checkbox, Gtk::PACK_SHRINK );
  inProfBox.pack_start( dcp_options_box, Gtk::PACK_SHRINK, 4 );

  outProfileModeSelectorBox.pack_end( outProfileModeSelector, Gtk::PACK_SHRINK, 4 );
  outProfBox.pack_start( outProfileModeSelectorBox, Gtk::PACK_SHRINK, 4 );

  //outProfileTypeSelectorBox.pack_end( outProfileTypeSelector, Gtk::PACK_SHRINK );
  //outProfBox.pack_start( outProfileTypeSelectorBox, Gtk::PACK_SHRINK );

  outTRCTypeSelectorBox.pack_end( outTRCTypeSelector, Gtk::PACK_SHRINK, 4 );
  outProfBox.pack_start( outTRCTypeSelectorBox, Gtk::PACK_SHRINK, 4 );

  outProfBox.pack_start( outProfHBox, Gtk::PACK_SHRINK );
  outProfLabel.set_text( "output profile name:" );
  outProfVBox.pack_start( outProfLabel, Gtk::PACK_SHRINK );
  outProfVBox.pack_start( outProfFileEntry, Gtk::PACK_SHRINK );
  outProfHBox.pack_start( outProfVBox, Gtk::PACK_SHRINK );
  outProfHBox.pack_start( outProfOpenButton, Gtk::PACK_SHRINK );
  //outputControlsBox.pack_start( outProfHBox, Gtk::PACK_SHRINK );

  outputControlsBox.pack_start( clip_negative_checkbox, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( clip_overflow_checkbox, Gtk::PACK_SHRINK );

  histogramBox.pack_start( histogramArea, Gtk::PACK_EXPAND_WIDGET );
  histogramBox.pack_start( histogramCtrlBox, Gtk::PACK_SHRINK );
  histogramCtrlBox.pack_start( hist_range_label, Gtk::PACK_SHRINK );
  histogramCtrlBox.pack_start( hist_range_full_check, Gtk::PACK_SHRINK );
  histogramCtrlBox.pack_start( hist_range_full_label, Gtk::PACK_SHRINK );
  histogramCtrlBox.pack_start( hist_range_sh_check, Gtk::PACK_SHRINK );
  histogramCtrlBox.pack_start( hist_range_sh_label, Gtk::PACK_SHRINK );
  histogramCtrlBox.pack_start( hist_range_hi_check, Gtk::PACK_SHRINK );
  histogramCtrlBox.pack_start( hist_range_hi_label, Gtk::PACK_SHRINK );

#ifdef GTKMM_2
  Gtk::RadioButton::Group hist_range_check_group = hist_range_full_check.get_group();
  hist_range_sh_check.set_group(hist_range_check_group);
  hist_range_hi_check.set_group(hist_range_check_group);
#endif

#ifdef GTKMM_3
  hist_range_sh_check.join_group(hist_range_full_check);
  hist_range_hi_check.join_group(hist_range_full_check);
#endif
  hist_range_full_check.set_active();


  notebook.append_page( wbControlsBox, "Input" );
  notebook.append_page( lensControlsBox, "Corrections" );
  notebook.append_page( outputControlsBox, "Output" );
  notebook.append_page( histogramBox, "Histogram" );
  notebook.append_page( exposureControlsBox, "Advanced" );

  add_widget( notebook );


  camProfFileEntry.signal_activate().
      connect(sigc::mem_fun(*this,
          &RawDeveloperConfigGUI::on_cam_filename_changed));
  camProfOpenButton.signal_clicked().connect(sigc::mem_fun(*this,
      &RawDeveloperConfigGUI::on_cam_button_open_clicked) );

  camDCPProfFileEntry.signal_activate().
      connect(sigc::mem_fun(*this,
          &RawDeveloperConfigGUI::on_cam_dcp_filename_changed));
  camDCPProfOpenButton.signal_clicked().connect(sigc::mem_fun(*this,
      &RawDeveloperConfigGUI::on_cam_dcp_button_open_clicked) );

  outProfFileEntry.signal_activate().
      connect(sigc::mem_fun(*this,
          &RawDeveloperConfigGUI::on_out_filename_changed));
  outProfOpenButton.signal_clicked().connect(sigc::mem_fun(*this,
      &RawDeveloperConfigGUI::on_out_button_open_clicked) );

  wbTempSlider.get_adjustment()->signal_value_changed().
      connect(sigc::mem_fun(*this,&PF::RawDeveloperConfigGUI::temp_tint_changed));
  wbTintSlider.get_adjustment()->signal_value_changed().
      connect(sigc::mem_fun(*this,&PF::RawDeveloperConfigGUI::temp_tint_changed));

  hist_range_full_check.signal_toggled().connect(
      sigc::mem_fun(*this,&PF::RawDeveloperConfigGUI::on_histogram_radio_group_changed));
  hist_range_sh_check.signal_toggled().connect(
      sigc::mem_fun(*this,&PF::RawDeveloperConfigGUI::on_histogram_radio_group_changed));
  hist_range_hi_check.signal_toggled().connect(
      sigc::mem_fun(*this,&PF::RawDeveloperConfigGUI::on_histogram_radio_group_changed));

  get_main_box().show_all_children();
}


void PF::RawDeveloperConfigGUI::temp_tint_changed()
{
  if( ignore_temp_tint_change ) return;

  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::RawDeveloperPar* par =
        dynamic_cast<PF::RawDeveloperPar*>(get_layer()->get_processor()->get_par());
    if( !par ) return;

    PropertyBase* prop = par->get_property( "wb_mode" );
    if( !prop )  return;

    double temp = wbTempSlider.get_adjustment()->get_value();
    double tint = wbTintSlider.get_adjustment()->get_value();

    double cam_mul[3];
    temp2mul( temp, tint, cam_mul );
    double min_mul = MIN3(cam_mul[0], cam_mul[1], cam_mul[2]);
    for( int i = 0; i < 3; i++ ) cam_mul[i] /= min_mul;
    std::cout<<"temp_tint_changed(): temp="<<temp<<"  tint="<<tint
        <<"  mul="<<cam_mul[0]<<","<<cam_mul[1]<<","<<cam_mul[2]<<std::endl;
    int wb_id = prop->get_enum_value().first;
    wbRedSliders[wb_id]->set_inhibit(true);
    wbRedSliders[wb_id]->get_adjustment()->set_value(cam_mul[0]);
    wbRedSliders[wb_id]->set_value();
    wbRedSliders[wb_id]->set_inhibit(false);
    wbGreenSliders[wb_id]->set_inhibit(true);
    wbGreenSliders[wb_id]->get_adjustment()->set_value(cam_mul[1]);
    wbGreenSliders[wb_id]->set_value();
    wbGreenSliders[wb_id]->set_inhibit(false);
    wbBlueSliders[wb_id]->set_inhibit(true);
    wbBlueSliders[wb_id]->get_adjustment()->set_value(cam_mul[2]);
    wbBlueSliders[wb_id]->set_value();
    wbBlueSliders[wb_id]->set_inhibit(false);
    /*
    switch( wb_id ) {
    case PF::WB_SPOT:
    case PF::WB_COLOR_SPOT:
      // In the spot WB case we directly set the WB multitpliers
      wbRedSlider.set_inhibit(true);
      wbRedSlider.get_adjustment()->set_value(cam_mul[0]);
      wbRedSlider.set_value();
      wbRedSlider.set_inhibit(false);
      wbGreenSliders.set_inhibit(true);
      wbGreenSlider.get_adjustment()->set_value(cam_mul[1]);
      wbGreenSlider.set_value();
      wbGreenSlider.set_inhibit(false);
      wbBlueSlider.set_inhibit(true);
      wbBlueSlider.get_adjustment()->set_value(cam_mul[2]);
      wbBlueSlider.set_value();
      wbBlueSlider.set_inhibit(false);
      break;
    default:
      // Otherwise set the WB multitplier corrections
      wbRedCorrSlider.set_inhibit(true);
      wbRedCorrSlider.get_adjustment()->set_value(cam_mul[0]/preset_wb[0]);
      wbRedCorrSlider.set_value();
      wbRedCorrSlider.set_inhibit(false);
      wbGreenCorrSlider.set_inhibit(true);
      wbGreenCorrSlider.get_adjustment()->set_value(cam_mul[1]/preset_wb[1]);
      wbGreenCorrSlider.set_value();
      wbGreenCorrSlider.set_inhibit(false);
      wbBlueCorrSlider.set_inhibit(true);
      wbBlueCorrSlider.get_adjustment()->set_value(cam_mul[2]/preset_wb[2]);
      wbBlueCorrSlider.set_value();
      wbBlueCorrSlider.set_inhibit(false);
      break;
    }
     */
    get_layer()->get_image()->update();
  }
}


void PF::RawDeveloperConfigGUI::do_update()
{
  tmp_area.clear();
  //std::cout<<"RawDeveloperConfigGUI::do_update() called."<<std::endl;
  if( !(get_layer()) || !(get_layer()->get_image()) ||
      !(get_layer()->get_processor()) ||
      !(get_layer()->get_processor()->get_par()) ) {
    OperationConfigGUI::do_update();
    return;
  }

  PF::RawDeveloperPar* par =
      dynamic_cast<PF::RawDeveloperPar*>(get_layer()->get_processor()->get_par());
  if( !par ) return;

  PropertyBase* prop = par->get_property( "wb_mode" );
  if( !prop )  return;

  bool is_xtrans = false;
  PF::exif_data_t* exif_data = NULL;
  dcraw_data_t* raw_data = NULL;
  unsigned long int* raw_hist = NULL;

  PF::Image* image = get_layer()->get_image();
  PF::Pipeline* pipeline = image->get_pipeline(0);
  PF::PipelineNode* node = NULL;
  PF::PipelineNode* inode = NULL;
  PF::ProcessorBase* processor = NULL;
  std::string maker, model;
  if( pipeline ) node = pipeline->get_node( get_layer()->get_id() );
  if( node ) inode = pipeline->get_node( node->input_id );
  if( node ) processor = node->processor;
  if( inode && inode->image) {
    size_t blobsz;
    if( PF_VIPS_IMAGE_GET_BLOB( inode->image, PF_META_EXIF_NAME, &exif_data, &blobsz ) ||
        blobsz != sizeof(PF::exif_data_t) ) {
      exif_data = NULL;
    }
    if( PF_VIPS_IMAGE_GET_BLOB( inode->image, "raw_image_data", &raw_data, &blobsz ) ||
        blobsz != sizeof(dcraw_data_t) ) {
      raw_data = NULL;
    }
    if( PF_VIPS_IMAGE_GET_BLOB( inode->image, "raw-hist", &raw_hist, &blobsz ) ||
        blobsz != (sizeof(unsigned long int)*65536*3) ) {
      std::cout<<"[RawDeveloperConfigGUI::do_update] raw_hist="<<raw_hist<<"  blobsz="<<blobsz<<std::endl;
      raw_hist = NULL;
    }
    histogramArea.hist = raw_hist;
    histogramArea.black = 128;
    histogramArea.white = 3700;
    if( histogram_range_full ) {
      histogramArea.zoom_black = false;
      histogramArea.zoom_white = false;
    }
    if( histogram_range_sh ) {
      histogramArea.zoom_black = true;
      histogramArea.zoom_white = false;
    }
    if( histogram_range_hi ) {
      histogramArea.zoom_black = false;
      histogramArea.zoom_white = true;
    }
    histogramArea.queue_draw();

    if( exif_data && raw_data ) {
      //char makermodel[1024];
      //char *tmodel = makermodel;
      //dt_colorspaces_get_makermodel_split(makermodel, sizeof(makermodel), &tmodel,
      //    exif_data->exif_maker, exif_data->exif_model );
      maker = exif_data->camera_maker;
      model = exif_data->camera_model;
      wbModeSelector.set_maker_model( maker, model );
      //std::cout<<"RawDeveloperConfigGUI::do_update(): maker="<<maker<<" model="<<model<<std::endl;

      if( processor ) {
        PF::RawDeveloperPar* par2 =
            dynamic_cast<PF::RawDeveloperPar*>(processor->get_par());
        if( par2 ) {
          // Initialize the WB coefficients from pipeline #0 the first time
          // we update the widgets

          PF::RawPreprocessorPar* rppar = par->get_rawpreprocessor_par();
#ifndef NDEBUG
          std::cout<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl;
          std::cout<<"RawDeveloperConfigGUI::do_update(): rppar="<<rppar<<std::endl;
#endif
          if( rppar ) {
#ifndef NDEBUG
            std::cout<<"RawDeveloperConfigGUI::do_update(): rppar->get_preset_wb_red(PF::WB_CAMERA)="
                <<rppar->get_preset_wb_red(PF::WB_CAMERA)<<std::endl;
#endif
            if( rppar->get_preset_wb_red(PF::WB_CAMERA) <= 0 ) {
              rppar->init_wb_coefficients( par2->get_image_data(), maker, model );
              for( unsigned int i = 0; i < PF::WB_LAST; i++ ) {
                wbRedSliders[i]->init();
                wbGreenSliders[i]->init();
                wbBlueSliders[i]->init();
              }
            }
          }

          //dt_colorspaces_get_makermodel( makermodel, sizeof(makermodel), exif_data->exif_maker, exif_data->exif_model );
          //std::cout<<"RawOutputPar::build(): makermodel="<<makermodel<<std::endl;
          //float xyz_to_cam[4][3];
          //xyz_to_cam[0][0] = NAN;
          //dt_dcraw_adobe_coeff(exif_data->camera_makermodel, (float(*)[12])xyz_to_cam);
          if(!std::isnan(raw_data->color.cam_xyz[0][0])) {
            for(int i = 0; i < 3; i++) {
              for(int j = 0; j < 3; j++) {
                XYZ_to_CAM[i][j] = (double)raw_data->color.cam_xyz[i][j];
              }
            }
          }
#ifndef NDEBUG
          printf("RawDeveloperConfigGUI::do_update(): xyz_to_cam:\n");
          for(int k = 0; k < 3; k++) {
            //printf("    %.4f %.4f %.4f\n",xyz_to_cam[k][0],xyz_to_cam[k][1],xyz_to_cam[k][2]);
            printf("    %.4f %.4f %.4f\n",raw_data->color.cam_xyz[k][0],raw_data->color.cam_xyz[k][1],raw_data->color.cam_xyz[k][2]);
          }
#endif

          // and inverse matrix
          mat3inv_double((double *)CAM_to_XYZ, (double *)XYZ_to_CAM);

          double temp, tint;
          par2->get_wb( preset_wb );
#ifndef NDEBUG
          std::cout<<"PF::RawDeveloperConfigGUI::do_update(): preset WB="
              <<preset_wb[0]<<","<<preset_wb[1]<<","<<preset_wb[2]<<std::endl;
#endif
          float real_wb[3];
          for( int i = 0; i < 3; i++ ) real_wb[i] = preset_wb[i];
          if( par2->get_wb_mode() != PF::WB_SPOT &&
              par2->get_wb_mode() != PF::WB_COLOR_SPOT ) {
            // we are using one of the WB presets, so we have to take into account the
            // WB correction sliders as well
            real_wb[0] *= wbRedCorrSlider.get_adjustment()->get_value();
            real_wb[1] *= wbGreenCorrSlider.get_adjustment()->get_value();
            real_wb[2] *= wbBlueCorrSlider.get_adjustment()->get_value();
          }
#ifndef NDEBUG
          std::cout<<"PF::RawDeveloperConfigGUI::do_update(): real WB="
              <<real_wb[0]<<","<<real_wb[1]<<","<<real_wb[2]<<std::endl;
#endif
          mul2temp( real_wb, &temp, &tint );

          ignore_temp_tint_change = true;
          wbTempSlider.get_adjustment()->set_value( temp );
          wbTintSlider.get_adjustment()->set_value( tint );
          ignore_temp_tint_change = false;
        }
      }
    }

    //dcraw_data_t* raw_data;
    if( raw_data ) {
      is_xtrans = PF::check_xtrans( raw_data->idata.filters );

      float black_level = raw_data->color.black;
      float white_level = raw_data->color.maximum;

      PropertyBase* blc = par->get_property( "black_level_correction" );
      float black_corr = 1;
      if( blc ) blc->get( black_corr );

      PropertyBase* wlc = par->get_property( "saturation_level_correction" );
      float white_corr = 1;
      if( wlc ) wlc->get( white_corr );

      float black_level2 = black_level * (black_corr);
      float white_level2 = white_level * (white_corr);

      char tstr[500];
      snprintf( tstr, 499, "RAW black: %.1f (def. = %.0f)",
          black_level2, black_level );
      black_level_label.set_text( tstr );
      snprintf( tstr, 499, "RAW white: %.1f (def. = %.0f)",
          white_level2, white_level );
      white_level_label.set_text( tstr );
    }
  }

  //std::cout<<"PF::RawDeveloperConfigGUI::do_update() called."<<std::endl;

  if( wbTargetBox.get_parent() == &wbControlsBox )
    wbControlsBox.remove( wbTargetBox );
  if( wb_best_match_label.get_parent() == &wbControlsBox )
    wbControlsBox.remove( wb_best_match_label );
  /*
    if( wbRedSlider.get_parent() == &wbControlsBox )
      wbControlsBox.remove( wbRedSlider );
    if( wbGreenSlider.get_parent() == &wbControlsBox )
      wbControlsBox.remove( wbGreenSlider );
    if( wbBlueSlider.get_parent() == &wbControlsBox )
      wbControlsBox.remove( wbBlueSlider );
   */
  for( unsigned int i = 0; i < PF::WB_LAST; i++ ) {
    wbSliderBoxes[i].hide();
  }
  wbSliderBoxes[prop->get_enum_value().first].show();

  if( wbRedCorrSlider.get_parent() == &wbControlsBox )
    wbControlsBox.remove( wbRedCorrSlider );
  if( wbGreenCorrSlider.get_parent() == &wbControlsBox )
    wbControlsBox.remove( wbGreenCorrSlider );
  if( wbBlueCorrSlider.get_parent() == &wbControlsBox )
    wbControlsBox.remove( wbBlueCorrSlider );

  switch( prop->get_enum_value().first ) {
  case PF::WB_SPOT:
    if( wbTargetBox.get_parent() == &wbControlsBox )
      wbControlsBox.remove( wbTargetBox );
    if( wb_best_match_label.get_parent() == &wbControlsBox )
      wbControlsBox.remove( wb_best_match_label );
    /*if( wbRedSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbRedSlider, Gtk::PACK_SHRINK );
      if( wbGreenSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbGreenSlider, Gtk::PACK_SHRINK );
      if( wbBlueSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbBlueSlider, Gtk::PACK_SHRINK );*/
    break;
  case PF::WB_COLOR_SPOT:
    if( wbTargetBox.get_parent() != &wbControlsBox )
      wbControlsBox.pack_start( wbTargetBox, Gtk::PACK_SHRINK );
    if( wb_best_match_label.get_parent() != &wbControlsBox )
      wbControlsBox.pack_start( wb_best_match_label, Gtk::PACK_SHRINK );
    /*if( wbRedSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbRedSlider, Gtk::PACK_SHRINK );
      if( wbGreenSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbGreenSlider, Gtk::PACK_SHRINK );
      if( wbBlueSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbBlueSlider, Gtk::PACK_SHRINK );*/
    break;
  default:
    /*if( wbRedCorrSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbRedCorrSlider, Gtk::PACK_SHRINK );
      if( wbGreenCorrSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbGreenCorrSlider, Gtk::PACK_SHRINK );
      if( wbBlueCorrSlider.get_parent() != &wbControlsBox )
        wbControlsBox.pack_start( wbBlueCorrSlider, Gtk::PACK_SHRINK );*/
    break;
  }

  if( is_xtrans ) demoMethodSelector.hide();
  else demoMethodSelector.show();


  // Lens corrections
  //if( custom_cam_maker.empty() && custom_cam_model.empty() && custom_lens_model.empty() ) {
    custom_cam_maker = par->get_lf_maker();
    custom_cam_model = par->get_lf_model();
    custom_lens_model = par->get_lf_lens();
    //std::cout<<"RawDeveloperConfigGUI::do_update(): camera=\""<<custom_cam_maker<<"\", \""<<custom_cam_model<<"\""<<std::endl;
    //std::cout<<"RawDeveloperConfigGUI::do_update(): lens=\""<<custom_lens_model<<"\""<<std::endl;
  //}
  if( lf_auto_matching_checkbox.get_active() ) {
    lf_cam_selector.disable();
    lf_lens_selector.disable();
    lf_selector.disable();
    if( exif_data ) {
      Glib::ustring cam_make, cam_model, lens_model;
      cam_make = exif_data->exif_maker;
      cam_model = exif_data->exif_model;
      lens_model = exif_data->exif_lens;
      lf_cam_selector.set_cam( cam_make, cam_model );
      lf_lens_selector.set_lens( cam_make, cam_model, lens_model );
      lf_selector.set_cam( cam_make, cam_model );
      lf_selector.set_lens( lens_model );
    }
  } else {
    if( custom_cam_maker.empty() && custom_cam_model.empty() && custom_lens_model.empty() ) {
      Glib::ustring cam_make, cam_model, lens_model;
      cam_make = exif_data->exif_maker;
      cam_model = exif_data->exif_model;
      lens_model = exif_data->exif_lens;
      lf_cam_selector.set_cam( cam_make, cam_model );
      lf_lens_selector.set_lens( cam_make, cam_model, lens_model );
      lf_cam_selector.set_value();
      lf_lens_selector.set_value();
      lf_selector.set_cam( cam_make, cam_model );
      lf_selector.set_lens( lens_model );
      lf_selector.set_value();
    } else {
      lf_cam_selector.get_value();
      lf_lens_selector.get_value();
      lf_selector.get_value();
    }
    lf_cam_selector.enable();
    lf_lens_selector.enable();
    lf_selector.enable();
  }
  if( processor ) {
    PF::RawDeveloperPar* par2 =
        dynamic_cast<PF::RawDeveloperPar*>(processor->get_par());
    if( par2 ) {
      lf_makerEntry.set_text( par2->get_lf_maker() );
      lf_makerEntry.set_tooltip_text( par2->get_lf_maker() );

      lf_modelEntry.set_text( par2->get_lf_model() );
      lf_modelEntry.set_tooltip_text( par2->get_lf_model() );

      lf_lensEntry.set_text( par2->get_lf_lens() );
      lf_lensEntry.set_tooltip_text( par2->get_lf_lens() );
    }
  }
  /*
  if( par->get_all_enabled() ) {
    lf_enable_distortion_button.hide();
    lf_enable_tca_button.hide();
    lf_enable_vignetting_button.hide();
  } else {
    lf_enable_distortion_button.show();
    lf_enable_tca_button.show();
    lf_enable_vignetting_button.show();
  }
  */
  if( par->get_tca_enabled() || par->get_all_enabled() ) {
    ca_mode_selector.show();
    if( par->get_tca_method() == PF::PF_TCA_CORR_MANUAL )
      ca_box.show();
    else
      ca_box.hide();
  } else {
    ca_mode_selector.hide();
    ca_box.hide();
  }

  prop = par->get_property( "cam_profile_name" );
  if( !prop )  return;
  std::string filename = prop->get_str();
  camProfFileEntry.set_text( filename.c_str() );

  prop = par->get_property( "cam_dcp_profile_name" );
  if( !prop )  return;
  filename = prop->get_str();
  camDCPProfFileEntry.set_text( filename.c_str() );

  prop = par->get_property( "out_profile_name" );
  if( !prop )  return;
  filename = prop->get_str();
  outProfFileEntry.set_text( filename.c_str() );

  prop = par->get_property( "profile_mode" );
  if( !prop )  return;

  switch( prop->get_enum_value().first ) {
  case PF::IN_PROF_NONE:
    camProfHBox.hide();
    gammaModeHBox.show();
    camDCPProfHBox.hide();
    dcp_options_box.hide();
    outProfFrame.hide();
    break;
  case PF::IN_PROF_MATRIX:
    camProfHBox.hide();
    gammaModeHBox.hide();
    camDCPProfHBox.hide();
    dcp_options_box.hide();
    outProfFrame.show();
    break;
  case PF::IN_PROF_ICC:
    camProfHBox.show();
    gammaModeHBox.show();
    camDCPProfHBox.hide();
    dcp_options_box.hide();
    outProfFrame.show();
    break;
  case PF::IN_PROF_DCP:
    camProfHBox.hide();
    gammaModeHBox.hide();
    camDCPProfHBox.show();
    dcp_options_box.show();
    outProfFrame.show();
    break;
  }

  prop = par->get_property( "out_profile_mode" );
  if( !prop )  return;

  if( prop->get_enum_value().first == PF::PROF_TYPE_EMBEDDED ||
      prop->get_enum_value().first == PF::PROF_TYPE_FROM_SETTINGS ) {
    outProfileTypeSelectorBox.hide();
    outTRCTypeSelectorBox.hide();
    outProfHBox.hide();
  } else if( prop->get_enum_value().first == PF::PROF_TYPE_FROM_DISK ) {
    outProfileTypeSelectorBox.hide();
    outTRCTypeSelectorBox.hide();
    outProfHBox.show();
  } else {//if( prop->get_enum_value().first == PF::PROF_MODE_CUSTOM ) {
    outProfileTypeSelectorBox.show();
    outTRCTypeSelectorBox.show();
    outProfHBox.hide();
  }

  OperationConfigGUI::do_update();
}


/*
void PF::RawDeveloperConfigGUI::pointer_press_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return;
}
 */


void PF::RawDeveloperConfigGUI::spot_wb( double x, double y )
{
  // Get the layer associated to this operation
  PF::Layer* l = get_layer();
  if( !l ) return;

  if( !l->get_processor() ) return;
  PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>( l->get_processor()->get_par() );
  if( !par ) return;


  // Get the image the layer belongs to
  PF::Image* img = l->get_image();
  if( !img ) return;

  // Get the default pipeline of the image 
  // (it is supposed to be at 1:1 zoom level 
  // and floating point accuracy)
  PF::Pipeline* pipeline = img->get_pipeline( 0 );
  if( !pipeline ) return;

  // Make sure the first pipeline is up-to-date
  //img->update( pipeline, false );
  //img->unlock();

  // Get the node associated to the layer
  PF::PipelineNode* node = pipeline->get_node( l->get_id() );
  if( !node ) return;
  if( !(node->processor) ) return;
  if( !(node->processor->get_par()) ) return;

  // Finally, get the underlying VIPS image associated to the layer
  VipsImage* image = node->image;
  if( !image ) return;

  //PF::PropertyBase* pwb_red = node->processor->get_par()->get_property("wb_red");
  //PF::PropertyBase* pwb_green = node->processor->get_par()->get_property("wb_green");
  //PF::PropertyBase* pwb_blue = node->processor->get_par()->get_property("wb_blue");

  //if( !pwb_red || !pwb_green || !pwb_blue ) return;

  PropertyBase* prop = par->get_property( "wb_mode" );
  if( !prop )  return;
  int wb_id = prop->get_enum_value().first;

  PropertyBase* wb_red_prop = wbRedSliders[wb_id]->get_prop();
  PropertyBase* wb_green_prop = wbGreenSliders[wb_id]->get_prop();
  PropertyBase* wb_blue_prop = wbBlueSliders[wb_id]->get_prop();
  if( !wb_red_prop || !wb_green_prop || !wb_blue_prop )
    return;

  PF::RawDeveloperPar* node_par = dynamic_cast<PF::RawDeveloperPar*>( node->processor->get_par() );
  if( !node_par ) return;

  par->set_caching( false );

  // We need to retrieve the input ICC profile for the RGB conversion later on
  cmsHPROFILE profile_in = NULL;
  cmsHPROFILE profile_out = NULL;
  cmsHTRANSFORM transform = NULL;
  void *data;
  size_t data_length;
  if( !PF_VIPS_IMAGE_GET_BLOB( image, VIPS_META_ICC_NAME, &data, &data_length ) ) {

    profile_in = cmsOpenProfileFromMem( data, data_length );
  }
  if( profile_in ) {

    //#ifndef NDEBUG
    char tstr2[1024];
    cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr2, 1024);
    std::cout<<"raw_developer: embedded profile found: "<<tstr2<<std::endl;
    //#endif

    std::string outprofname = PF::PhotoFlow::Instance().get_data_dir() + "/icc/ACES-elle-V4-g10.icc";
    profile_out = cmsOpenProfileFromFile( outprofname.c_str(), "r" );
    if( !profile_out )
      return;

    cmsUInt32Number infmt = vips2lcms_pixel_format( image->BandFmt, profile_in );
    cmsUInt32Number outfmt = TYPE_RGB_FLT;

    transform = cmsCreateTransform( profile_in,
        infmt,
        profile_out,
        outfmt,
        INTENT_RELATIVE_COLORIMETRIC, cmsFLAGS_NOCACHE );
  }

  std::cout<<"profile_in: "<<profile_in<<std::endl;
  std::cout<<"transform:  "<<transform<<std::endl;
  int sample_size = 7;

  float in_check[3] = { 0, 0, 0 };
  float rgb_check[3] = { 0, 0, 0 };
  float rgb_prev[3] = { 1000, 1000, 1000 };

  std::vector<VipsRect> areas;
  if( is_area_wb() ) {
    for(int ai = 0; ai < par->get_wb_areas().size(); ai++) {
      int left = par->get_wb_areas()[ai][0];
      int top = par->get_wb_areas()[ai][1];
      int width = par->get_wb_areas()[ai][2] - par->get_wb_areas()[ai][0] + 1;
      int height = par->get_wb_areas()[ai][3] - par->get_wb_areas()[ai][1] + 1;
      VipsRect area = {left, top, width, height};
      areas.push_back(area);
    }
  } else {
    int left = (int)x-3;
    int top = (int)y-3;
    int width = 7;
    int height = 7;
    VipsRect area = {left, top, width, height};
    areas.push_back(area);
    //std::cout<<"RawDeveloperConfigGUI: getting spot WB ("<<x<<","<<y<<")"<<std::endl;
  }

  for( int i = 0; i < 100; i++ ) {
    // Now we have to process a small portion of the image 
    // to get the corresponding Lab values
    VipsImage* spot;

    float in_avg[3] = {0, 0, 0};
    float rgb_avg[3] = {0, 0, 0};
    std::vector<float> values;

    img->sample( l->get_id(), areas, is_area_wb(), NULL, values );
    if( values.size() != 3 ) {
      std::cout<<"RawDeveloperConfigGUI::pointer_relese_event(): values.size() "
          <<values.size()<<" (!= 3)"<<std::endl;
      return;
    }
    in_avg[0] = values[0];
    in_avg[1] = values[1];
    in_avg[2] = values[2];
    if( transform ) {
      if( cmsGetColorSpace(profile_in) == cmsSigLabData )
        PF::Lab_pf2lcms( in_avg );
      cmsDoTransform( transform, in_avg, rgb_avg, 1 );
    } else {
      rgb_avg[0] = in_avg[0];
      rgb_avg[1] = in_avg[1];
      rgb_avg[2] = in_avg[2];
    }

    //#ifndef NDEBUG
    std::cout<<" sampled("<<i<<"): "<<rgb_avg[0]<<" "<<rgb_avg[1]<<" "<<rgb_avg[2]<<std::endl;
    //#endif

    float rgb_out[3] = {0, 0, 0};

    const float epsilon = 1.0e-5;
    float wb_red_mul;
    float wb_green_mul;
    float wb_blue_mul;

    // The target color is gray, so we simply neutralize the spot value
    // The green channel is kept fixed and the other two are scaled to
    // the green value
    wb_red_mul = rgb_avg[1]/rgb_avg[0];
    wb_blue_mul = rgb_avg[1]/rgb_avg[2];
    wb_green_mul = 1;

    // Limit the values of correction coefficients
    // to compensate for non-linear RGB values
    if(wb_red_mul > 1.5) wb_red_mul = 1.5;
    if(wb_blue_mul > 1.5) wb_blue_mul = 1.5;
    if(wb_red_mul < 0.66) wb_red_mul = 0.66;
    if(wb_blue_mul < 0.66) wb_blue_mul = 0.66;

    //PropertyBase* wb_red_prop = wbRedSlider.get_prop();
    //PropertyBase* wb_green_prop = wbGreenSlider.get_prop();
    //PropertyBase* wb_blue_prop = wbBlueSlider.get_prop();
    if( wb_red_prop && wb_green_prop && wb_blue_prop ) {
      float wb_red_in;
      float wb_green_in;
      float wb_blue_in;
      wb_red_prop->get( wb_red_in );
      wb_green_prop->get( wb_green_in );
      wb_blue_prop->get( wb_blue_in );
      float wb_red_out = wb_red_mul*wb_red_in;
      float wb_green_out = wb_green_mul*wb_green_in;
      float wb_blue_out = wb_blue_mul*wb_blue_in;
      float scale = (wb_red_out+wb_green_out+wb_blue_out)/3.0f;
      //scale = 1;
      std::cout<<" WB coefficients (1): "<<wb_red_in<<"*"<<wb_red_mul<<" -> "<<wb_red_out<<std::endl
          <<"                      "<<wb_green_in<<"*"<<wb_green_mul<<" -> "<<wb_green_out<<std::endl
          <<"                      "<<wb_blue_in<<"*"<<wb_blue_mul<<" -> "<<wb_blue_out<<std::endl;
      std::cout<<"  scale: "<<scale<<std::endl;
      //float scale = wb_green_mul;
      wb_red_out /= scale;
      wb_green_out /= scale;
      wb_blue_out /= scale;
      wb_red_prop->update( wb_red_out );
      wb_green_prop->update( wb_green_out );
      wb_blue_prop->update( wb_blue_out );

      std::cout<<" WB coefficients (2): "<<wb_red_in<<"*"<<wb_red_mul<<" -> "<<wb_red_out<<std::endl
          <<"                      "<<wb_green_in<<"*"<<wb_green_mul<<" -> "<<wb_green_out<<std::endl
          <<"                      "<<wb_blue_in<<"*"<<wb_blue_mul<<" -> "<<wb_blue_out<<std::endl;

      wbRedSliders[wb_id]->init();
      wbGreenSliders[wb_id]->init();
      wbBlueSliders[wb_id]->init();

      //pwb_red->update( wbRedSlider.get_adjustment()->get_value() );
      //pwb_green->update( wbGreenSlider.get_adjustment()->get_value() );
      //pwb_blue->update( wbBlueSlider.get_adjustment()->get_value() );

      //node_par->set_wb( wbRedSlider.get_adjustment()->get_value(), wbGreenSlider.get_adjustment()->get_value(), wbBlueSlider.get_adjustment()->get_value() );

      node_par->set_modified();
      img->update( pipeline, true );
      //img->unlock();
    }

    //std::cout<<"RawDeveloperConfigGUI: checking spot WB"<<std::endl;
    img->sample( l->get_id(), areas, is_area_wb(), NULL, values );
    if( values.size() != 3 ) {
      std::cout<<"RawDeveloperConfigGUI::pointer_relese_event(): values.size() "
          <<values.size()<<" (!= 3)"<<std::endl;
      return;
    }
    in_check[0] = values[0];
    in_check[1] = values[1];
    in_check[2] = values[2];
    if( transform ) {
      cmsDoTransform( transform, in_check, rgb_check, 1 );
    } else {
      rgb_check[0] = in_check[0];
      rgb_check[1] = in_check[1];
      rgb_check[2] = in_check[2];
    }

    std::cout<<" rgb check("<<i<<"): "<<rgb_check[0]<<" "<<rgb_check[1]<<" "<<rgb_check[2]<<std::endl;

    if( i == 0 ) continue;
    float delta_r = rgb_check[0] - rgb_prev[1];
    float delta_g = rgb_check[1] - rgb_prev[1];
    float delta_b = rgb_check[2] - rgb_prev[2];
    if( (fabs(delta_r) < 0.001) && (fabs(delta_g) < 0.001) && (fabs(delta_b) < 0.001) )
      break;
    rgb_prev[0] = rgb_check[0];
    rgb_prev[1] = rgb_check[1];
    rgb_prev[2] = rgb_check[2];
  }

  cmsDeleteTransform( transform );
  cmsCloseProfile( profile_in );
  cmsCloseProfile( profile_out );

  par->set_caching( true );
  l->set_dirty(true);
  par->set_modified();
  // Update the prepipeline to reflect the new settings
  img->update();
}



void PF::RawDeveloperConfigGUI::color_spot_wb( double x, double y )
{
  // Get the layer associated to this operation
  PF::Layer* l = get_layer();
  if( !l ) return;

  if( !l->get_processor() ) return;
  PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>( l->get_processor()->get_par() );
  if( !par ) return;


  // Get the image the layer belongs to
  PF::Image* img = l->get_image();
  if( !img ) return;

  // Get the default pipeline of the image 
  // (it is supposed to be at 1:1 zoom level 
  // and same accuracy as the preview one)
  PF::Pipeline* pipeline = img->get_pipeline( 0 );
  if( !pipeline ) return;

  // Make sure the first pipeline is up-to-date
  //img->update( pipeline, true );
  //img->update( NULL, true );
  //img->unlock();

  // Get the node associated to the layer
  PF::PipelineNode* node = pipeline->get_node( l->get_id() );
  if( !node ) return;

  // Finally, get the underlying VIPS image associated to the layer
  VipsImage* image = node->image;
  if( !image ) return;

  PropertyBase* prop = par->get_property( "wb_mode" );
  if( !prop )  return;
  int wb_id = prop->get_enum_value().first;

  PropertyBase* wb_red_prop = wbRedSliders[wb_id]->get_prop();
  PropertyBase* wb_green_prop = wbGreenSliders[wb_id]->get_prop();
  PropertyBase* wb_blue_prop = wbBlueSliders[wb_id]->get_prop();
  if( !wb_red_prop || !wb_green_prop || !wb_blue_prop ) 
    return;

  // We need to retrieve the input ICC profile for the Lab conversion later on
  void *data;
  size_t data_length;
  if( PF_VIPS_IMAGE_GET_BLOB( image, VIPS_META_ICC_NAME, &data, &data_length ) )
    return;

  cmsHPROFILE profile_in = cmsOpenProfileFromMem( data, data_length );
  if( !profile_in ) 
    return;

#ifndef NDEBUG
  char tstr2[1024];
  cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr2, 1024);
  std::cout<<"raw_developer: embedded profile found: "<<tstr2<<std::endl;
#endif

  cmsCIExyY white;
  cmsWhitePointFromTemp( &white, 6500 );
  //cmsHPROFILE profile_out = cmsCreateLab4Profile( &white );
  cmsHPROFILE profile_out = cmsCreateLab4Profile( NULL );

  cmsUInt32Number infmt = TYPE_RGB_FLT;
  cmsUInt32Number outfmt = TYPE_Lab_FLT;

  cmsHTRANSFORM transform = cmsCreateTransform( profile_in, 
      infmt,
      profile_out,
      outfmt,
      INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE );
  if( !transform )
    return;

  cmsHTRANSFORM transform_inv = cmsCreateTransform( profile_out, 
      outfmt,
      profile_in,
      infmt,
      INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE );
  if( !transform_inv )
    return;

  //x = 2800; y = 654;

  PF::raw_preproc_sample_x = x;
  PF::raw_preproc_sample_y = y;


  float wb_red_mul = 1;
  float wb_green_mul = 1;
  float wb_blue_mul = 1;
  float wb_red_mul_prev = 1;
  float wb_green_mul_prev = 1;
  float wb_blue_mul_prev = 1;

  PF::RawDeveloperPar* node_par = dynamic_cast<PF::RawDeveloperPar*>( node->processor->get_par() );
  if( !node_par ) return;

  par->set_caching( false );

  float Lab_check[3] = { 0, 0, 0 };
  float Lab_prev[3] = { 0, 1000, 1000 };
  for( int i = 0; i < 100; i++ ) {
    // Now we have to process a small portion of the image 
    // to get the corresponding Lab values
    VipsImage* spot;
    int left = (int)x-3;
    int top = (int)y-3;
    int width = 7;
    int height = 7;

    float wb_red_in;
    float wb_green_in;
    float wb_blue_in;
    wb_red_prop->get( wb_red_in );
    wb_green_prop->get( wb_green_in );
    wb_blue_prop->get( wb_blue_in );
    float norm_in = MIN3(wb_red_in,wb_green_in,wb_blue_in);
    //wb_red_in /= norm_in;
    //wb_green_in /= norm_in;
    //wb_blue_in /= norm_in;

    float wb_red_out;
    float wb_green_out;
    float wb_blue_out;

    /*
    VipsRect crop = {left, top, width, height};
    VipsRect all = {0 ,0, image->Xsize, image->Ysize};
    VipsRect clipped;
    vips_rect_intersectrect( &crop, &all, &clipped );

    if( vips_crop( image, &spot, 
                   clipped.left, clipped.top,
                   clipped.width, clipped.height,
                   NULL ) )
      return;

    VipsRect rspot = {0 ,0, spot->Xsize, spot->Ysize};

    VipsImage* outimg = im_open( "spot_wb_img", "p" );
    if (vips_sink_screen (spot, outimg, NULL,
                          64, 64, 1,
                          0, NULL, this))
      return;
    VipsRegion* region = vips_region_new( outimg );
    if (vips_region_prepare (region, &rspot))
      return;
     */
    //if( vips_sink_memory( spot ) )
    //  return;

    int sample_size = 7;
    int row, col;
    float* p;
    float red, green, blue;
    float rgb_avg[3] = {0, 0, 0};
    std::vector<float> values;

    std::cout<<std::endl<<std::endl<<"==============================================="<<std::endl;
    std::cout<<"RawDeveloperConfigGUI: getting color spot WB"<<std::endl;
    /*
    int line_size = clipped.width*3;
    for( row = 0; row < rspot.height; row++ ) {
      p = (float*)VIPS_REGION_ADDR( region, rspot.left, rspot.top );
      for( col = 0; col < line_size; col += 3 ) {
        red = p[col];      rgb_avg[0] += red;
        green = p[col+1];  rgb_avg[1] += green;
        blue = p[col+2];   rgb_avg[2] += blue;
        //std::cout<<"  pixel="<<row<<","<<col<<"    red="<<red<<"  green="<<green<<"  blue="<<blue<<std::endl;
      }
    }
    rgb_avg[0] /= rspot.width*rspot.height;
    rgb_avg[1] /= rspot.width*rspot.height;
    rgb_avg[2] /= rspot.width*rspot.height;
     */
    std::cout<<"RawDeveloperConfigGUI: getting color spot WB ("<<x<<","<<y<<")"<<std::endl;
    img->sample( l->get_id(), x, y, sample_size, NULL, values );
    //values.clear(); img->sample( l->get_id(), x, y, sample_size, NULL, values );
    if( values.size() != 3 ) {
      std::cout<<"RawDeveloperConfigGUI::pointer_relese_event(): values.size() "
          <<values.size()<<" (!= 3)"<<std::endl;
      return;
    }
    rgb_avg[0] = values[0];
    rgb_avg[1] = values[1];
    rgb_avg[2] = values[2];

    std::cout<<" RGB in: "<<rgb_avg[0]*255<<" "<<rgb_avg[1]*255<<" "<<rgb_avg[2]*255<<std::endl;

    float rgb_out[3] = {0, 0, 0};
    float Lab_in[3] = {0, 0, 0};
    float Lab_out[3] = {0, 0, 0};
    float Lab_wb[3] = {
        static_cast<float>(wb_target_L_slider.get_adjustment()->get_value()),
        static_cast<float>(wb_target_a_slider.get_adjustment()->get_value()),
        static_cast<float>(wb_target_b_slider.get_adjustment()->get_value())
    };
    //float Lab_wb[3] = {70, 15, 10};
    // Now we convert the average RGB values in the WB spot region to Lab
    cmsDoTransform( transform, rgb_avg, Lab_in, 1 );

    std::cout<<" Lab in: "<<Lab_in[0]<<" "<<Lab_in[1]<<" "<<Lab_in[2]<<std::endl;
    //return;

    const float epsilon = 1.0e-5;
    float ab_zero = 0;
    //float ab_zero = 0.5;
    float delta1 = Lab_in[1] - ab_zero;
    float delta2 = Lab_in[2] - ab_zero;

    float wb_delta1 = Lab_wb[1] - ab_zero;
    float wb_delta2 = Lab_wb[2] - ab_zero;

    if( (fabs(wb_delta1) < epsilon) &&
        (fabs(wb_delta2) < epsilon) ) {

      // The target color is gray, so we simply neutralize the spot value
      // The green channel is kept fixed and the other two are scaled to 
      // the green value
      wb_red_mul = rgb_avg[1]/rgb_avg[0];
      wb_blue_mul = rgb_avg[1]/rgb_avg[2];
      wb_green_mul = 1;

    } else if( fabs(wb_delta1) < epsilon ) {

      // The target "a" channel is very close to the neutral value,
      // in this case we set the ouput "a" channel equal to the target one
      // and we eventually invert the "b" channel if the input sign is opposite
      // to the target one, without applying any scaling
      Lab_out[0] = Lab_in[0];
      Lab_out[1] = Lab_wb[1];
      Lab_out[2] = Lab_in[2];
      if( delta2*wb_delta2 < 0 )
        Lab_out[2] = -Lab_in[2];

      // Now we convert back to RGB and we compute the multiplicative
      // factors that bring from the current WB to the target one
      cmsDoTransform( transform_inv, Lab_out, rgb_out, 1 );
      wb_red_mul = rgb_out[0]/rgb_avg[0];
      wb_green_mul = rgb_out[1]/rgb_avg[1];
      wb_blue_mul = rgb_out[2]/rgb_avg[2];

    } else if( fabs(wb_delta2) < epsilon ) {

      // The target "b" channel is very close to the neutral value,
      // in this case we set the ouput "b" channel equal to the target one
      // and we eventually invert the "a" channel if the input sign is opposite
      // to the target one, without applying any scaling
      Lab_out[0] = Lab_in[0];
      Lab_out[1] = Lab_in[1];
      Lab_out[2] = Lab_wb[2];
      if( delta1*wb_delta1 < 0 )
        Lab_out[1] = -Lab_in[1];

      // Now we convert back to RGB and we compute the multiplicative
      // factors that bring from the current WB to the target one
      cmsDoTransform( transform_inv, Lab_out, rgb_out, 1 );
      wb_red_mul = rgb_out[0]/rgb_avg[0];
      wb_green_mul = rgb_out[1]/rgb_avg[1];
      wb_blue_mul = rgb_out[2]/rgb_avg[2];

    } else {

      // Both "a" and "b" target channels are different from zero, so we try to 
      // preserve the target a/b ratio
      float sign1 = (delta1*wb_delta1 < 0) ? -1 : 1;
      float sign2 = (delta2*wb_delta2 < 0) ? -1 : 1;
      float ab_ratio = (sign1*delta1)/(sign2*delta2);
      float wb_ab_ratio = wb_delta1/wb_delta2;

      Lab_out[0] = Lab_in[0];
      if( fabs(wb_delta1) > fabs(wb_delta2) ) {
        Lab_out[1] = sign1*delta1 + ab_zero;
        Lab_out[2] = sign2*delta2*ab_ratio/wb_ab_ratio + ab_zero;
      } else {
        Lab_out[1] = sign1*delta1*wb_ab_ratio/ab_ratio + ab_zero;
        Lab_out[2] = sign2*delta2 + ab_zero;
      }
      Lab_out[1] = Lab_wb[1];
      Lab_out[2] = Lab_wb[2];
      std::cout<<" Lab out: "<<Lab_out[0]<<" "<<Lab_out[1]<<" "<<Lab_out[2]<<std::endl;

      float delta_a = Lab_out[1] - Lab_in[1];
      float delta_b = Lab_out[2] - Lab_in[2];
      float wb_red_out1 = wb_red_in;
      float wb_red_out2 = wb_red_in;
      float wb_green_out1 = wb_green_in;
      float wb_green_out2 = wb_green_in;
      float wb_blue_out1 = wb_blue_in;
      float wb_blue_out2 = wb_blue_in;

      if( Lab_out[1] >= 0 ) {
        // Target "a" is positive, therefore we have to act 
        // on the red and blue multipliers simultaneously
        wb_red_out1 += wb_red_in * (delta_a*0.1/Lab_out[1]);
        wb_blue_out1 += wb_blue_in * (delta_a*0.1/Lab_out[1]);
      } else {
        // Target "a" is negative, therefore we have to act 
        // on the green channel only
        wb_green_out1 += wb_green_in * (delta_a*0.1/Lab_out[1]);
      }

      if( Lab_out[2] >= 0 ) {
        // Target "b" is positive, therefore we have to act 
        // on the red and green multipliers simultaneously
        wb_red_out2 += wb_red_in * (delta_b*0.1/Lab_out[2]);
        wb_green_out2 += wb_green_in * (delta_b*0.1/Lab_out[2]);
      } else {
        // Target "b" is negative, therefore we have to act 
        // on the blue channel only
        wb_blue_out2 += wb_blue_in * (delta_b*0.1/Lab_out[2]);
      }

      wb_red_out = (wb_red_out1 + wb_red_out2)/2.0f;
      wb_green_out = (wb_green_out1 + wb_green_out2)/2.0f;
      wb_blue_out = (wb_blue_out1 + wb_blue_out2)/2.0f;

      /*
        // Now we convert back to RGB and we compute the multiplicative
        // factors that bring from the current WB to the target one
        cmsDoTransform( transform_inv, Lab_out, rgb_out, 1 );
        std::cout<<" RGB out: "<<rgb_out[0]*255<<" "<<rgb_out[1]*255<<" "<<rgb_out[2]*255<<std::endl;

        wb_red_mul = rgb_out[0]/rgb_avg[0];
        wb_green_mul = rgb_out[1]/rgb_avg[1];
        wb_blue_mul = rgb_out[2]/rgb_avg[2];

        float f = 1.5;
        wb_red_out = (f*wb_red_mul+1-f)*wb_red_in;
        wb_green_out = (f*wb_green_mul+1-f)*wb_green_in;
        wb_blue_out = (f*wb_blue_mul+1-f)*wb_blue_in;
        float scale = (wb_red_out+wb_green_out+wb_blue_out)/3.0f;
        std::cout<<" scale: "<<scale<<std::endl;
      //float norm_out = MIN3(wb_red_out,wb_green_out,wb_blue_out);
       */
      /*
      // Scale target L channel according to norm_out
      Lab_out[0] /= norm_out*1.01;
      std::cout<<" Lab out #2: "<<Lab_out[0]<<" "<<Lab_out[1]<<" "<<Lab_out[2]<<std::endl;

      // Repeat the transform with the new luminosity
      cmsDoTransform( transform_inv, Lab_out, rgb_out, 1 );
      std::cout<<" RGB out #2: "<<rgb_out[0]*255<<" "<<rgb_out[1]*255<<" "<<rgb_out[2]*255<<std::endl;

      wb_red_mul = rgb_out[0]/rgb_avg[0];
      wb_green_mul = rgb_out[1]/rgb_avg[1];
      wb_blue_mul = rgb_out[2]/rgb_avg[2];

      wb_red_out = wb_red_mul*wb_red_in;
      wb_green_out = wb_green_mul*wb_green_in;
      wb_blue_out = wb_blue_mul*wb_blue_in;
       */
    }

    /*
    float wb_min = MIN3( wb_red_mul, wb_green_mul, wb_blue_mul );
    wb_red_mul /= wb_min;
    wb_green_mul /= wb_min;
    wb_blue_mul /= wb_min;

    float wb_red_d = wb_red_mul - wb_red_mul_prev;
    float wb_green_d = wb_green_mul - wb_green_mul_prev;
    float wb_blue_d = wb_blue_mul - wb_blue_mul_prev;

    wb_red_mul = wb_red_mul_prev + 1.00001*wb_red_d;
    wb_green_mul = wb_green_mul_prev + 1.00001*wb_green_d;
    wb_blue_mul = wb_blue_mul_prev + 1.00001*wb_blue_d;

    wb_red_mul_prev = wb_red_mul;
    wb_green_mul_prev = wb_green_mul;
    wb_blue_mul_prev = wb_blue_mul;
     */
    /*
    // The WB multiplicative factors are scaled so that their product is equal to 1
    float scale = wb_red_mul*wb_green_mul*wb_blue_mul;
    //float scale = wb_green_mul;
    wb_red_mul /= scale;
    wb_green_mul /= scale;
    wb_blue_mul /= scale;
     */

    //float wb_red_out = wb_red_mul*wb_red_in;
    // float wb_green_out = wb_green_mul*wb_green_in;
    //float wb_blue_out = wb_blue_mul*wb_blue_in;
    //float scale = (wb_red_out+wb_green_out+wb_blue_out)/3.0f;
    //float scale = MIN3(wb_red_out,wb_green_out,wb_blue_out);
    float scale = MIN3(wb_red_out,wb_green_out,wb_blue_out);
    //scale = 1;
    std::cout<<" WB coefficients (1): "<<wb_red_in<<"*"<<wb_red_mul<<" -> "<<wb_red_out<<std::endl
        <<"                      "<<wb_green_in<<"*"<<wb_green_mul<<" -> "<<wb_green_out<<std::endl
        <<"                      "<<wb_blue_in<<"*"<<wb_blue_mul<<" -> "<<wb_blue_out<<std::endl;
    std::cout<<"  scale: "<<scale<<std::endl;
    //float scale = wb_green_mul;
    wb_red_out /= scale;
    wb_green_out /= scale;
    wb_blue_out /= scale;
    wb_red_prop->update( wb_red_out );
    wb_green_prop->update( wb_green_out );
    wb_blue_prop->update( wb_blue_out );

    std::cout<<" WB coefficients (2): "<<wb_red_in<<"*"<<wb_red_mul<<" -> "<<wb_red_out<<std::endl
        <<"                      "<<wb_green_in<<"*"<<wb_green_mul<<" -> "<<wb_green_out<<std::endl
        <<"                      "<<wb_blue_in<<"*"<<wb_blue_mul<<" -> "<<wb_blue_out<<std::endl;

    wbRedSliders[wb_id]->init();
    wbGreenSliders[wb_id]->init();
    wbBlueSliders[wb_id]->init();

    //bool async = img->is_async();
    //img->set_async( false );
    par->set_modified();
    img->update( pipeline, true );
    //img->update( NULL, true );
    //img->unlock();
    //img->set_async( async );


    /*
    g_object_unref( spot );
    g_object_unref( outimg );
    g_object_unref( region );

    if( vips_crop( image, &spot, 
                   clipped.left, clipped.top,
                   clipped.width, clipped.height,
                   NULL ) )
      return;

    outimg = im_open( "spot_wb_img", "p" );
    if (vips_sink_screen (spot, outimg, NULL,
                          64, 64, 1,
                          0, NULL, this))
      return;
    region = vips_region_new( outimg );
    if (vips_region_prepare (region, &rspot))
      return;

    std::cout<<"RawDeveloperConfigGUI: checking spot WB"<<std::endl;
    rgb_avg[0] = rgb_avg[1] = rgb_avg[2] = 0;
    for( row = 0; row < rspot.height; row++ ) {
      p = (float*)VIPS_REGION_ADDR( region, rspot.left, rspot.top );
      for( col = 0; col < line_size; col += 3 ) {
        red = p[col];      rgb_avg[0] += red;
        green = p[col+1];  rgb_avg[1] += green;
        blue = p[col+2];   rgb_avg[2] += blue;
        //std::cout<<"  pixel="<<row<<","<<col<<"    red="<<red<<"  green="<<green<<"  blue="<<blue<<std::endl;
      }
    }
    rgb_avg[0] /= rspot.width*rspot.height;
    rgb_avg[1] /= rspot.width*rspot.height;
    rgb_avg[2] /= rspot.width*rspot.height;
     */
    std::cout<<"RawDeveloperConfigGUI: checking spot WB"<<std::endl;
    img->sample( l->get_id(), x, y, sample_size, NULL, values );
    if( values.size() != 3 ) {
      std::cout<<"RawDeveloperConfigGUI::pointer_relese_event(): values.size() "
          <<values.size()<<" (!= 3)"<<std::endl;
      return;
    }
    rgb_avg[0] = values[0];
    rgb_avg[1] = values[1];
    rgb_avg[2] = values[2];

    std::cout<<" RGB check: "<<rgb_avg[0]*255<<" "<<rgb_avg[1]*255<<" "<<rgb_avg[2]*255<<std::endl;
    // Now we convert the average RGB values in the WB spot region to Lab
    cmsDoTransform( transform, rgb_avg, Lab_check, 1 );
    std::cout<<" Lab check("<<i<<"): "<<Lab_check[0]<<" "<<Lab_check[1]<<" "<<Lab_check[2]<<std::endl;

    /*
    g_object_unref( spot );
    g_object_unref( outimg );
    g_object_unref( region );
     */

    if( i == 0 ) continue;
    float delta_a = Lab_check[1] - Lab_prev[1];
    float delta_b = Lab_check[2] - Lab_prev[2];
    if( (fabs(delta_a) < 0.005) && (fabs(delta_b) < 0.005) )
      break;
    Lab_prev[0] = Lab_check[0];
    Lab_prev[1] = Lab_check[1];
    Lab_prev[2] = Lab_check[2];
  }

  PF::raw_preproc_sample_x = 0;
  PF::raw_preproc_sample_y = 0;

  char tstr[500];
  snprintf( tstr, 499, "Best match: L=%0.2f a=%0.2f b=%0.2f",
      Lab_check[0], Lab_check[1], Lab_check[2] );
  wb_best_match_label.set_text( tstr );

  cmsDeleteTransform( transform );
  cmsDeleteTransform( transform_inv );
  cmsCloseProfile( profile_in );
  cmsCloseProfile( profile_out );

  par->set_caching( true );
  l->set_dirty(true);
  par->set_modified();
  // Update the preview to reflect the new settings
  img->update();
}



void PF::RawDeveloperConfigGUI::on_cam_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
      Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_icc_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK):
            {
    std::cout << "Open clicked." << std::endl;

    last_dir = dialog.get_current_folder();
    PF::PhotoFlow::Instance().get_options().set_last_visited_icc_folder( last_dir );

    //Notice that this is a std::string, not a Glib::ustring.
    std::string filename = dialog.get_filename();
    std::cout << "File selected: " <<  filename << std::endl;
    camProfFileEntry.set_text( filename.c_str() );
    on_cam_filename_changed();
    break;
            }
  case(Gtk::RESPONSE_CANCEL):
            {
    std::cout << "Cancel clicked." << std::endl;
    break;
            }
  default:
  {
    std::cout << "Unexpected button clicked." << std::endl;
    break;
  }
  }
}



void PF::RawDeveloperConfigGUI::on_cam_dcp_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a DCP profile",
      Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_icc_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK):
            {
    std::cout << "Open clicked." << std::endl;

    last_dir = dialog.get_current_folder();
    PF::PhotoFlow::Instance().get_options().set_last_visited_icc_folder( last_dir );

    //Notice that this is a std::string, not a Glib::ustring.
    std::string filename = dialog.get_filename();
    std::cout << "File selected: " <<  filename << std::endl;
    camDCPProfFileEntry.set_text( filename.c_str() );
    on_cam_dcp_filename_changed();
    break;
            }
  case(Gtk::RESPONSE_CANCEL):
            {
    std::cout << "Cancel clicked." << std::endl;
    break;
            }
  default:
  {
    std::cout << "Unexpected button clicked." << std::endl;
    break;
  }
  }
}



void PF::RawDeveloperConfigGUI::on_out_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
      Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  Glib::ustring last_dir = PF::PhotoFlow::Instance().get_options().get_last_visited_icc_folder();
  if( !last_dir.empty() ) dialog.set_current_folder( last_dir );

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK):
            {
    std::cout << "Open clicked." << std::endl;

    last_dir = dialog.get_current_folder();
    PF::PhotoFlow::Instance().get_options().set_last_visited_icc_folder( last_dir );

    //Notice that this is a std::string, not a Glib::ustring.
    std::string filename = dialog.get_filename();
    std::cout << "File selected: " <<  filename << std::endl;
    outProfFileEntry.set_text( filename.c_str() );
    on_out_filename_changed();
    break;
            }
  case(Gtk::RESPONSE_CANCEL):
            {
    std::cout << "Cancel clicked." << std::endl;
    break;
            }
  default:
  {
    std::cout << "Unexpected button clicked." << std::endl;
    break;
  }
  }
}



void PF::RawDeveloperConfigGUI::on_cam_filename_changed()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = camProfFileEntry.get_text();
    if( filename.empty() )
      return;
    //std::cout<<"New input profile name: "<<filename<<std::endl;
    PF::RawDeveloperPar* par =
        dynamic_cast<PF::RawDeveloperPar*>(get_layer()->get_processor()->get_par());
    if( !par )
      return;
    PropertyBase* prop = par->get_property( "cam_profile_name" );
    if( !prop )
      return;
    prop->update( filename );
    get_layer()->set_dirty( true );
    //std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}



void PF::RawDeveloperConfigGUI::on_cam_dcp_filename_changed()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = camDCPProfFileEntry.get_text();
    if( filename.empty() )
      return;
    std::cout<<"New DCP profile name: "<<filename<<std::endl;
    PF::RawDeveloperPar* par =
        dynamic_cast<PF::RawDeveloperPar*>(get_layer()->get_processor()->get_par());
    if( !par )
      return;
    PropertyBase* prop = par->get_property( "cam_dcp_profile_name" );
    if( !prop )
      return;
    prop->update( filename );
    get_layer()->set_dirty( true );
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}



void PF::RawDeveloperConfigGUI::on_out_filename_changed()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = outProfFileEntry.get_text();
    if( filename.empty() )
      return;
    //std::cout<<"New output profile name: "<<filename<<std::endl;
    PF::RawDeveloperPar* par =
        dynamic_cast<PF::RawDeveloperPar*>(get_layer()->get_processor()->get_par());
    if( !par )
      return;
    PropertyBase* prop = par->get_property( "out_profile_name" );
    if( !prop )
      return;
    prop->update( filename );
    get_layer()->set_dirty( true );
    //std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}


void PF::RawDeveloperConfigGUI::on_histogram_radio_group_changed()
{
  histogram_range_full = hist_range_full_check.get_active();
  histogram_range_sh = hist_range_sh_check.get_active();
  histogram_range_hi = hist_range_hi_check.get_active();
  if( histogram_range_full ) {
    histogramArea.zoom_black = false;
    histogramArea.zoom_white = false;
  }
  if( histogram_range_sh ) {
    histogramArea.zoom_black = true;
    histogramArea.zoom_white = false;
  }
  if( histogram_range_hi ) {
    histogramArea.zoom_black = false;
    histogramArea.zoom_white = true;
  }
  histogramArea.queue_draw();
}


bool PF::RawDeveloperConfigGUI::is_area_wb()
{
  if( wbModeSelector.get_prop() &&
      wbModeSelector.get_prop()->is_enum() &&
      (wbModeSelector.get_prop()->get_enum_value().first == (int)PF::WB_AREA_SPOT) )
    return true;
  return false;
}



void PF::RawDeveloperConfigGUI::find_handle_point(int x, int y)
{
  double wb_point_D = 1000000000;
  PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>(get_par());
  if( !par ) return;
  std::vector< std::vector<int> >& wb_areas = par->get_wb_areas();
  selected_wb_area_point = -1;

  for(unsigned int ai = 0; ai < wb_areas.size(); ai++) {
    std::vector<int>& area = wb_areas[ai];
    if( area.size() != 4 ) continue;

    double px1 = area[0], py1 = area[1];
    double px2 = area[2], py2 = area[3];

    double D[4];
    D[0] = (x-px1)*(x-px1) + (y-py1)*(y-py1);
    D[1] = (x-px2)*(x-px2) + (y-py1)*(y-py1);
    D[2] = (x-px1)*(x-px1) + (y-py2)*(y-py2);
    D[3] = (x-px2)*(x-px2) + (y-py2)*(y-py2);

    for(int pi = 0; pi < 4; pi++) {
      if(D[pi] >= wb_point_D) continue;
      wb_point_D = D[pi];
      double dx = wb_point_D, dy = wb_point_D;
      double w = 1; double h = 1;
      layer2screen(dx, dy, w, h);
      if(dx < 25) selected_wb_area_point = ai*10 + pi;
    }
  }
}



void PF::RawDeveloperConfigGUI::find_area(int x, int y)
{
  double wb_point_D = 1000000000;
  PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>(get_par());
  if( !par ) return;
  std::vector< std::vector<int> >& wb_areas = par->get_wb_areas();
  selected_wb_area_id = -1;

  for(unsigned int ai = 0; ai < wb_areas.size(); ai++) {
    std::vector<int>& area = wb_areas[ai];
    if( area.size() != 4 ) continue;

    double px1 = area[0], py1 = area[1];
    double px2 = area[2], py2 = area[3];
    std::cout<<"x="<<x<<" y="<<y<<" px1="<<px1<<" py1="<<py1<<" px2="<<px2<<" py2="<<py2<<std::endl;
    if( x<px1 || y<py1 || x>px2 || y>py2) continue;

    selected_wb_area_id = ai;
    wb_area_dx = (int)(x - px1);
    wb_area_dy = (int)(y - py1);

    break;
  }
}




bool PF::RawDeveloperConfigGUI::pointer_press_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  //if( button != 1 ) return false;

  PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>(get_par());
  if( !par ) return false;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );

  selected_wb_area_id = -1;
  selected_wb_area_point = -1;
  wb_area_dx = 0;
  wb_area_dy = 0;

  if( is_area_wb() && button == 1 ) {
    // Find handle point
    find_handle_point(x, y);
    if( selected_wb_area_point >= 0 ) {
      std::cout<<"RawDeveloperConfigGUI: selected WB area point "<<selected_wb_area_point<<std::endl;
      tmp_area.clear();
      return true;
    }

    find_area(x, y);
    if( selected_wb_area_id >= 0 ) {
      std::cout<<"RawDeveloperConfigGUI: selected WB area id "<<selected_wb_area_point<<std::endl;
      tmp_area.clear();
      return true;
    }
  }

  if( is_area_wb() && button == 3 ) {
    find_area(x, y);
    if( selected_wb_area_id >= 0 ) {
      std::cout<<"RawDeveloperConfigGUI: selected WB area id "<<selected_wb_area_point<<std::endl;
      tmp_area.clear();
      return true;
    }
  }
  return false;
}


bool PF::RawDeveloperConfigGUI::pointer_release_event( int button, double sx, double sy, int mod_key )
{
  std::cout<<"release: button="<<button<<"  selected_wb_area_point: "<<selected_wb_area_point
      <<" selected_wb_area_id: "<<selected_wb_area_id<<std::endl;

  if( button == 1 ) {
    if( wbModeSelector.get_prop() &&
        wbModeSelector.get_prop()->is_enum() &&
        (wbModeSelector.get_prop()->get_enum_value().first == (int)PF::WB_SPOT) ) {
      double x = sx, y = sy, w = 1, h = 1;
      screen2layer( x, y, w, h );
      spot_wb( x, y );
    }

    if( wbModeSelector.get_prop() &&
        wbModeSelector.get_prop()->is_enum() &&
        (wbModeSelector.get_prop()->get_enum_value().first == (int)PF::WB_COLOR_SPOT) ) {
      double x = sx, y = sy, w = 1, h = 1;
      screen2layer( x, y, w, h );
      color_spot_wb( x, y );
    }

    if( wbModeSelector.get_prop() &&
        wbModeSelector.get_prop()->is_enum() &&
        (wbModeSelector.get_prop()->get_enum_value().first == (int)PF::WB_AREA_SPOT) ) {
      if( selected_wb_area_point >= 0 ) {
        spot_wb(0,0);
        return true;
      }
      if( selected_wb_area_id >= 0 ) {
        spot_wb(0,0);
        return true;
      }

      double x = sx, y = sy, w = 1, h = 1;
      screen2layer( x, y, w, h );
      if( tmp_area.size() == 4 ) {
        PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>(get_par());
        if( !par ) return false;
        if( par->get_wb_areas().empty() || mod_key == (PF::MOD_KEY_CTRL+PF::MOD_KEY_ALT) )
          par->add_wb_area(tmp_area);
        spot_wb(0,0);
        return true;
      } else {
        PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>(get_par());
        if( !par ) return false;
        if( par->get_wb_areas().empty() || mod_key != (PF::MOD_KEY_CTRL+PF::MOD_KEY_ALT) ) return false;
        // There are no placed areas yet, so we suggest a new placement
        x = sx-10; y=sy-10; w=1; h=1;
        screen2layer( x, y, w, h );
        tmp_area.push_back(x); tmp_area.push_back(y);
        x = sx+10; y=sy+10; w=1; h=1;
        screen2layer( x, y, w, h );
        tmp_area.push_back(x); tmp_area.push_back(y);
        par->add_wb_area(tmp_area);
        spot_wb(0,0);
        return true;
      }
    }
  } else if( button == 3 && is_area_wb() && selected_wb_area_id >= 0 ) {
    PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>(get_par());
    if( !par ) return false;
    std::vector< std::vector<int> >& wb_areas = par->get_wb_areas();
    std::vector< std::vector<int> > wb_areas_new;
    for(int ai = 0; ai < wb_areas.size(); ai++) {
      if( ai == selected_wb_area_id ) continue;
      wb_areas_new.push_back( wb_areas[ai] );
    }
    par->get_wb_areas() = wb_areas_new;
    selected_wb_area_id = -1;
    spot_wb(0,0);
    return true;
  }

  return false;
}


bool PF::RawDeveloperConfigGUI::pointer_motion_event( int button, double sx, double sy, int mod_key )
{
  if( !get_editing_flag() ) return false;

  PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>(get_par());
  if( !par ) return false;

  double x = sx, y = sy, w = 1, h = 1;
  screen2layer( x, y, w, h );

  int ix = x;
  int iy = y;

  if( is_area_wb() ) {
    // Find handle point
    std::vector< std::vector<int> >& wb_areas = par->get_wb_areas();
    tmp_area.clear();

    //std::cout<<"selected_wb_area_point: "<<selected_wb_area_point
    //    <<" selected_wb_area_id: "<<selected_wb_area_id<<std::endl;

    if( selected_wb_area_point >= 0 && button == 1 ) {
      int aid = selected_wb_area_point/10;
      if( aid < 0 || aid >= wb_areas.size() ) return false;
      int pid = selected_wb_area_point%10;
      if( pid < 0 || pid >= 4 ) return false;
      std::vector<int>& area = wb_areas[aid];
      int Dmin = 10;
      switch(pid) {
      case 0: // top-left point
        if((area[2]-x) > Dmin) area[0] = x;
        if((area[3]-y) > Dmin) area[1] = y;
        break;
      case 1: // top-right point
        if((x-area[0]) > Dmin) area[2] = x;
        if((area[3]-y) > Dmin) area[1] = y;
        break;
      case 2: // bottom-left point
        if((area[2]-x) > Dmin) area[0] = x;
        if((y-area[1]) > Dmin) area[3] = y;
        break;
      case 3: // bottom-right point
        if((x-area[0]) > Dmin) area[2] = x;
        if((y-area[1]) > Dmin) area[3] = y;
        break;
      default: return false;
      }
      return true;
    }

    if( selected_wb_area_id >= 0 && button == 1) {
      std::vector<int>& area = wb_areas[selected_wb_area_id];
      int dx = area[2] - area[0];
      int dy = area[3] - area[1];
      area[0] = x - wb_area_dx;
      area[1] = y - wb_area_dy;
      area[2] = area[0] + dx;
      area[3] = area[1] + dx;
      return true;
    }

    //std::cout<<"selected_wb_area_id: "<<selected_wb_area_id<<std::endl;
    if( button < 0 ) {
      if( par->get_wb_areas().empty() ) {
        // There are no placed areas yet, so we suggest a new placement
        x = sx-10; y=sy-10; w=1; h=1;
        screen2layer( x, y, w, h );
        tmp_area.push_back(x); tmp_area.push_back(y);
        x = sx+10; y=sy+10; w=1; h=1;
        screen2layer( x, y, w, h );
        tmp_area.push_back(x); tmp_area.push_back(y);
        return true;
      }

      // Find handle point
      find_handle_point(x, y);
      if( selected_wb_area_point >= 0 ) {
        tmp_area.clear();
        return true;
      }
    }
  }
  return false;
}


void PF::RawDeveloperConfigGUI::draw_point(int px, int py, int point_size, PF::PixelBuffer& buf_out)
{
  VipsRect point = { (int)px-point_size-1,
      (int)py-point_size-1,
      point_size*2+3, point_size*2+3};
  VipsRect point2 = { (int)px-point_size,
      (int)py-point_size,
      point_size*2+1, point_size*2+1};
  buf_out.fill( point, 0, 0, 0 );
  buf_out.fill( point2, 255, 0, 0 );
}


void PF::RawDeveloperConfigGUI::draw_area(std::vector<int>& area,
    PF::PixelBuffer& buf_in, PF::PixelBuffer& buf_out)
{
  if( area.size() != 4 ) return;

  int point_size = 2;

  double pw1 = 1, ph1 = 1;
  double px1 = area[0], py1 = area[1];
  layer2screen( px1, py1, pw1, ph1 );

  pw1 = 1, ph1 = 1;
  double px2 = area[2], py2 = area[3];
  layer2screen( px2, py2, pw1, ph1 );

  buf_out.draw_line( px1, py1, px2, py1, buf_in );
  buf_out.draw_line( px1, py1, px1, py2, buf_in );
  buf_out.draw_line( px1, py2, px2, py2, buf_in );
  buf_out.draw_line( px2, py1, px2, py2, buf_in );

  draw_point(px1, py1, point_size, buf_out);
  draw_point(px2, py1, point_size, buf_out);
  draw_point(px1, py2, point_size, buf_out);
  draw_point(px2, py2, point_size, buf_out);
}


bool PF::RawDeveloperConfigGUI::modify_preview( PF::PixelBuffer& buf_in, PF::PixelBuffer& buf_out,
    float scale, int xoffset, int yoffset )
{
  PF::RawDeveloperPar* par = dynamic_cast<PF::RawDeveloperPar*>(get_par());
  if( !par ) return false;

  // Resize the output buffer to match the input one
  buf_out.resize( buf_in.get_rect() );

  // Copy pixel data from input to output
  buf_out.copy( buf_in );

  if( is_area_wb() ) {
    std::vector< std::vector<int> >& wb_areas = par->get_wb_areas();

    for(unsigned int ai = 0; ai < wb_areas.size(); ai++) {
      std::vector<int>& area = wb_areas[ai];
      draw_area( area, buf_in, buf_out );
    }
    if( tmp_area.size() == 4) {
      draw_area( tmp_area, buf_in, buf_out );
    }
  }
  return true;
}
