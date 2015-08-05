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

//#include "../../operations/hue_saturation.hh"

#include "../../base/color.hh"
#include "hsl_mask_config.hh"


class HueEqualizerArea: public PF::CurveArea
{
public:
  void draw_background(const Cairo::RefPtr<Cairo::Context>& cr)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width() - get_border_size()*2;
    const int height = allocation.get_height() - get_border_size()*2;
    const int x0 = get_border_size();
    const int y0 = get_border_size();

    cr->set_source_rgb( 0.5, 0.5, 0.5 );
    for( int x = 0; x < width; x++ ) {
      float h = static_cast<float>(x)*360/width, s = 0.99, l = 0.5;
      //std::cout<<"Hue: "<<h<<std::endl;
      float R, G, B;
      PF::hsl2rgb( h, s, l, R, G, B );
      cr->set_source_rgb( R, G, B );
      cr->move_to( double(0.5+x0+x), double(y0+height-height/1) );
      cr->rel_line_to (double(0), double(height/1) );
      cr->stroke ();
    }

    // Draw grid
    cr->set_source_rgb( 0.9, 0.9, 0.9 );
    std::vector<double> ds (2);
    ds[0] = 4;
    ds[1] = 4;
    cr->set_dash (ds, 0);
    //cr->move_to( double(0.5+x0+width/4), double(y0) );
    //cr->rel_line_to (double(0), double(height) );
    //cr->move_to( double(0.5+x0+width/2), double(y0) );
    //cr->rel_line_to (double(0), double(height) );
    //cr->move_to( double(0.5+x0+width*3/4), double(y0) );
    //cr->rel_line_to (double(0), double(height) );
    cr->move_to( double(x0), double(0.5+y0+height/4) );
    cr->rel_line_to (double(width), double(0) );
    cr->move_to( double(x0), double(0.5+y0+height/2) );
    cr->rel_line_to (double(width), double(0) );
    cr->move_to( double(x0), double(0.5+y0+height*3/4) );
    cr->rel_line_to (double(width), double(0) );
    cr->stroke ();
    cr->unset_dash ();
  }
};


PF::HSLMaskConfigDialog::HSLMaskConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, _("HSL Mask") ),
  hueHeq( this, "H_curve", new HueEqualizerArea(), 0, 360, 0, 100, 400, 150 ),
  hueSeq( this, "S_curve", new PF::CurveArea(), 0, 100, 0, 100, 400, 150 ),
  hueLeq( this, "L_curve", new PF::CurveArea(), 0, 100, 0, 100, 400, 150 ),
  hueHeq_enable( this, "H_curve_enabled", "Enable", true ),
  hueSeq_enable( this, "S_curve_enabled", "Enable", true  ),
  hueLeq_enable( this, "L_curve_enabled", "Enable", true  ),
  layer_list( this, _("Layer name:") )
{
  curves_nb[0].append_page( hueHeq_box, "Hue curve" );
  curves_nb[0].append_page( hueSeq_box, "Saturation curve" );
  curves_nb[0].append_page( hueLeq_box, "Luminosity curve" );

  hueHeq_box.pack_start( hueHeq, Gtk::PACK_SHRINK );
  hueHeq_box.pack_start( hueHeq_enable_box, Gtk::PACK_SHRINK );
  hueSeq_box.pack_start( hueSeq, Gtk::PACK_SHRINK );
  hueSeq_box.pack_start( hueSeq_enable_box, Gtk::PACK_SHRINK );
  hueLeq_box.pack_start( hueLeq, Gtk::PACK_SHRINK );
  hueLeq_box.pack_start( hueLeq_enable_box, Gtk::PACK_SHRINK );

  hueHeq_enable_box.pack_end( hueHeq_enable, Gtk::PACK_SHRINK );
  hueSeq_enable_box.pack_end( hueSeq_enable, Gtk::PACK_SHRINK );
  hueLeq_enable_box.pack_end( hueLeq_enable, Gtk::PACK_SHRINK );
  //hueHeq_enable_box.pack_end( hueHeq_enable_padding, Gtk::PACK_EXPAND_WIDGET );

  controlsBox.pack_start( curves_nb[0], Gtk::PACK_SHRINK );

  controlsBox.pack_start( layer_list, Gtk::PACK_SHRINK );
  /*
  controlsBox.pack_start( adjustments_nb );

  adjustments_nb.append_page( adjustment_box[0], "Hue" );
  adjustments_nb.append_page( adjustment_box[1], "Saturation" );
  adjustments_nb.append_page( adjustment_box[2], "Contrast" );

  // Hue
  adjustment_box[0].pack_start( hueSlider, Gtk::PACK_SHRINK );
  adjustment_box[0].pack_start( hue2Slider, Gtk::PACK_SHRINK );
  adjustment_box[0].pack_start( curves_nb[0], Gtk::PACK_SHRINK );

  curves_nb[0].append_page( hueHeq, "Hue curve" );
  curves_nb[0].append_page( hueSeq, "Saturation curve" );
  curves_nb[0].append_page( hueLeq, "Luminosity curve" );

  // Saturation
  adjustment_box[1].pack_start( saturationSlider, Gtk::PACK_SHRINK );
  adjustment_box[1].pack_start( saturation2Slider, Gtk::PACK_SHRINK );
  adjustment_box[1].pack_start( curves_nb[1], Gtk::PACK_SHRINK );

  curves_nb[1].append_page( saturationHeq, "Hue curve" );
  curves_nb[1].append_page( saturationSeq, "Saturation curve" );
  curves_nb[1].append_page( saturationLeq, "Luminosity curve" );

  // Contrast
  adjustment_box[2].pack_start( contrastSlider, Gtk::PACK_SHRINK );
  adjustment_box[2].pack_start( contrast2Slider, Gtk::PACK_SHRINK );
  adjustment_box[2].pack_start( curves_nb[2], Gtk::PACK_SHRINK );

  curves_nb[2].append_page( contrastHeq, "Hue curve" );
  curves_nb[2].append_page( contrastSeq, "Saturation curve" );
  curves_nb[2].append_page( contrastLeq, "Luminosity curve" );
*/
  /*
  controlsBox.pack_start( expanders[0][0] );
  controlsBox.pack_start( expanders[1][0] );
  controlsBox.pack_start( expanders[2][0] );

  expanders[0][0].set_label( "hue adjustment" );
  expanders[1][0].set_label( "saturation adjustment" );
  expanders[2][0].set_label( "contrast adjustment" );

  //for( int i = 0; i < 3; i++ )
  //  for( int j = 0; j < 4; j++ )
  //    expanders[i][j].set_resize_toplevel( true );

  for( int i = 0; i < 3; i++ )
    for( int j = 0; j < 4; j++ )
      expander_paddings[i][j].set_size_request(20,-1);

  expanders[0][0].add( expander_hboxes[0][0] );
  expander_hboxes[0][0].pack_start( expander_paddings[0][0], Gtk::PACK_SHRINK );
  expander_hboxes[0][0].pack_start( expander_vboxes[0], Gtk::PACK_SHRINK, 0 );
  expander_vboxes[0].pack_start( hueSlider, Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( hue2Slider, Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( expanders[0][1], Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( expanders[0][2], Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( expanders[0][3], Gtk::PACK_SHRINK );
  expanders[0][1].set_label( "hue curve" );
  expanders[0][1].add( expander_hboxes[0][1] );
  expander_hboxes[0][1].pack_start( expander_paddings[0][1], Gtk::PACK_SHRINK );
  expander_hboxes[0][1].pack_start( hueHeq, Gtk::PACK_SHRINK, 0 );
  expanders[0][2].set_label( "saturation curve" );
  expanders[0][2].add( hueSeq );
  expanders[0][3].set_label( "luminance curve" );
  expanders[0][3].add( hueLeq );
  */
  
  add_widget( controlsBox );
}


void PF::HSLMaskConfigDialog::do_update()
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    if( get_layer()->get_processor()->get_par()->is_map() ) {
      layer_list.update_model();
      if( layer_list.get_parent() != &controlsBox ) {
        controlsBox.pack_start( layer_list, Gtk::PACK_SHRINK );
        layer_list.show();
      }
    } else {
      if( layer_list.get_parent() == &controlsBox ) {
        controlsBox.remove( layer_list );
      }
    }
  }
  OperationConfigDialog::do_update();
}


void PF::HSLMaskConfigDialog::init()
{
  layer_list.update_model();
  OperationConfigDialog::init();
}


bool PF::HSLMaskConfigDialog::pointer_press_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  return false;
}


bool PF::HSLMaskConfigDialog::pointer_release_event( int button, double x, double y, int mod_key )
{
  if( button != 1 || mod_key != PF::MOD_KEY_CTRL ) return false;
  std::cout<<"HSLMaskConfigDialog::pointer_release_event(): x="<<x<<"  y="<<y<<"    mod_key="<<mod_key<<std::endl;

  // Retrieve the layer associated to the filter
  PF::Layer* layer = get_layer();
  if( !layer ) return false;

  // Retrieve the image the layer belongs to
  PF::Image* image = layer->get_image();
  if( !image ) return false;

  // Retrieve the pipeline #0 (full resolution preview)
  PF::Pipeline* pipeline = image->get_pipeline( 0 );
  if( !pipeline ) return false;

  // Find the pipeline node associated to the current layer
  PF::PipelineNode* node = pipeline->get_node( layer->get_id() );
  if( !node ) return false;

  // Find the input layer of the current filter
  if( node->input_id < 0 ) return false;
  PF::Layer* lin = image->get_layer_manager().get_layer( node->input_id );
  if( !lin ) return false;

  // Sample a 5x5 pixels region of the input layer
  std::vector<float> values;
  float H, S, L;
  double lx = x, ly = y, lw = 1, lh = 1;
  screen2layer( lx, ly, lw, lh );
  std::cout<<"image->sample( lin->get_id(), "<<lx<<", "<<ly<<", 5, NULL, values );"<<std::endl;
  image->sample( lin->get_id(), lx, ly, 5, NULL, values );

  std::cout<<"HSLMaskConfigDialog::pointer_release_event(): values="<<values[0]<<","<<values[1]<<","<<values[2]<<std::endl;

  rgb2hsl( values[0], values[1], values[2], H, S, L );

  PF::OpParBase* par = get_layer()->get_processor()->get_par();
  PF::colorspace_t cs = PF::convert_colorspace( par->get_interpretation() );
  switch( cs ) {
  case PF_COLORSPACE_GRAYSCALE:
    break;
  case PF_COLORSPACE_RGB:
    if( values.size() != 3 ) return false;
    switch( curves_nb[0].get_current_page() ) {
    case 0:
      hueHeq.add_point( H/360.0f );
      break;
    case 1:
      hueSeq.add_point( S );
      break;
    case 2:
      hueLeq.add_point( L );
      break;
    }
    break;
    case PF_COLORSPACE_LAB:
      break;
  case PF_COLORSPACE_CMYK:
    break;
  default:
    break;
  }

  return false;
}


bool PF::HSLMaskConfigDialog::pointer_motion_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  return false;
}
