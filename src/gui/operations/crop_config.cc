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

#include "../../operations/crop.hh"

#include "crop_config.hh"


PF::CropConfigDialog::CropConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "Crop" ),
  handle( CROP_HANDLE_NONE ),
  cropLeftSlider( this, "crop_left", "Crop left", 0, 0, 10000000, 1, 10, 1),
  cropTopSlider( this, "crop_top", "Crop top", 0, 0, 10000000, 1, 10, 1),
  cropWidthSlider( this, "crop_width", "Crop width", 0, 0, 10000000, 1, 10, 1),
  cropHeightSlider( this, "crop_height", "Crop height", 0, 0, 10000000, 1, 10, 1),
  keepARCheckBox( this, "keep_ar", "Keep aspect ratio", 0 ),
  cropARWidthSlider( this, "ar_width", "Aspect ratio: W=", 0, 0, 10000000, 1, 10, 1),
  cropARHeightSlider( this, "ar_height", "/H=", 0, 0, 10000000, 1, 10, 1)
{
  controlsBox.pack_start( cropLeftSlider );
  controlsBox.pack_start( cropTopSlider );
  controlsBox.pack_start( cropWidthSlider );
  controlsBox.pack_start( cropHeightSlider );
  
  controlsBox.pack_start( keepARCheckBox );
  arControlsBox.pack_start( cropARWidthSlider );
  arControlsBox.pack_start( cropARHeightSlider );
  controlsBox.pack_start( arControlsBox );

  add_widget( controlsBox );
}



void PF::CropConfigDialog::open()
{
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    cropLeftSlider.init();
    cropTopSlider.init();
    cropWidthSlider.init();
    cropHeightSlider.init();
  }
  OperationConfigDialog::open();
}


bool PF::CropConfigDialog::pointer_press_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;
  handle = CROP_HANDLE_NONE;

  if( !get_layer() ) return false;
  if( !get_layer()->get_image() ) return false;
  if( !get_layer()->get_processor() ) return false;
  if( !get_layer()->get_processor()->get_par() ) return false;

  PF::OpParBase* par = get_layer()->get_processor()->get_par();
  /*
  PF::PropertyBase* crop_left_p = par->get_property( "crop_left" );
  PF::PropertyBase* crop_top_p = par->get_property( "crop_top" );
  PF::PropertyBase* crop_width_p = par->get_property( "crop_width" );
  PF::PropertyBase* crop_height_p = par->get_property( "crop_height" );
  if( !crop_left_p || !crop_top_p || !crop_width_p || !crop_height_p ) return false;
  int crop_left; crop_left_p->get(crop_left);
  int crop_top; crop_top_p->get(crop_top);
  int crop_width; crop_width_p->get(crop_width);
  int crop_height; crop_height_p->get(crop_height);
  */
  int crop_left = cropLeftSlider.get_adjustment()->get_value();
  int crop_top = cropTopSlider.get_adjustment()->get_value();
  int crop_width = cropWidthSlider.get_adjustment()->get_value();
  int crop_height = cropHeightSlider.get_adjustment()->get_value();
  int crop_bottom = crop_top + crop_height - 1;
  int crop_right = crop_left + crop_width - 1;

  cropLeftSlider.set_inhibit( true );
  cropTopSlider.set_inhibit( true );
  cropWidthSlider.set_inhibit( true );
  cropHeightSlider.set_inhibit( true );

  if( (crop_width == 0) || (crop_height == 0) ) {
    // No cropping area defined yet
    cropLeftSlider.get_adjustment()->set_value((int)x);
    cropTopSlider.get_adjustment()->set_value((int)y);
    cropWidthSlider.get_adjustment()->set_value(1);
    cropHeightSlider.get_adjustment()->set_value(1);

    handle = CROP_HANDLE_BOTTOMRIGHT;

    return true;
  }

  if( !get_layer()->get_image()->get_pipeline(PREVIEW_PIPELINE_ID) ) return false;
  float scale = 1;
  for( unsigned int l = 1; l <= get_layer()->get_image()->get_pipeline(PREVIEW_PIPELINE_ID)->get_level(); l++ )
    scale *= 2;

  double dx1 = x - crop_left; dx1 /= scale;
  double dx2 = x - crop_right; dx2 /= scale;
  double dy1 = y - crop_top; dy1 /= scale;
  double dy2 = y - crop_bottom; dy2 /= scale;

  bool is_right = ( (dx2 >= -10) && (dx2 <= 5) ) ? true : false;
  bool is_bottom = ( (dy2 >= -10) && (dy2 <= 5) ) ? true : false;

  if( is_right && is_bottom )
    handle = CROP_HANDLE_BOTTOMRIGHT;
  else if( is_right )
    handle = CROP_HANDLE_RIGHT;
  else if( is_bottom )
    handle = CROP_HANDLE_BOTTOM;
  else if( (x > crop_left) && (x < crop_right) &&
           (y > crop_top) && (y < crop_bottom) )
    handle = CROP_HANDLE_CENTER;

  crop_center_dx = (int)(x - crop_left - crop_width/2);
  crop_center_dy = (int)(y - crop_top - crop_height/2);

  std::cout<<"handle: "<<handle<<std::endl;

  return true;
}


bool PF::CropConfigDialog::pointer_release_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;

  cropLeftSlider.set_value();
  cropTopSlider.set_value();
  cropWidthSlider.set_value();
  cropHeightSlider.set_value();

  cropLeftSlider.set_inhibit( false );
  cropTopSlider.set_inhibit( false );
  cropWidthSlider.set_inhibit( false );
  cropHeightSlider.set_inhibit( false );
  return true;
}


void PF::CropConfigDialog::move_handle( int x, int y )
{
  int x0 = cropLeftSlider.get_adjustment()->get_value();
  int y0 = cropTopSlider.get_adjustment()->get_value();
  switch( handle ) {
  case CROP_HANDLE_BOTTOMRIGHT: {
    int new_w = (x >= x0) ? x+1-x0 : 1;
    int new_h = (y >= y0) ? y+1-y0 : 1;
    if( keepARCheckBox.get_active() ) {
      int ar_w = new_h * cropARWidthSlider.get_adjustment()->get_value()/
          cropARHeightSlider.get_adjustment()->get_value();
      int ar_h = new_w * cropARHeightSlider.get_adjustment()->get_value()/
          cropARWidthSlider.get_adjustment()->get_value();
      if( ar_w>new_w ) new_w = ar_w;
      if( ar_h>new_h ) new_h = ar_h;
    }
    cropWidthSlider.get_adjustment()->set_value( new_w );
    cropHeightSlider.get_adjustment()->set_value( new_h );
    break;
  }
  case CROP_HANDLE_RIGHT: {
    //int x0 = cropLeftSlider.get_adjustment()->get_value();
    int new_w = (x >= x0) ? x+1-x0 : 1;
    int new_h = cropHeightSlider.get_adjustment()->get_value();
    if( keepARCheckBox.get_active() ) {
      //int ar_w = new_h * cropARWidthSlider.get_adjustment()->get_value()/
      //    cropARHeightSlider.get_adjustment()->get_value();
      int ar_h = new_w * cropARHeightSlider.get_adjustment()->get_value()/
          cropARWidthSlider.get_adjustment()->get_value();
      //if( ar_w>new_w ) new_w = ar_w;
      new_h = ar_h;
    }
    cropWidthSlider.get_adjustment()->set_value( new_w );
    cropHeightSlider.get_adjustment()->set_value( new_h );
    //if( x >= x0 ) cropWidthSlider.get_adjustment()->set_value( x+1-x0 );
    //else cropWidthSlider.get_adjustment()->set_value( 1 );
    break;
  }
  case CROP_HANDLE_BOTTOM: {
    //int y0 = cropTopSlider.get_adjustment()->get_value();
    int new_w = cropWidthSlider.get_adjustment()->get_value();
    int new_h = (y >= y0) ? y+1-y0 : 1;
    if( keepARCheckBox.get_active() ) {
      int ar_w = new_h * cropARWidthSlider.get_adjustment()->get_value()/
          cropARHeightSlider.get_adjustment()->get_value();
      //int ar_h = new_w * cropARHeightSlider.get_adjustment()->get_value()/
      //    cropARWidthSlider.get_adjustment()->get_value();
      //if( ar_w>new_w ) new_w = ar_w;
      new_w = ar_w;
    }
    cropWidthSlider.get_adjustment()->set_value( new_w );
    cropHeightSlider.get_adjustment()->set_value( new_h );
    //if( y >= y0 ) cropHeightSlider.get_adjustment()->set_value( y+1-y0 );
    //else cropHeightSlider.get_adjustment()->set_value( 1 );
    break;
  }
  case CROP_HANDLE_CENTER: {
    if( !get_layer() ) return;
    if( !get_layer()->get_image() ) return;
    if( !get_layer()->get_image()->get_pipeline(0) ) return;
    PF::PipelineNode* node = get_layer()->get_image()->get_pipeline(0)->get_node( get_layer()->get_id() );
    if( !node ) return;
    if( node->input_id < 0 ) return;
    PF::PipelineNode* node2 = get_layer()->get_image()->get_pipeline(0)->get_node( node->input_id );
    if( !node2 ) return;
    if( !node2->blended ) return;
    
    int x0 = cropLeftSlider.get_adjustment()->get_value();
    int xc = x0 + cropWidthSlider.get_adjustment()->get_value()/2;
    int y0 = cropTopSlider.get_adjustment()->get_value();
    int yc = y0 + cropHeightSlider.get_adjustment()->get_value()/2;
    int dx = x - (xc + crop_center_dx);
    int dy = y - (yc + crop_center_dy);
    int xnew = x0 + dx;
    int ynew = y0 + dy;
    if( xnew < 0 ) xnew = 0;
    if( ynew < 0 ) ynew = 0;
    if( (xnew + cropWidthSlider.get_adjustment()->get_value() - 1) > node2->blended->Xsize )
      xnew = node2->blended->Xsize - cropWidthSlider.get_adjustment()->get_value();
    if( (ynew + cropHeightSlider.get_adjustment()->get_value() - 1) > node2->blended->Ysize )
      ynew = node2->blended->Ysize - cropHeightSlider.get_adjustment()->get_value();
    cropLeftSlider.get_adjustment()->set_value( xnew );
    cropTopSlider.get_adjustment()->set_value( ynew );
    break;
  }
  }
}




bool PF::CropConfigDialog::pointer_motion_event( int button, double x, double y, int mod_key )
{
  if( button != 1 ) return false;

  int ix = x;
  int iy = y;

  move_handle( ix, iy );

  return true;
}




bool PF::CropConfigDialog::modify_preview( PixelBuffer& buf_in, PixelBuffer& buf_out, 
                                           float scale, int xoffset, int yoffset )
{
  if( !get_mapped() ) 
    return false;

  if( !get_layer() ) return false;
  if( !get_layer()->get_image() ) return false;
  if( !get_layer()->get_processor() ) return false;
  if( !get_layer()->get_processor()->get_par() ) return false;

  PF::OpParBase* par = get_layer()->get_processor()->get_par();

  /*
  PF::PropertyBase* crop_left_p = par->get_property( "crop_left" );
  PF::PropertyBase* crop_top_p = par->get_property( "crop_top" );
  PF::PropertyBase* crop_width_p = par->get_property( "crop_width" );
  PF::PropertyBase* crop_height_p = par->get_property( "crop_height" );
  if( !crop_left_p || !crop_top_p || !crop_width_p || !crop_height_p ) return false;
  int crop_left; crop_left_p->get(crop_left); crop_left *= scale; crop_left += xoffset;
  int crop_top; crop_top_p->get(crop_top); crop_top *= scale; crop_top += yoffset;
  int crop_width; crop_width_p->get(crop_width); crop_width *= scale; 
  int crop_height; crop_height_p->get(crop_height); crop_height *= scale;
  */
  int crop_left = cropLeftSlider.get_adjustment()->get_value(); crop_left *= scale; crop_left += xoffset;
  int crop_top = cropTopSlider.get_adjustment()->get_value(); crop_top *= scale; crop_top += yoffset;
  int crop_width = cropWidthSlider.get_adjustment()->get_value(); crop_width *= scale; 
  int crop_height = cropHeightSlider.get_adjustment()->get_value(); crop_height *= scale;
  int crop_bottom = crop_top + crop_height - 1;
  int crop_right = crop_left + crop_width - 1;
  
  if( (crop_width == 0) || (crop_height == 0) ) return false;

  // We only draw on top of the preview image if we are mapped

  // Resize the output buffer to match the input one
  buf_out.resize( buf_in.get_rect() );

  // Copy pixel data from input to output
  buf_out.copy( buf_in );

  guint8* px = buf_out.get_pxbuf()->get_pixels();
  const int rs = buf_out.get_pxbuf()->get_rowstride();
  const int bl = 3; /*buf->get_byte_length();*/

  int buf_left = buf_out.get_rect().left;
  int buf_right = buf_out.get_rect().left+buf_out.get_rect().width-1;
  int buf_top = buf_out.get_rect().top;
  int buf_bottom = buf_out.get_rect().top+buf_out.get_rect().height-1;

  int x, y;

  // Top dark band
  int bottom = ((crop_top-1) < buf_bottom) ? (crop_top-1) : buf_bottom;
  for( y = buf_out.get_rect().top; y <= bottom; y++ ) {
    guint8* p = px + rs*(y-buf_out.get_rect().top);
    for( x = 0; x < buf_out.get_rect().width; x++, p += 3 ) {
      p[0] = p[0]/2;
      p[1] = p[1]/2;
      p[2] = p[2]/2;
    }
  }

  // Bottom dark band
  int top = ((crop_bottom+1) > buf_top) ? (crop_bottom+1) : buf_top;
  for( y = top; y <= buf_bottom; y++ ) {
    guint8* p = px + rs*(y-buf_out.get_rect().top);// + xoffset*bl;
    for( x = 0; x < buf_out.get_rect().width; x++, p += 3 ) {
      p[0] = p[0]/2;
      p[1] = p[1]/2;
      p[2] = p[2]/2;
    }
  }

  if( crop_height > 10 ) {
    if( buf_top < (crop_bottom-10) ) {
      int left = (crop_left > buf_left) ? crop_left : buf_left;
      int right = (crop_right < buf_right) ? crop_right : buf_right;
      for( y = crop_bottom-10; y <= crop_bottom-9; y++) {
        guint8* p = px + rs*(y-buf_out.get_rect().top) + (left-buf_left)*bl;
        for( x = left; x <= right; x++, p += bl ) {
          p[0] = 255-p[0];
          p[1] = 255-p[1];
          p[2] = 255-p[2];
        }
      }
    }
  }

  // Left dark band
  top = (crop_top >= buf_top) ? crop_top : buf_top;
  bottom = (crop_bottom <= buf_bottom) ? crop_bottom : buf_bottom;
  int right = ((crop_left-1) <= buf_right) ? crop_left-1 : buf_right;
  int width = right+1-buf_left;
  //std::cout<<"crop_left-1="<<crop_left-1<<"  buf_right="<<buf_right<<"  right="<<right<<std::endl;
  for( y = top; y <= bottom; y++ ) {
    guint8* p = px + rs*(y-buf_out.get_rect().top);// + buf_left*bl;
    for( x = 0; x < width; x++, p += 3 ) {
      p[0] = p[0]/2;
      p[1] = p[1]/2;
      p[2] = p[2]/2;
    }
  }

  // Right dark band
  int left = ((crop_right+1) >= buf_left) ? crop_right+1 : buf_left;
  width = buf_right+1-left;
  for( y = top; y <= bottom; y++ ) {
    guint8* p = px + rs*(y-buf_out.get_rect().top) + (left-buf_left)*bl;
    for( x = 0; x < width; x++, p += 3 ) {
      p[0] = p[0]/2;
      p[1] = p[1]/2;
      p[2] = p[2]/2;
    }
  }

  if( crop_width > 10 ) {
    if( (buf_left < (crop_right-10)) &&
        (buf_right > (crop_right-10)) ) {
      int top = (crop_top > buf_top) ? crop_top : buf_top;
      int bottom = (crop_bottom < buf_bottom) ? crop_bottom : buf_bottom;
      for( y = top; y <= bottom; y++ ) {
        guint8* p = px + rs*(y-buf_out.get_rect().top) + (crop_right-10-buf_left)*bl;
        p[0] = 255-p[0];
        p[1] = 255-p[1];
        p[2] = 255-p[2];
        p[3] = 255-p[3];
        p[4] = 255-p[4];
        p[5] = 255-p[5];
      }
    }
  }

  return true;

  // Draw outline of cropped area
  if( ((crop_top-2) >= buf_top) &&
      ((crop_top-1) <= buf_bottom) ) {
    for( y = crop_top-2; y <= crop_top-1; y++ ) {
      guint8* p = px + rs*(y-buf_out.get_rect().top);
      for( x = 0; x < buf_out.get_rect().width; x++, p += 3 ) {
        p[0] = 255-p[0];
        p[1] = 255-p[1];
        p[2] = 255-p[2];
      }
    }
  }

  return true;
}
