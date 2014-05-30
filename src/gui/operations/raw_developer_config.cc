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

#include "../../operations/raw_developer.hh"

#include "raw_developer_config.hh"


PF::RawDeveloperConfigDialog::RawDeveloperConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Raw Developer" ),
  wbModeSelector( this, "wb_mode", "WB mode: ", 0 ),
  wbRedSlider( this, "wb_red", "Red WB mult.", 1, 0, 10, 0.05, 0.1, 1),
  wbGreenSlider( this, "wb_green", "Green WB mult.", 1, 0, 10, 0.05, 0.1, 1),
  wbBlueSlider( this, "wb_blue", "Blue WB mult.", 1, 0, 10, 0.05, 0.1, 1),
  exposureSlider( this, "exposure", "Exp. compensation", 0, -5, 5, 0.05, 0.5 ),
  profileModeSelector( this, "profile_mode", "Color conversion mode: ", 0 ),
  camProfOpenButton(Gtk::Stock::OPEN),
  gammaModeSelector( this, "gamma_mode", "Raw gamma: ", 0 ),
  inGammaLinSlider( this, "gamma_lin", "Gamma linear", 0, 0, 100000, 0.05, 0.1, 1),
  inGammaExpSlider( this, "gamma_exp", "Gamma exponent", 2.2, 0, 100000, 0.05, 0.1, 1),
  outProfileModeSelector( this, "out_profile_mode", "Output profile: ", 1 ),
  outProfOpenButton(Gtk::Stock::OPEN)
{
  wbControlsBox.pack_start( wbModeSelector );
  wbControlsBox.pack_start( wbRedSlider );
  //controlsBox.pack_start( wbGreenSlider );
  wbControlsBox.pack_start( wbBlueSlider );

  exposureControlsBox.pack_start( exposureSlider );

  profileModeSelectorBox.pack_start( profileModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( profileModeSelectorBox, Gtk::PACK_SHRINK );

  camProfLabel.set_text( "camera profile name:" );
  camProfVBox.pack_start( camProfLabel );
  camProfVBox.pack_start( camProfFileEntry );
  camProfHBox.pack_start( camProfVBox );
  camProfHBox.pack_start( camProfOpenButton, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( camProfHBox );

  gammaModeVBox.pack_start( gammaModeSelector );
  //gammaModeVBox.pack_start( inGammaLinSlider );
  gammaModeVBox.pack_start( inGammaExpSlider );
  gammaModeHBox.pack_start( gammaModeVBox, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( gammaModeHBox );

  outProfileModeSelectorBox.pack_start( outProfileModeSelector, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outProfileModeSelectorBox, Gtk::PACK_SHRINK );

  outProfLabel.set_text( "output profile name:" );
  outProfVBox.pack_start( outProfLabel );
  outProfVBox.pack_start( outProfFileEntry );
  outProfHBox.pack_start( outProfVBox );
  outProfHBox.pack_start( outProfOpenButton, Gtk::PACK_SHRINK );
  outputControlsBox.pack_start( outProfHBox );


  notebook.append_page( wbControlsBox, "White balance" );
  notebook.append_page( exposureControlsBox, "Exposure" );
  notebook.append_page( outputControlsBox, "Output" );
    
  add_widget( notebook );


  camProfFileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &RawDeveloperConfigDialog::on_cam_filename_changed));
  camProfOpenButton.signal_clicked().connect(sigc::mem_fun(*this,
							   &RawDeveloperConfigDialog::on_cam_button_open_clicked) );

  outProfFileEntry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &RawDeveloperConfigDialog::on_out_filename_changed));
  outProfOpenButton.signal_clicked().connect(sigc::mem_fun(*this,
							   &RawDeveloperConfigDialog::on_out_button_open_clicked) );

  show_all_children();
}




void PF::RawDeveloperConfigDialog::update()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::RawDeveloperPar* par = 
      dynamic_cast<PF::RawDeveloperPar*>(get_layer()->get_processor()->get_par());
    if( !par ) return;
    PropertyBase* prop = par->get_property( "cam_profile_name" );
    if( !prop )  return;
    std::string filename = prop->get_str();
    camProfFileEntry.set_text( filename.c_str() );

    prop = par->get_property( "out_profile_name" );
    if( !prop )  return;
    filename = prop->get_str();
    outProfFileEntry.set_text( filename.c_str() );

    prop = par->get_property( "profile_mode" );
    if( !prop )  return;

    if( camProfHBox.get_parent() == &outputControlsBox )
      outputControlsBox.remove( camProfHBox );

    if( outProfHBox.get_parent() == &outputControlsBox )
      outputControlsBox.remove( outProfHBox );

    if( gammaModeHBox.get_parent() == &outputControlsBox )
      outputControlsBox.remove( gammaModeHBox );

    switch( prop->get_enum_value().first ) {
    case PF::IN_PROF_NONE:
      outputControlsBox.pack_start( gammaModeHBox, Gtk::PACK_SHRINK );
      break;
    case PF::IN_PROF_MATRIX:
      outputControlsBox.pack_start( outProfHBox, Gtk::PACK_SHRINK );
      break;
    case PF::IN_PROF_ICC:
      outputControlsBox.pack_start( gammaModeHBox, Gtk::PACK_SHRINK );
      outputControlsBox.pack_start( camProfHBox, Gtk::PACK_SHRINK );
      outputControlsBox.pack_start( outProfHBox, Gtk::PACK_SHRINK );
      break;
    }
  }
  OperationConfigDialog::update();
}



void PF::RawDeveloperConfigDialog::pointer_press_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return;
}


void PF::RawDeveloperConfigDialog::pointer_release_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return;

  if( !(wbModeSelector.get_prop()) ||
      !(wbModeSelector.get_prop()->is_enum()) ||
      (wbModeSelector.get_prop()->get_enum_value().first != (int)PF::WB_SPOT) )
    return;

  // Get the layer associated to this operation
  PF::Layer* l = get_layer();
  if( !l ) return;

  // Get the image the layer belongs to
  PF::Image* img = l->get_image();
  if( !img ) return;
  
  // Get the default view of the image 
  // (it is supposed to be at 1:1 zoom level 
  // and floating point accuracy)
  PF::View* view = img->get_view( 0 );
  if( !view ) return;

  // Get the node associated to the layer
  PF::ViewNode* node = view->get_node( l->get_id() );
  if( !node ) return;

  // Finally, get the underlying VIPS image associated to the layer
  VipsImage* image = node->image;
  if( !image ) return;

  // We need to retrieve the input ICC profile for the Lab conversion later on
  void *data;
  size_t data_length;
  if( vips_image_get_blob( image, VIPS_META_ICC_NAME, 
			   &data, &data_length ) )
    return;

  cmsHPROFILE profile_in = cmsOpenProfileFromMem( data, data_length );
  if( !profile_in ) 
    return;
  
#ifndef NDEBUG
  char tstr[1024];
  cmsGetProfileInfoASCII(profile_in, cmsInfoDescription, "en", "US", tstr, 1024);
  std::cout<<"raw_developer: embedded profile found: "<<tstr<<std::endl;
#endif

  cmsCIExyY white;
  cmsWhitePointFromTemp( &white, 6500 );
  cmsHPROFILE profile_out = cmsCreateLab4Profile( &white );

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

  
  // Now we have to process a small portion of the image 
  // to get the corresponding Lab values
  VipsImage* spot;
  int left = (int)x-3;
  int top = (int)y-3;
  int width = 7;
  int height = 7;

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
  
  //if( vips_sink_memory( spot ) )
  //  return;

  int row, col;
  int line_size = clipped.width*3;
  float* p;
  float red, green, blue;
  float rgb_avg[3] = {0, 0, 0};
  float rgb_out[3] = {0, 0, 0};
  float Lab_in[3] = {0, 0, 0};
  float Lab_out[3] = {0, 0, 0};
  float Lab_wb[3] = {50, 0, 0};
  //float Lab_wb[3] = {70, 15, 10};
  std::cout<<"RawDeveloperConfigDialog: getting spot WB"<<std::endl;
  for( row = 0; row < rspot.height; row++ ) {
    p = (float*)VIPS_REGION_ADDR( region, rspot.left, rspot.top );
    for( col = 0; col < line_size; col += 3 ) {
      red = p[col];      rgb_avg[0] += red;
      green = p[col+1];  rgb_avg[1] += green;
      blue = p[col+2];   rgb_avg[2] += blue;
      std::cout<<"  pixel="<<row<<","<<col<<"    red="<<red<<"  green="<<green<<"  blue="<<blue<<std::endl;
    }
  }
  rgb_avg[0] /= rspot.width*rspot.height;
  rgb_avg[1] /= rspot.width*rspot.height;
  rgb_avg[2] /= rspot.width*rspot.height;

  // Now we convert the average RGB values in the WB spot region to Lab
  cmsDoTransform( transform, rgb_avg, Lab_in, 1 );

  const float epsilon = 1.0e-5;
  float ab_zero = 0;
  //float ab_zero = 0.5;
  float delta1 = Lab_in[1] - ab_zero;
  float delta2 = Lab_in[2] - ab_zero;

  float wb_delta1 = Lab_wb[1] - ab_zero;
  float wb_delta2 = Lab_wb[2] - ab_zero;

  float wb_red_mul;
  float wb_green_mul;
  float wb_blue_mul;

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
    
    // Now we convert back to RGB and we compute the multiplicative
    // factors that bring from the current WB to the target one
    cmsDoTransform( transform_inv, Lab_out, rgb_out, 1 );
    wb_red_mul = rgb_out[0]/rgb_avg[0];
    wb_green_mul = rgb_out[1]/rgb_avg[1];
    wb_blue_mul = rgb_out[2]/rgb_avg[2];
    
  }

  // The WB multiplicative factors are scaled so that their product is equal to 1
  //float scale = wb_red_mul*wb_green_mul*wb_blue_mul;
  float scale = wb_green_mul;
  wb_red_mul /= scale;
  wb_green_mul /= scale;
  wb_blue_mul /= scale;

  PropertyBase* wb_red_prop = wbRedSlider.get_prop();
  PropertyBase* wb_green_prop = wbGreenSlider.get_prop();
  PropertyBase* wb_blue_prop = wbBlueSlider.get_prop();
  if( wb_red_prop && wb_green_prop && wb_blue_prop ) {
    float wb_red_in;
    wb_red_prop->get( wb_red_in );
    wb_red_prop->update( wb_red_mul*wb_red_in );
    //float wb_green_in;
    //wb_green_prop->get( wb_green_in );
    //wb_green_prop->update( wb_green_mul*wb_green_in );
    float wb_blue_in;
    wb_blue_prop->get( wb_blue_in );
    wb_blue_prop->update( wb_blue_mul*wb_blue_in );

    wbRedSlider.init();
    //wbGreenSlider.init();
    wbBlueSlider.init();

    img->update();
  }
}



void PF::RawDeveloperConfigDialog::on_cam_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
				Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);
  
  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Open clicked." << std::endl;

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



void PF::RawDeveloperConfigDialog::on_out_button_open_clicked()
{
  Gtk::FileChooserDialog dialog("Please choose a file",
				Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);
  
  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK): 
    {
      std::cout << "Open clicked." << std::endl;

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



void PF::RawDeveloperConfigDialog::on_cam_filename_changed()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = camProfFileEntry.get_text();
    if( filename.empty() )
      return;
    std::cout<<"New input profile name: "<<filename<<std::endl;
    PF::RawDeveloperPar* par = 
      dynamic_cast<PF::RawDeveloperPar*>(get_layer()->get_processor()->get_par());
    if( !par )
      return;
    PropertyBase* prop = par->get_property( "cam_profile_name" );
    if( !prop ) 
      return;
    prop->update( filename );
    get_layer()->set_dirty( true );
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}



void PF::RawDeveloperConfigDialog::on_out_filename_changed()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string filename = outProfFileEntry.get_text();
    if( filename.empty() )
      return;
    std::cout<<"New output profile name: "<<filename<<std::endl;
    PF::RawDeveloperPar* par = 
      dynamic_cast<PF::RawDeveloperPar*>(get_layer()->get_processor()->get_par());
    if( !par )
      return;
    PropertyBase* prop = par->get_property( "out_profile_name" );
    if( !prop ) 
      return;
    prop->update( filename );
    get_layer()->set_dirty( true );
    std::cout<<"  updating image"<<std::endl;
    get_layer()->get_image()->update();
  }
}
