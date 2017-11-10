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


#include "../base/imageprocessor.hh"
#include "sampler.hh"


//#define OPTIMIZE_SCROLLING


static int pxm_[8][16][16] = {
{// 1
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0}
},
{// 2
    {0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0}
},
{// 3
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
{// 1
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
{// 1
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
{// 1
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
{// 1
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
{// 1
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}};


gboolean PF::Sampler::queue_draw_cb (PF::Sampler::Update * update)
{
  update->sampler->set_values(update->val, update->lch, update->type);
  //std::cout<<"update->histogram->queue_draw() called"<<std::endl;
  g_free (update);
  return FALSE;
}



PF::Sampler::Sampler( Pipeline* v, Glib::ustring title, int i ):
      PipelineSink( v ), Gtk::Frame(title),
      display_merged( true ),
      sampler_x(10), sampler_y(10), sampler_size(3),
      active_layer( -1 ), enabled( false ), grabbed( true ), id(i)
{
  //set_size_request( 290, 100 );

  Pango::FontDescription font_desc("monospace 11");

  label_value1.modify_font(font_desc);
  label_value2.modify_font(font_desc);
  label_value3.modify_font(font_desc);
  label_value4.modify_font(font_desc);

  labels_box.pack_start(label_value1,Gtk::PACK_SHRINK);
  labels_box.pack_start(label_value2,Gtk::PACK_SHRINK);
  labels_box.pack_start(label_value3,Gtk::PACK_SHRINK);
  //labels_box.pack_start(label_value1);

  float tmp[4] = {0,0,0,0};
  set_values(tmp, tmp, VIPS_INTERPRETATION_RGB);

  //labels_box.pack_start(LCH_label_value1,Gtk::PACK_SHRINK);
  //labels_box.pack_start(LCH_label_value2,Gtk::PACK_SHRINK);
  //labels_box.pack_start(LCH_label_value3,Gtk::PACK_SHRINK);

  hbox.pack_start(labels_box,Gtk::PACK_SHRINK);

  check.set_active( enabled );
  hbox.pack_start(check,Gtk::PACK_SHRINK);

  check.signal_toggled().connect(sigc::mem_fun(*this,&PF::Sampler::enable_changed));

  add(hbox);

  show_all();
}

PF::Sampler::~Sampler ()
{
}


void PF::Sampler::set_values(float val[4], float lch[3], VipsInterpretation type)
{
  char tstr[500];
  char ch[4] = {'x','x','x','x'};
  switch(type) {
  case VIPS_INTERPRETATION_RGB:
  case VIPS_INTERPRETATION_sRGB:
  case VIPS_INTERPRETATION_MULTIBAND:
    ch[0] = 'R'; ch[1] = 'G'; ch[2] = 'B'; break;
  case VIPS_INTERPRETATION_LAB:
    ch[0] = 'L'; ch[1] = 'a'; ch[2] = 'b'; break;
  case VIPS_INTERPRETATION_CMYK:
    ch[0] = 'C'; ch[1] = 'M'; ch[2] = 'Y'; ch[2] = 'K'; break;
  }
  snprintf(tstr,499,"%c=%7.2 f  L=%7.2 f",ch[0], val[0], lch[0]);
  label_value1.set_text(tstr);
  snprintf(tstr,499,"%c=%7.2 f  C=%7.2 f",ch[1], val[1], lch[1]);
  label_value2.set_text(tstr);
  snprintf(tstr,499,"%c=%7.2 f  H=%7.2 f",ch[2], val[2], lch[2]);
  label_value3.set_text(tstr);
  snprintf(tstr,499,"%0.2f",val[3]);
  label_value4.set_text(tstr);

  snprintf(tstr,499,"L=%0.2f",lch[0]);
  LCH_label_value1.set_text(tstr);
  snprintf(tstr,499,"C=%0.2f",lch[1]);
  LCH_label_value2.set_text(tstr);
  snprintf(tstr,499,"H=%0.2f",lch[2]);
  LCH_label_value3.set_text(tstr);
}



void PF::Sampler::enable_changed()
{
  enabled = check.get_active();
  if( get_pipeline() && get_pipeline()->get_image() ) {
    std::cout<<"Sampler::enable_changed(): get_pipeline()->get_image()->update() called."<<std::endl;
    get_pipeline()->get_image()->update();
  }
}




static VipsImage* convert_raw_data( VipsImage* raw )
{
  if( !raw ) return NULL;

  // The image to be displayed has two channels, we assume in this case that it represents RAW data
  // and we only show the first channel (the second channel contains the color information)
  VipsImage* band = NULL;
  if( vips_extract_band( raw, &band, 0, NULL ) ) {
    std::cout<<"Sampler::update(): vips_extract_band() failed."<<std::endl;
    return NULL;
  }

  vips_image_init_fields( band,
      raw->Xsize, raw->Ysize,
      1, raw->BandFmt,
      raw->Coding,
      raw->Type,
      1.0, 1.0);

  VipsImage* norm = NULL;
  double par1 = 1.0f/65535.0f;
  double par2 = 0;
  if( vips_linear( band, &norm, &par1, &par2, 1, NULL ) ) {
    PF_UNREF( band, "Sampler::update(): band unref after vips_linear() failure" );
    std::cout<<"Sampler::update(): vips_linear() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( band, "Sampler::update(): band unref after vips_linear()" );

  VipsImage* gamma = NULL;
  float exp = 2.2;
  if( vips_gamma( norm, &gamma, "exponent", exp, NULL ) ) {
    PF_UNREF( norm, "Sampler::update(): norm unref after vips_gamma() failure" );
    std::cout<<"Sampler::update(): vips_gamma() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( norm, "Sampler::update(): norm unref after vips_gamma()" );

  VipsImage* cast = gamma;
  /*
if( vips_cast_ushort( gamma, &cast, NULL ) ) {
  PF_UNREF( gamma, "Sampler::update(): gamma unref after vips_cast_ushort() failure" );
  std::cout<<"Sampler::update(): vips_cast_ushort() failed."<<std::endl;
  return NULL;
}
PF_UNREF( gamma, "Sampler::update(): gamma unref after vips_cast_ushort()" );
   */

  VipsImage* bandv[3] = {cast, cast, cast};
  VipsImage* out = NULL;
  if( vips_bandjoin( bandv, &out, 3, NULL ) ) {
    PF_UNREF( cast, "Sampler::update(): cast unref after bandjoin failure" );
    std::cout<<"Sampler::update(): vips_bandjoin() failed."<<std::endl;
    return NULL;
  }
  PF_UNREF( cast, "Sampler::update(): cast unref after bandjoin" );

  return out;
}



void PF::Sampler::update( VipsRect* area )
{
  //PF::Pipeline* pipeline = pf_image->get_pipeline(0);

  if( !enabled ) return;

  //#ifdef DEBUG_DISPLAY
  std::cout<<"PF::Sampler::update(): called"<<std::endl;
  //#endif
  if( !get_pipeline() ) {
    std::cout<<"Sampler::update(): error: NULL pipeline"<<std::endl;
    return;
  }
  if( !get_pipeline()->get_output() ) {
    std::cout<<"Sampler::update(): error: NULL image"<<std::endl;
    return;
  }

  //return;

  VipsImage* image = NULL;
  bool do_merged = display_merged;
  //std::cout<<"Sampler::update(): do_merged="<<do_merged<<"  active_layer="<<active_layer<<std::endl;
  if( !do_merged ) {
    if( active_layer < 0 ) do_merged = true;
    else {
      PF::PipelineNode* node = get_pipeline()->get_node( active_layer );
      if( !node ) do_merged = true;
      //std::cout<<"Sampler::update(): node="<<node<<std::endl;
      if( node->processor &&
          node->processor->get_par() &&
          node->processor->get_par()->is_map() )
        do_merged = true;
      if( get_pipeline()->get_image() ) {
        PF::Layer* temp_layer = get_pipeline()->get_image()->get_layer_manager().get_layer( active_layer );
        if( !temp_layer ) do_merged = true;
        if( !(temp_layer->is_visible()) ) do_merged = true;
      }
    }
  }
  if( do_merged ) {
    image = get_pipeline()->get_output();
    //#ifdef DEBUG_DISPLAY
    std::cout<<"Sampler::update(): image="<<image<<std::endl;
    std::cout<<"                     image->Bands="<<image->Bands<<std::endl;
    std::cout<<"                     image->BandFmt="<<image->BandFmt<<std::endl;
    std::cout<<"                     image size: "<<image->Xsize<<"x"<<image->Ysize<<std::endl;
    //#endif
    if( image && (image->Bands!=2) ) {
      PF_REF( image, "Sampler::update(): merged image ref" );
      std::cout<<"PF_REF(image) called"<<std::endl;
    } else {
      image = convert_raw_data( image );
    }
  } else {
    PF::PipelineNode* node = get_pipeline()->get_node( active_layer );
    if( !node ) return;
    if( !(node->blended) ) return;

    image = node->blended;
    //#ifdef DEBUG_DISPLAY
    std::cout<<"Sampler::update(): node->image("<<node->image<<")->Xsize="<<node->image->Xsize
        <<"    node->image->Ysize="<<node->image->Ysize<<std::endl;
    std::cout<<"Sampler::update(): node->blended("<<node->blended<<")->Xsize="<<node->blended->Xsize
        <<"    node->blended->Ysize="<<image->Ysize<<std::endl;
    //#endif
    if( image && (image->Bands!=2) ) {
      PF_REF( image, "Sampler::update(): active image ref" );
    } else {
      image = convert_raw_data( image );
    }
  }
  if( !image ) return;

  PF::ICCProfile* img_profile = PF::get_icc_profile( image );

  VipsRect tile_area = { sampler_x-sampler_size/2, sampler_y-sampler_size/2, sampler_size, sampler_size };
  VipsRect image_area = { 0, 0, image->Xsize, image->Ysize };

  vips_rect_intersectrect( &image_area, &tile_area, &tile_area );
  if( tile_area.width<=0 || tile_area.height<=0 ) {
    std::cout<<"CacheBuffer::step_cb(): empty tile area."<<std::endl;
    PF_UNREF( image, "Sampler::update(): image unref after vips_region_prepare()" );
    return;
  }

  //if( tile_area.left == 0 ) std::cout<<"CacheBuffer::step(): row="<<tile_area.top<<std::endl;

  float tot[4] = {0,0,0,0};
  // Update the image region corresponding to the current tile
  VipsRegion* reg = vips_region_new( image );
  if( vips_region_prepare( reg, &tile_area ) ) {
    std::cout<<"CacheBuffer::step_cb(): vips_region_prepare() failed."<<std::endl;
    VIPS_UNREF( reg );
    PF_UNREF( image, "Sampler::update(): image unref after vips_region_prepare()" );
    return;
  }

  float* p;
  off_t offset = VIPS_IMAGE_SIZEOF_LINE(image)*tile_area.top+VIPS_IMAGE_SIZEOF_PEL(image)*tile_area.left;
  int sampler_area = tile_area.height*tile_area.width;
  for( int y = 0; y < tile_area.height; y++ ) {
    p = (float*)VIPS_REGION_ADDR( reg, tile_area.left, tile_area.top+y );
    for( int x = 0; x < tile_area.width; x++ ) {
      for( int c = 0; c < image->Bands; c++ ) {
        tot[c] += p[c];
      }
      p += image->Bands;
    }
  }
  VIPS_UNREF( reg );
  PF_UNREF( image, "Sampler::update(): image unref after vips_region_prepare()" );

  transform.init(img_profile, PF::ICCStore::Instance().get_Lab_profile(),
      image->BandFmt, INTENT_RELATIVE_COLORIMETRIC, true, 0);

  float lch[3];
  for( int c = 0; c < image->Bands; c++ ) tot[c] /= sampler_area;
  if( vips_image_get_interpretation(image) == VIPS_INTERPRETATION_LAB ) {
    PF::Lab_pf2lcms( tot );
    lch[0] = tot[0];
    lch[1] = std::sqrt(tot[1]*tot[1] + tot[2]*tot[2]);
    lch[2] = std::atan2(tot[2],tot[1])*180.f/M_PI;
    if(lch[2] < 0) lch[2] += 360;
  } else {
    float lab[3];
    transform.apply(tot, lab, 1);
    lch[0] = lab[0];
    lch[1] = std::sqrt(lab[1]*lab[1] + lab[2]*lab[2]);
    lch[2] = std::atan2(lab[2],lab[1])*180.f/M_PI;
    if(lch[2] < 0) lch[2] += 360;
    for( int c = 0; c < image->Bands; c++ ) tot[c] *= 100;
  }

  Update * update = g_new (Update, 1);
  update->sampler = this;
  update->type = vips_image_get_interpretation(image);
  for( int c = 0; c < image->Bands; c++ ) {
    std::cout<<"PF::Sampler::update(): tot["<<c<<"]="<<tot[c]<<std::endl;
    update->val[c] = tot[c];
    std::cout<<"PF::Sampler::update(): update->val["<<c<<"]="<<update->val[c]<<std::endl;
  }
  for( int c = 0; c < 3; c++ ) {
    std::cout<<"PF::Sampler::update(): lch["<<c<<"]="<<lch[c]<<std::endl;
    update->lch[c] = lch[c];
    std::cout<<"PF::Sampler::update(): update->lch["<<c<<"]="<<update->lch[c]<<std::endl;
  }
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
}



bool PF::Sampler::pointer_press_event( int button, double x, double y, double D, int mod_key )
{
  if( !enabled ) return false;
  grabbed = false;

  double px = 0, py = 0;
  px = sampler_x;
  py = sampler_y;
  double dx = x - px;
  double dy = y - py;
  if( (fabs(dx) > D) || (fabs(dy) > D) ) return false;

  grabbed = true;

  return true;
}


bool PF::Sampler::pointer_release_event( int button, double x, double y, int mod_key )
{
  if( !enabled ) return false;
  if( !grabbed ) return false;

  grabbed = false;

  if( get_pipeline() && get_pipeline()->get_image() ) {
    std::cout<<"Sampler::pointer_release_event(): get_pipeline()->get_image()->update() called."<<std::endl;
    get_pipeline()->get_image()->update();
  }
  return true;
}


bool PF::Sampler::pointer_motion_event( int button, double x, double y, int mod_key )
{
  if( !enabled ) return false;
  if( !grabbed ) return false;

  sampler_x = x;
  sampler_y = y;

  return true;
}


bool PF::Sampler::modify_preview( PF::PixelBuffer& buf_out, float scale, int xoffset, int yoffset )
{
  if( !enabled ) return false;

  std::cout<<"Sampler::modify_preview() called"<<std::endl
      <<"  scale="<<scale<<std::endl
      <<"  sampler_x="<<sampler_x<<std::endl
      <<"  sampler_y="<<sampler_y<<std::endl;

  double cx = sampler_x * scale;
  double cy = sampler_y * scale;
  buf_out.draw_line( cx, cy-10, cx, cy+10, buf_out );
  //buf_out.draw_line( cx-1, cy-10, cx-1, cy+10, buf_out );
  //buf_out.draw_line( cx+1, cy-10, cx+1, cy+10, buf_out );

  buf_out.draw_line( cx-10, cy, cx+10, cy, buf_out );
  //buf_out.draw_line( cx-10, cy-1, cx+10, cy-1, buf_out );
  //buf_out.draw_line( cx-10, cy+1, cx+10, cy+1, buf_out );

  buf_out.draw_line( cx-5, cy-5, cx-5, cy+5, buf_out );
  buf_out.draw_line( cx+5, cy-5, cx+5, cy+5, buf_out );
  buf_out.draw_line( cx-4, cy-5, cx+4, cy-5, buf_out );
  buf_out.draw_line( cx-4, cy+5, cx+4, cy+5, buf_out );

  for(int i = 0; i < 16; i++) {
    for(int j = 0; j < 16; j++) {
      if(pxm_[id][i][j] > 0)
        buf_out.draw_point( cx+10+j, cy-10+i, buf_out );
    }
  }
  return true;
}



PF::SamplerGroup::SamplerGroup( Pipeline* v ):
        s1(v, _("sampler 1"), 0),s2(v, _("sampler 2"), 1),
        s3(v, _("sampler 3"), 2),s4(v, _("sampler 4"), 3),
        s5(v, _("sampler 5"), 4),s6(v, _("sampler 6"), 5),
        s7(v, _("sampler 7"), 6),s8(v, _("sampler 8"), 7)
{
  row1.pack_start(s1,Gtk::PACK_SHRINK,2);
  row2.pack_start(s2,Gtk::PACK_SHRINK,2);
  row3.pack_start(s3,Gtk::PACK_SHRINK,2);
  row4.pack_start(s4,Gtk::PACK_SHRINK,2);
  //row2.pack_start(s5,Gtk::PACK_SHRINK,10);
  //row2.pack_start(s6,Gtk::PACK_SHRINK,10);
  //row2.pack_start(s7,Gtk::PACK_SHRINK,10);
  //row2.pack_start(s8,Gtk::PACK_SHRINK,10);
  box.pack_start(row1,Gtk::PACK_SHRINK,0);
  box.pack_start(row2,Gtk::PACK_SHRINK,0);
  box.pack_start(row3,Gtk::PACK_SHRINK,0);
  box.pack_start(row4,Gtk::PACK_SHRINK,0);

  add(box);
  set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
  set_shadow_type( Gtk::SHADOW_NONE );

  //pack_start(s1,Gtk::PACK_SHRINK,5);
  //pack_start(s2,Gtk::PACK_SHRINK,5);
  //pack_start(s3,Gtk::PACK_SHRINK,5);
  //pack_start(s4,Gtk::PACK_SHRINK,5);
}


PF::Sampler& PF::SamplerGroup::get_sampler(int i)
{
  switch(i) {
  case 0: return s1;
  case 1: return s2;
  case 2: return s3;
  case 3: return s4;
  case 4: return s5;
  case 5: return s6;
  case 6: return s7;
  case 7: return s8;
  default: return s1;
  }
}
