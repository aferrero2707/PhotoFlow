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

#include <iostream>
#include <vips/vips.h>

#include "../base/photoflow.hh"

#include "operationstree.hh"
#include "layerwidget.hh"
#include "help.hh"

/*
#include "../operations/vips_operation.hh"
#include "../operations/image_reader.hh"
#include "../operations/brightness_contrast.hh"
#include "../operations/invert.hh"
#include "../operations/gradient.hh"
#include "../operations/convert2lab.hh"
#include "../operations/clone.hh"
#include "../operations/curves.hh"

#include "../gui/operations/brightness_contrast_config.hh"
#include "../gui/operations/imageread_config.hh"
#include "../gui/operations/vips_operation_config.hh"
#include "../gui/operations/clone_config.hh"
#include "../gui/operations/curves_config.hh"
 */

PF::OperationsTree::OperationsTree( )
{
  treeModel = Gtk::TreeStore::create(columns);
  set_model(treeModel);
  append_column("Name", get_columns().col_name);
  set_headers_visible( false );
}



PF::OperationsTree::~OperationsTree()
{
}


PF::OperationsTreeWidget::OperationsTreeWidget()
{
  textview.set_wrap_mode(Gtk::WRAP_WORD);
  //Glib::RefPtr< Gtk::TextBuffer >  buf = textview.get_buffer ();
  //buf->set_text("This is a test!!!");
  left_win.add( tree );
  left_frame.add( left_win );
  right_win.add( textview );
  right_frame.add( right_win );
  left_win.set_size_request(200,-1);
  right_win.set_size_request(300,-1);

  left_frame.set_label("Tool chooser");
  right_frame.set_label("Brief description");
  pack_start( left_frame );
  pack_start( right_frame );

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = tree.get_selection();
  refTreeSelection->signal_changed().connect(
      sigc::mem_fun(*this, &OperationsTreeWidget::on_selection_changed) );
}


void PF::OperationsTreeWidget::on_selection_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = tree.get_selection();
  Gtk::TreeStore::iterator iter = refTreeSelection->get_selected();
  Glib::RefPtr< Gtk::TextBuffer >  buf = textview.get_buffer ();
  //buf->set_text("Ths help is not yet available. Sorry.");
  if(iter) {
    Gtk::TreeStore::Row row = *iter;
    PF::OperationsTreeColumns& columns = tree.get_columns();
    Glib::ustring help = (*iter)[columns.col_help];
    buf->set_text( help );
    /*
    std::string op_type = (*iter)[columns.col_nickname];
    if( op_type == "curves" ) {
      buf->set_text("The curves tool allows to adjust the tonal range and colors of the image through control points.\n\n");
      buf->insert_at_cursor("The tool provides a separate curve for each image channel. In the case of RGB images, ");
      buf->insert_at_cursor("an additional curve, called \"RGB\", allows to modify all three channels at the same time.\n\n");
      buf->insert_at_cursor("In the graphs, the horizontal axis represents the input values and the vertical axis the output ones.");
      buf->insert_at_cursor("The curve is initially a simple diagonal line, meaning that the output exactly matches input.\n\n");
      buf->insert_at_cursor("Left-clicking on the graph with the mouse adds a new control point, while right-clicking on an existing ");
      buf->insert_at_cursor("control point deletes it. Control points can be moved by either dragging them with the moiuse, or by setting the corresponding input and output numerical values in the boxes below the graph.");
    }
     */
  }
}

static std::map< std::string, std::list<std::string> > vips_operations;
static std::string vips_category = "";

static void* collect_class( GType type )
{
  const int depth = vips_type_depth( type );
  VipsObjectClass *klass = VIPS_OBJECT_CLASS( g_type_class_ref( type ) );
  bool abstract = G_TYPE_IS_ABSTRACT( type );

  std::cout<<"depth: "<<depth<<"  nickname: "<<klass->nickname;
  if( !abstract ) 
    std::cout<<"  concrete"<<std::endl;
  else
    std::cout<<"  abstract"<<std::endl;

  if( depth == 2 && abstract ) {
    std::map< std::string, std::list<std::string> >::iterator i = 
        vips_operations.find( klass->nickname );
    if(i == vips_operations.end() ) {
      vips_operations.insert( make_pair( std::string(klass->nickname), 
          std::list<std::string>() ) );
      i = vips_operations.find( klass->nickname );
      if(i == vips_operations.end() ) 
        return( NULL );
    }
    vips_category = klass->nickname;
  } else if( depth >= 2 && !abstract ) {
    std::map< std::string, std::list<std::string> >::iterator i = 
        vips_operations.find( vips_category );
    if(i == vips_operations.end() ) 
      return( NULL );

    /* We need an instance to be able to call get_flags().
     */
    VipsOperation* operation = VIPS_OPERATION( g_object_new( type, NULL ) );
    VipsOperationFlags flags = vips_operation_get_flags( operation );

    int nout = 0;
    VIPS_ARGUMENT_FOR_ALL( operation, 
        pspec, argument_class, argument_instance ) {

      g_assert( argument_instance );

      if( (argument_class->flags & VIPS_ARGUMENT_OUTPUT) ) {
        GType otype = G_PARAM_SPEC_VALUE_TYPE( pspec );
        const gchar* arg_name = g_param_spec_get_name( pspec );

        if( g_type_is_a( otype, VIPS_TYPE_IMAGE ) ) nout += 1;
      }
    } 
    VIPS_ARGUMENT_FOR_ALL_END;

    std::cout<<"klass->nickname -> # of output images: "<<nout<<std::endl;

    g_object_unref( operation ); 

    if( nout==1 )
      //#if VIPS_MAJOR_VERSION < 8 && VIPS_MINOR_VERSION < 40
      //&& !(flags & VIPS_OPERATION_DEPRECATED)
      //#endif
      i->second.push_back( klass->nickname );
  }

  return( NULL );

}


void PF::OperationsTree::add_op( Glib::ustring name, const std::string nik)
{
  Gtk::TreeModel::Row row;

  row = *(treeModel->append());
  row[columns.col_name] = name;
  row[columns.col_nickname] = nik;
  Glib::ustring helpPath = Glib::ustring(INSTALL_PREFIX) + "/share/photoflow/help/en/" + nik + ".hlp";
  std::ifstream file(helpPath.c_str());
  Glib::ustring help;
  char ch;
  if( !file.fail() ) {
    while(!file.eof()) {
      //std::string tmpStr;
      //std::getline(file, tmpStr);
      //help += tmpStr;
      file.get( ch );
      if( !file.fail() ) help += ch;
    }
  } else {
    help = "Ths help is not yet available. Sorry.";
  }
  row[columns.col_help] = help;
  PF::operations_help_map.insert( std::make_pair(nik, help) );
}

/*
  return;

  row = *(treeModel->append());
  row[columns.col_name] = "-------------------";

  row_vips =  *(treeModel->append());
  row_vips[columns.col_name] = "VIPS";

  std::map< std::string, std::list<std::string> >::iterator i;
  for( i = vips_operations.begin(); i != vips_operations.end(); i++ ) {
    group = *(treeModel->append(row_vips.children()));
    group[columns.col_name] = i->first.c_str();
    std::list<std::string>::iterator j;
    for( j = i->second.begin(); j != i->second.end(); j++ ) {
      row = *(treeModel->append(group.children()));
      row[columns.col_name] = (*j).c_str();
      row[columns.col_nickname] = Glib::ustring("vips-")+(*j).c_str();
    }
  }



  //expand_all();
}
 */





PF::OperationsTreeDialog::OperationsTreeDialog( Image* img, LayerWidget* lw ):
      Gtk::Dialog( _("Add New Layer"),true),
      image( img ),
      layer_widget( lw )
{
  set_default_size(300,300);

  add_button( _("OK"), 1 );
  add_button( _("Cancel"), 0 );

  signal_response().connect( sigc::mem_fun(*this,
      &OperationsTreeDialog::on_button_clicked) );

  //op_load_box.add( op_load );
  notebook.append_page( op_load, "load" );

  //notebook.append_page( op_raw, "raw" );

  //op_conv_box.add( op_conv );
  //notebook.append_page( op_conv, "conv" );

  //op_color_box.add( op_color );
  notebook.append_page( op_color, "color" );

  //op_detail_box.add( op_detail );
  notebook.append_page( op_detail, "detail" );

  //op_geom_box.add( op_geom );
  notebook.append_page( op_geom, "geom" );

  //#ifndef PF_DISABLE_GMIC
  //op_gmic_box.add( op_gmic );
  notebook.append_page( op_gmic, "G'MIC" );
  //#endif

  //op_misc_box.add( op_misc );
  notebook.append_page( op_misc, "misc" );

  op_load.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );
  //op_raw.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );
  //op_conv.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );
  op_color.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );
  op_detail.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );
  op_geom.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );
  op_misc.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );


  op_load.get_tree().add_op( "Open image", "imageread" );
  op_load.get_tree().add_op( "Open RAW image", "raw_loader" );

  //op_raw.get_tree().add_op( "RAW developer", "raw_developer" );
  op_load.get_tree().add_op( "RAW developer", "raw_developer" );

  //op_conv.get_tree().add_op( "Color profile conversion", "convert_colorspace" );
  //op_conv.get_tree().add_op( "Lab conversion", "convert2lab" );

  op_color.get_tree().add_op( "Curves", "curves" );
  op_color.get_tree().add_op( "B/C/S/H Adjustment", "hue_saturation" );
  op_color.get_tree().add_op( "Invert", "invert" );
  op_color.get_tree().add_op( "Desaturate", "desaturate" );
  //op_color.get_tree().add_op( "Brightness/Contrast", "brightness_contrast" );
  op_color.get_tree().add_op( "Uniform Fill", "uniform");
  op_color.get_tree().add_op( "Gradient", "gradient");
  op_color.get_tree().add_op( "Channel Mixer", "channel_mixer" );
  op_color.get_tree().add_op( "H/S/L Mask", "hsl_mask" );
  op_color.get_tree().add_op( "Color profile conversion", "convert_colorspace" );
  op_color.get_tree().add_op( "Emulate film [color slide]", "gmic_emulate_film_colorslide" );
  op_color.get_tree().add_op( "Emulate film [B&W]", "gmic_emulate_film_bw" );
  op_color.get_tree().add_op( "Emulate film [instant consumer]", "gmic_emulate_film_instant_consumer" );
  op_color.get_tree().add_op( "Emulate film [instant pro]", "gmic_emulate_film_instant_pro" );
  op_color.get_tree().add_op( "Emulate film [negative color]", "gmic_emulate_film_negative_color" );
  op_color.get_tree().add_op( "Emulate film [negative new]", "gmic_emulate_film_negative_new" );
  op_color.get_tree().add_op( "Emulate film [negative old]", "gmic_emulate_film_negative_old" );
  op_color.get_tree().add_op( "Emulate film [print films]", "gmic_emulate_film_print_films" );
  op_color.get_tree().add_op( "Emulate film [various]", "gmic_emulate_film_various" );

  op_detail.get_tree().add_op( "Gaussian blur", "gaussblur" );
  op_detail.get_tree().add_op( "Noise reduction", "denoise" );
  op_detail.get_tree().add_op( "Sharpen", "sharpen" );
  op_detail.get_tree().add_op( "Volume", "volume" );
  op_detail.get_tree().add_op( "Multi-level decomposition", "gmic_split_details" );

  op_geom.get_tree().add_op( "Crop image", "crop" );
  op_geom.get_tree().add_op( "Scale & rotate image", "scale" );
  op_geom.get_tree().add_op( "Optical corrections (experimental)", "lensfun" );

  //#if !defined(__APPLE__) && !defined(__MACH__)
#ifndef PF_DISABLE_GMIC
  //op_gmic.get_tree().add_op( "G'MIC Interpreter", "gmic" );
  op_gmic.get_tree().add_op( "Dream Smoothing", "gmic_dream_smooth" );
  op_gmic.get_tree().add_op( "Gradient Norm", "gmic_gradient_norm" );
  op_gmic.get_tree().add_op( "Convolve", "gmic_convolve" );
  op_gmic.get_tree().add_op( "Extract Foreground", "gmic_extract_foreground" );
  op_gmic.get_tree().add_op( "Inpaint [patch-based]", "gmic_inpaint" );
  op_gmic.get_tree().add_op( "Despeckle", "gmic_gcd_despeckle" );
  op_gmic.get_tree().add_op( "Iain's Noise Reduction", "gmic_iain_denoise" );
  op_gmic.get_tree().add_op( "Sharpen [richardson-lucy]", "gmic_sharpen_rl" );
  op_gmic.get_tree().add_op( "Smooth [anisotropic]", "gmic_smooth_anisotropic" );
  op_gmic.get_tree().add_op( "Smooth [bilateral]", "gmic_blur_bilateral" );
  op_gmic.get_tree().add_op( "Smooth [diffusion]", "gmic_smooth_diffusion" );
  op_gmic.get_tree().add_op( "Smooth [mean-curvature]", "gmic_smooth_mean_curvature" );
  op_gmic.get_tree().add_op( "Smooth [median]", "gmic_smooth_median" );
  op_gmic.get_tree().add_op( "Smooth [patch-based]", "gmic_denoise" );
  op_gmic.get_tree().add_op( "Smooth [selective gaussian]", "gmic_smooth_selective_gaussian" );
  op_gmic.get_tree().add_op( "Smooth [total variation]", "gmic_smooth_total_variation" );
  op_gmic.get_tree().add_op( "Smooth [wavelets]", "gmic_smooth_wavelets_haar" );
  op_gmic.get_tree().add_op( "Smooth [guided]", "gmic_smooth_guided" );
  op_gmic.get_tree().add_op( "Tone mapping", "gmic_tone_mapping" );
  op_gmic.get_tree().add_op( "Transfer colors [advanced]", "gmic_transfer_colors" );
#endif

  op_misc.get_tree().add_op( "Draw", "draw" );
  op_misc.get_tree().add_op( "Clone stamp", "clone_stamp" );
  op_misc.get_tree().add_op( "Clone layer", "clone" );
  op_misc.get_tree().add_op( "Buffer layer", "buffer" );
  op_misc.get_tree().add_op( "Digital watermark", "gmic_watermark_fourier" );

  get_vbox()->pack_start( notebook );

  show_all_children();
}


PF::OperationsTreeDialog::~OperationsTreeDialog()
{
}


void PF::OperationsTreeDialog::open()
{
  //op_tree.update_model();
  show_all();
}


void PF::OperationsTreeDialog::on_button_clicked(int id)
{
  switch(id) {
  case 0:
    //hide_all();
    hide();
    break;
  case 1:
    add_layer();
    //hide_all();
    hide();
    break;
  }
}


void PF::OperationsTreeDialog::on_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
  add_layer();
  //hide_all();
  hide();
  /*
  Gtk::TreeModel::iterator iter = op_tree.get_model()->get_iter( path );
  if (iter) {
    PF::OperationsTreeColumns& columns = op_tree.get_columns();
    Glib::ustring nickname = (*iter)[columns.col_nickname];
    std::cout<<"Adding layer of type \""<<(*iter)[columns.col_name]<<"\""
	     <<" ("<<(*iter)[columns.col_nickname]<<")"<<std::endl;
    //std::cout<<"Activated row "<<l->get_name()<<std::endl;

    //PF::OperationConfigUI* dialog = l->get_processor()->get_par()->get_config_ui();
    //if(dialog) dialog->open();
  }
   */
}



void PF::OperationsTreeDialog::add_layer()
{
  int page = notebook.get_current_page();
  if( page < 0 ) return;

  PF::OperationsTree* op_tree = NULL;

  switch( page ) {
  case 0:
    op_tree = &(op_load.get_tree());
    break;
    //case 1:
    //  op_tree = &(op_raw.get_tree());
    //  break;
    //case 2:
    //  op_tree = &(op_conv.get_tree());
    //  break;
  case 1:
    op_tree = &(op_color.get_tree());
    break;
  case 2:
    op_tree = &(op_detail.get_tree());
    break;
  case 3:
    op_tree = &(op_geom.get_tree());
    break;
  case 4:
    op_tree = &(op_gmic.get_tree());
    break;
  case 5:
    op_tree = &(op_misc.get_tree());
    break;
  default:
    return;
  }

  if( !op_tree ) return;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = op_tree->get_selection();
  Gtk::TreeStore::iterator iter = refTreeSelection->get_selected();
  if( !iter ) return;

  PF::OperationsTreeColumns& columns = op_tree->get_columns();
  //std::cout<<"Adding layer of type \""<<(*iter)[columns.col_name]<<"\""
  //   <<" ("<<(*iter)[columns.col_nickname]<<")"<<std::endl;

  if( !image ) return;

  PF::LayerManager& layer_manager = image->get_layer_manager();
  PF::Layer* layer = layer_manager.new_layer();
  if( !layer ) return;


  std::string op_type = (*iter)[columns.col_nickname];
  PF::ProcessorBase* processor = 
      PF::PhotoFlow::Instance().new_operation( op_type.c_str(), layer );
  if( !processor || !processor->get_par() ) return;
  PF::OperationConfigUI* ui = dynamic_cast<PF::OperationConfigUI*>( processor->get_par()->get_config_ui() );
  if( processor->get_par()->get_default_name().empty() )
    layer->set_name( _("New Layer") );
  else
    layer->set_name( processor->get_par()->get_default_name() );

  /*
  if( (*iter)[columns.col_nickname] == "imageread" ) { 

    processor = new PF::Processor<PF::ImageReaderPar,PF::ImageReader>();
    layer->set_processor( processor );
    dialog = new PF::ImageReadConfigDialog( layer );

  } else if( (*iter)[columns.col_nickname] == "clone" ) {

    processor = new PF::Processor<PF::ClonePar,PF::CloneProc>();
    layer->set_processor( processor );
    dialog = new PF::CloneConfigDialog( layer );

  } else if( (*iter)[columns.col_nickname] == "invert" ) {

    processor = new PF::Processor<PF::InvertPar,PF::Invert>();
    layer->set_processor( processor );
    dialog = new PF::OperationConfigDialog( layer, "Invert Image" );

  } else if( (*iter)[columns.col_nickname] == "brightness_contrast" ) {

    processor = new PF::Processor<PF::BrightnessContrastPar,PF::BrightnessContrast>();
    layer->set_processor( processor );
    dialog = new PF::BrightnessContrastConfigDialog( layer );

  } else if( (*iter)[columns.col_nickname] == "curves" ) {

    processor = new PF::Processor<PF::CurvesPar,PF::Curves>();
    layer->set_processor( processor );
    dialog = new PF::CurvesConfigDialog( layer );

  } else if( (*iter)[columns.col_nickname] == "convert2lab" ) {

    processor = new PF::Processor<PF::Convert2LabPar,PF::Convert2LabProc>();
    layer->set_processor( processor );
    //dialog = new PF::BrightnessContrastConfigDialog( layer );

  } else { // it must be a VIPS operation...

    PF::Processor<PF::VipsOperationPar,PF::VipsOperationProc>* vips_op = 
      new PF::Processor<PF::VipsOperationPar,PF::VipsOperationProc>();
    Glib::ustring str = (*iter)[columns.col_nickname];
    vips_op->get_par()->set_op( str.c_str() );
    processor = vips_op;
    layer->set_processor( processor );

    PF::VipsOperationConfigDialog* vips_config = 
      new PF::VipsOperationConfigDialog( layer );
    vips_config->set_op( str.c_str() );
    dialog = vips_config;
  }
   */

  if( processor ) {
    layer_widget->add_layer( layer );
    //layer_manager.get_layers().push_back( layer );
    //layer_manager.modified();
    if( ui ) {
      PF::OperationConfigDialog* dialog = dynamic_cast<PF::OperationConfigDialog*>( ui );
      if(dialog) {
        if( dialog ) {
          //processor->get_par()->set_config_ui( dialog );
          //dialog->update();
          dialog->open();
          dialog->enable_editing();
        }
      }
    }
  }
}
