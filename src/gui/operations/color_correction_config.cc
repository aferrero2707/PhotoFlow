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


#include "../../base/color.hh"
#include "../../operations/color_correction.hh"
#include "color_correction_config.hh"



static double slope_conv(double& val, PF::OperationConfigGUI*, void*)
{
  //std::cout<<"slope_conv: val="<<val<<std::endl;
  //return( val + 1.f);
  return( val );
}

static double slope_conv_inv(double& val, PF::OperationConfigGUI*, void*)
{
  //return( val - 1.f);
  return( val );
}

static double pow_conv(double& val, PF::OperationConfigGUI*, void*)
{
  //return( 1.f - val);
  return( val);
}

static double pow_conv_inv(double& val, PF::OperationConfigGUI*, void*)
{
  //return( 1.f - val);
  return( val);
}



PF::ColorCorrectionConfigGUI::ColorCorrectionConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Color Correction" ),
  load_button(_("load")),
  save_button(_("save")),
  offs_frame(_("offset")),
  slope_frame(_("slope")),
  pow_frame(_("power")),
  offs_slider( this, "offs", _("offset (x10)"), 0, -10, 10, 0.01, 0.05, 10),
  r_offs_slider( this, "r_offs", _("red (x10)"), 0, -10, 10, 0.01, 0.05, 10),
  g_offs_slider( this, "g_offs", _("green (x10)"), 0, -10, 10, 0.01, 0.05, 10),
  b_offs_slider( this, "b_offs", _("blue (x10)"), 0, -10, 10, 0.01, 0.05, 10),
  slope_slider( this, "slope", _("slope"), 0, 0, 100, 0.01, 0.05, 1),
  r_slope_slider( this, "r_slope", _("red"), 0, 0, 100, 0.01, 0.05, 1),
  g_slope_slider( this, "g_slope", _("green"), 0, 0, 100, 0.01, 0.05, 1),
  b_slope_slider( this, "b_slope", _("blue"), 0, 0, 100, 0.01, 0.05, 1),
  pow_slider( this, "pow", _("power"), 0, 0, 100, 0.01, 0.05, 1),
  r_pow_slider( this, "r_pow", _("red"), 0, 0, 100, 0.01, 0.05, 1),
  g_pow_slider( this, "g_pow", _("green"), 0, 0, 100, 0.01, 0.05, 1),
  b_pow_slider( this, "b_pow", _("blue"), 0, 0, 100, 0.01, 0.05, 1),
  saturation_slider( this, "saturation", _("saturation"), 0, 0, 100, 0.01, 0.1, 1),
  is_log( this, "log_encoding", _("log encoding"), false )
{
  button_hbox.pack_start(load_button, Gtk::PACK_EXPAND_WIDGET, 5);
  button_hbox.pack_start(save_button, Gtk::PACK_EXPAND_WIDGET, 5);
  controlsBox.pack_start( button_hbox, Gtk::PACK_SHRINK, 2 );

  load_button.signal_clicked().connect(
      sigc::mem_fun(*this,&PF::ColorCorrectionConfigGUI::on_button_load) );
  save_button.signal_clicked().connect(
      sigc::mem_fun(*this,&PF::ColorCorrectionConfigGUI::on_button_save) );

  slope_slider.set_conversion_functions( &slope_conv, &slope_conv_inv );
  r_slope_slider.set_conversion_functions( &slope_conv, &slope_conv_inv );
  g_slope_slider.set_conversion_functions( &slope_conv, &slope_conv_inv );
  b_slope_slider.set_conversion_functions( &slope_conv, &slope_conv_inv );

  pow_slider.set_conversion_functions( &pow_conv, &pow_conv_inv );
  r_pow_slider.set_conversion_functions( &pow_conv, &pow_conv_inv );
  g_pow_slider.set_conversion_functions( &pow_conv, &pow_conv_inv );
  b_pow_slider.set_conversion_functions( &pow_conv, &pow_conv_inv );

  slope_box.pack_start( slope_slider, Gtk::PACK_SHRINK );
  slope_box.pack_start( r_slope_slider, Gtk::PACK_SHRINK );
  slope_box.pack_start( g_slope_slider, Gtk::PACK_SHRINK );
  slope_box.pack_start( b_slope_slider, Gtk::PACK_SHRINK );
  slope_frame.add(slope_box);
  controlsBox.pack_start( slope_frame, Gtk::PACK_SHRINK, 4 );

  offs_box.pack_start( offs_slider, Gtk::PACK_SHRINK );
  offs_box.pack_start( r_offs_slider, Gtk::PACK_SHRINK );
  offs_box.pack_start( g_offs_slider, Gtk::PACK_SHRINK );
  offs_box.pack_start( b_offs_slider, Gtk::PACK_SHRINK );
  offs_frame.add(offs_box);
  controlsBox.pack_start( offs_frame, Gtk::PACK_SHRINK, 4 );

  pow_box.pack_start( pow_slider, Gtk::PACK_SHRINK );
  pow_box.pack_start( r_pow_slider, Gtk::PACK_SHRINK );
  pow_box.pack_start( g_pow_slider, Gtk::PACK_SHRINK );
  pow_box.pack_start( b_pow_slider, Gtk::PACK_SHRINK );
  pow_frame.add(pow_box);
  controlsBox.pack_start( pow_frame, Gtk::PACK_SHRINK, 4 );

  controlsBox.pack_start( saturation_slider, Gtk::PACK_SHRINK, 4 );
  controlsBox.pack_start( is_log, Gtk::PACK_SHRINK );

  add_widget( controlsBox );
}


void PF::ColorCorrectionConfigGUI::on_button_load()
{
  Gtk::FileChooserDialog dialog(_("Open preset"),
        Gtk::FILE_CHOOSER_ACTION_OPEN);
  //dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

/*
#ifdef GTKMM_2
  Gtk::FileFilter filter_pfp;
  filter_pfp.set_name( _("Photoflow presets") );
  filter_pfp.add_pattern("*.pfp");
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::FileFilter> filter_pfp = Gtk::FileFilter::create();
  filter_pfp->set_name( _("Photoflow presets") );
  filter_pfp->add_pattern("*.pfp");
#endif
  dialog.add_filter(filter_pfp);
*/

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  std::string filename;

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK):
    {
      std::cout << "Save clicked." << std::endl;

      //Notice that this is a std::string, not a Glib::ustring.
      filename = dialog.get_filename();
#ifndef NDEBUG
      std::cout << "File selected: " <<  filename << std::endl;
#endif
      break;
    }
  case(Gtk::RESPONSE_CANCEL):
    {
      std::cout << "Cancel clicked." << std::endl;
      return;
    }
  default:
    {
      std::cout << "Unexpected button clicked." << std::endl;
      return;
    }
  }

  load_preset( filename );
}



enum cdl_parse_state_t
{
  CDL_PARSE_NONE,
  CDL_PARSE_SLOPE,
  CDL_PARSE_OFFSET,
  CDL_PARSE_POWER,
  CDL_PARSE_SATURATION,
};

static cdl_parse_state_t cdl_parse_state = CDL_PARSE_NONE;


/* The handler functions. */

static void start_element (GMarkupParseContext *context,
                    const gchar         *element_name,
                    const gchar        **attribute_names,
                    const gchar        **attribute_values,
                    gpointer             user_data,
                    GError             **error) {

  const gchar **name_cursor = attribute_names;
  const gchar **value_cursor = attribute_values;

  printf("start_element: %s found\n", element_name);
  if( strcmp (element_name, "Slope") == 0 ) {
    cdl_parse_state = CDL_PARSE_SLOPE;
  } else if( strcmp (element_name, "Offset") == 0 ) {
    cdl_parse_state = CDL_PARSE_OFFSET;
  } else if( strcmp (element_name, "Power") == 0 ) {
    cdl_parse_state = CDL_PARSE_POWER;
  } else if( strcmp (element_name, "Saturation") == 0 ) {
    cdl_parse_state = CDL_PARSE_SATURATION;
  }
}

static void text(GMarkupParseContext *context,
          const gchar         *text,
          gsize                text_len,
          gpointer             user_data,
          GError             **error)
{
  /* Note that "text" is not a regular C string: it is
   * not null-terminated. This is the reason for the
   * unusual %*s format below.
   */
  PF::ColorCorrectionPar* par = (PF::ColorCorrectionPar*)(user_data);
  if( !par ) return;
  if( text_len == 0 ) return;

  char* str = new char[text_len+1];
  memcpy(str, text, text_len); str[text_len] = '\0';

  printf("str: %s\n", str);
  switch(cdl_parse_state) {
  case CDL_PARSE_SLOPE: {
    std::istringstream istr(str);
    float r, g, b;
    istr>>r>>g>>b;
    std::cout<<"r="<<r<<" g="<<g<<" b="<<b<<std::endl;
    par->set_slope(r, g, b);
    break;
  }
  case CDL_PARSE_OFFSET: {
    std::istringstream istr(str);
    float r, g, b;
    istr>>r>>g>>b;
    std::cout<<"r="<<r<<" g="<<g<<" b="<<b<<std::endl;
    par->set_offset(r, g, b);
    break;
  }
  case CDL_PARSE_POWER: {
    std::istringstream istr(str);
    float r, g, b;
    istr>>r>>g>>b;
    std::cout<<"r="<<r<<" g="<<g<<" b="<<b<<std::endl;
    par->set_power(r, g, b);
    break;
  }
  case CDL_PARSE_SATURATION: {
    std::istringstream istr(str);
    float s;
    istr>>s;
    std::cout<<"s="<<s<<std::endl;
    par->set_saturation(s);
    break;
  }
  default:
    break;
  }
  delete[] str;
}

static void end_element (GMarkupParseContext *context,
                  const gchar         *element_name,
                  gpointer             user_data,
                  GError             **error)
{
  cdl_parse_state = CDL_PARSE_NONE;
  // If element is a layer, pop it from the stack
  printf("end_element: %s found\n", element_name);
  if( strcmp (element_name, "Slope") == 0 ) {
  }
}

/* The list of what handler does what. */
static GMarkupParser parser = {
  start_element,
  end_element,
  text,
  NULL,
  NULL
};


void PF::ColorCorrectionConfigGUI::load_preset(std::string filename)
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::ColorCorrectionPar* par =
        dynamic_cast<PF::ColorCorrectionPar*>(get_layer()->get_processor()->get_par());
    if( !par ) return;

    char *text;
    gsize length;
    GMarkupParseContext *context = g_markup_parse_context_new (&parser,
        (GMarkupParseFlags)0,
        par,
        NULL);
    /* seriously crummy error checking */

    if (g_file_get_contents (filename.c_str(), &text, &length, NULL) == FALSE) {
      printf("ColorCorrectionConfigGUI::load_preset: couldn't load XML\n");
      return;
    }

    if (g_markup_parse_context_parse (context, text, length, NULL) == FALSE) {
      printf("ColorCorrectionConfigGUI::load_preset: parse failed\n");
      return;
    }

    g_free(text);
    g_markup_parse_context_free (context);

    offs_slider.init();
    r_offs_slider.init();
    g_offs_slider.init();
    b_offs_slider.init();

    slope_slider.init();
    r_slope_slider.init();
    g_slope_slider.init();
    b_slope_slider.init();

    pow_slider.init();
    r_pow_slider.init();
    g_pow_slider.init();
    b_pow_slider.init();

    saturation_slider.init();

    get_layer()->get_image()->update();
  }
}



void PF::ColorCorrectionConfigGUI::on_button_save()
{
  Gtk::FileChooserDialog dialog( _("Save preset as..."),
        Gtk::FILE_CHOOSER_ACTION_SAVE);
  //dialog.set_transient_for(*this);

  //Add response buttons the the dialog:
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
/*
#ifdef GTKMM_2
  Gtk::FileFilter filter_pfp;
  filter_pfp.set_name( _("Photoflow presets") );
  filter_pfp.add_pattern("*.pfp");
#endif
#ifdef GTKMM_3
  Glib::RefPtr<Gtk::FileFilter> filter_pfp = Gtk::FileFilter::create();
  filter_pfp->set_name( _("Photoflow presets") );
  filter_pfp->add_pattern("*.pfp");
#endif
  dialog.add_filter(filter_pfp);
*/

  //Show the dialog and wait for a user response:
  int result = dialog.run();

  std::string filename;

  //Handle the response:
  switch(result) {
  case(Gtk::RESPONSE_OK):
    {
      std::cout << "Save clicked." << std::endl;

      //Notice that this is a std::string, not a Glib::ustring.
      filename = dialog.get_filename();
#ifndef NDEBUG
      std::cout << "File selected: " <<  filename << std::endl;
#endif
      break;
    }
  case(Gtk::RESPONSE_CANCEL):
    {
      std::cout << "Cancel clicked." << std::endl;
      return;
    }
  default:
    {
      std::cout << "Unexpected button clicked." << std::endl;
      return;
    }
  }

  save_preset(filename);
}


void PF::ColorCorrectionConfigGUI::save_preset(std::string filename)
{
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    PF::ColorCorrectionPar* par =
        dynamic_cast<PF::ColorCorrectionPar*>(get_layer()->get_processor()->get_par());
    if( !par ) return;

    std::ofstream of;
    of.open( filename.c_str() );
    if( !of ) return;

    of<<"<ColorCorrection id=\"\">"<<std::endl<<"  <SOPNode>"<<std::endl<<"    <Description></Description>"<<std::endl;
    of<<"    <Slope>"<<par->get_r_slope()<<" "<<par->get_g_slope()<<" "<<par->get_b_slope()<<"</Slope>"<<std::endl;
    of<<"    <Offset>"<<par->get_r_offset()<<" "<<par->get_g_offset()<<" "<<par->get_b_offset()<<"</Offset>"<<std::endl;
    of<<"    <Power>"<<par->get_r_power()<<" "<<par->get_g_power()<<" "<<par->get_b_power()<<"</Power>"<<std::endl;
    of<<"  </SOPNode>"<<std::endl;
    of<<"  <SatNode>"<<std::endl;
    of<<"    <Saturation>"<<par->get_saturation()<<"</Saturation>"<<std::endl;
    of<<"  </SatNode>"<<std::endl;
    of<<"</ColorCorrection>"<<std::endl;
  }
}

