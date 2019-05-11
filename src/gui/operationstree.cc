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
      buf->insert_at_cursor("The tool provides a separate curve for each image channel. In the case of RGB images, "));
      buf->insert_at_cursor("an additional curve, called \"RGB\", allows to modify all three channels at the same time.\n\n");
      buf->insert_at_cursor("In the graphs, the horizontal axis represents the input values and the vertical axis the output ones.");
      buf->insert_at_cursor("The curve is initially a simple diagonal line, meaning that the output exactly matches input.\n\n");
      buf->insert_at_cursor("Left-clicking on the graph with the mouse adds a new control point, while right-clicking on an existing ");
      buf->insert_at_cursor("control point deletes it. Control points can be moved by either dragging them with the mouse, or by setting the corresponding input and output numerical values in the boxes below the graph.");
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
  Glib::ustring helpPath = Glib::ustring(PF::PhotoFlow::Instance().get_data_dir()) + "/help/en/" + nik + ".hlp";
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
    help = _("This help is not yet available. Sorry.");
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
  op_load.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );

  //notebook.append_page( op_raw, "raw" );

  //op_conv_box.add( op_conv );
  //notebook.append_page( op_conv, "conv" );

  //op_color_box.add( op_color );
  notebook.append_page( op_color, "color" );
  op_color.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );

#ifdef HAVE_OCIO
  notebook.append_page( op_ocio, "OCIO" );
  op_ocio.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );
  op_ocio.get_tree().add_op( _("OCIO Transform"), "ocio_transform" );
  op_ocio.get_tree().add_op( _("OCIO - Filmic"), "ocio_filmic" );
  op_ocio.get_tree().add_op( _("OCIO - ACES"), "ocio_aces" );
#endif

  //op_detail_box.add( op_detail );
  notebook.append_page( op_detail, "detail" );
  op_detail.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );

  //op_geom_box.add( op_geom );
  notebook.append_page( op_geom, "geom" );
  op_geom.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );

  //op_geom_box.add( op_geom );
  notebook.append_page( op_mask, "mask" );

  //#ifndef PF_DISABLE_GMIC
  //op_gmic_box.add( op_gmic );
  notebook.append_page( op_gmic, "G'MIC" );
  //#endif

  //op_misc_box.add( op_misc );
  notebook.append_page( op_misc, "misc" );
  op_misc.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );

  //op_raw.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );
  //op_conv.get_tree().signal_row_activated().connect( sigc::mem_fun(*this, &PF::OperationsTreeDialog::on_row_activated) );


  op_load.get_tree().add_op( _("Open image"), "imageread" );
  op_load.get_tree().add_op( _("Open RAW image"), "raw_loader" );
  op_load.get_tree().add_op( _("RAW developer"), "raw_developer" );

  //op_conv.get_tree().add_op( "Color profile conversion"), "convert_colorspace" );
  //op_conv.get_tree().add_op( "Lab conversion"), "convert2lab" );

  op_color.get_tree().add_op( _("Color profile conversion"), "convert_colorspace" );
  op_color.get_tree().add_op( _("White Balance"), "white_balance" );
  //op_color.get_tree().add_op( _("Levels"), "levels" );
  op_color.get_tree().add_op( _("Basic Adjustments"), "basic_adjustments" );
  op_color.get_tree().add_op( _("Color Adjustments"), "color_correction" );
  op_color.get_tree().add_op( _("Desaturate"), "desaturate" );
  op_color.get_tree().add_op( _("Curves"), "curves" );
  op_color.get_tree().add_op( _("Shadows/Highlights"), "shadows_highlights_v2" );
  op_color.get_tree().add_op( _("Tone mapping"), "tone_mapping_v2" );
  op_color.get_tree().add_op( _("Dynamic range compressor"), "dynamic_range_compressor_v2" );
  op_color.get_tree().add_op( _("Shadows/Highlights old"), "shadows_highlights" );
  op_color.get_tree().add_op( _("Tone mapping old"), "tone_mapping" );
  op_color.get_tree().add_op( _("Channel Mixer"), "channel_mixer" );
  //op_color.get_tree().add_op( "Brightness/Contrast"), "brightness_contrast" );
  op_color.get_tree().add_op( _("Noise"), "noise_generator" );
  op_color.get_tree().add_op( _("Clip values"), "clip" );
  op_color.get_tree().add_op( _("Apply LUT"), "gmic_emulate_film_user_defined" );
  op_color.get_tree().add_op( _("Emulate film [color slide]"), "gmic_emulate_film_colorslide" );
  op_color.get_tree().add_op( _("Emulate film [B&W]"), "gmic_emulate_film_bw" );
  op_color.get_tree().add_op( _("Emulate film [instant consumer]"), "gmic_emulate_film_instant_consumer" );
  op_color.get_tree().add_op( _("Emulate film [instant pro]"), "gmic_emulate_film_instant_pro" );
  op_color.get_tree().add_op( _("Emulate film [negative color]"), "gmic_emulate_film_negative_color" );
  op_color.get_tree().add_op( _("Emulate film [negative new]"), "gmic_emulate_film_negative_new" );
  op_color.get_tree().add_op( _("Emulate film [negative old]"), "gmic_emulate_film_negative_old" );
  op_color.get_tree().add_op( _("Emulate film [print films]"), "gmic_emulate_film_print_films" );
  op_color.get_tree().add_op( _("Emulate film [various]"), "gmic_emulate_film_various" );

  op_detail.get_tree().add_op( _("Sharpen"), "sharpen" );
  op_detail.get_tree().add_op( _("Gaussian blur"), "gaussblur" );
  op_detail.get_tree().add_op( _("Guided filter"), "guided_filter" );
  op_detail.get_tree().add_op( _("Bilateral blur"), "blur_bilateral" );
  op_detail.get_tree().add_op( _("Median filter"), "median_filter" );
  op_detail.get_tree().add_op( _("Local contrast"), "local_contrast" );
  //op_detail.get_tree().add_op( _("CLAHE"), "clahe" );
  op_detail.get_tree().add_op( _("Gradient Norm"), "gmic_gradient_norm" );
  op_detail.get_tree().add_op( _("Split Details"), "split_details" );
  op_detail.get_tree().add_op( _("Defringe"), "defringe" );
  //op_detail.get_tree().add_op( _("Multi-level decomposition"), "gmic_split_details" );
  op_detail.get_tree().add_op( _("Noise reduction"), "denoise" );

  op_geom.get_tree().add_op( _("Crop image"), "crop" );
  op_geom.get_tree().add_op( _("Scale & rotate image"), "scale" );
  op_geom.get_tree().add_op( _("Perspective correction"), "perspective" );
//#if !defined(__MINGW32__) && !defined(__MINGW64__)
  op_geom.get_tree().add_op( _("Optical corrections"), "lensfun" );
//#endif

  //#if !defined(__APPLE__) && !defined(__MACH__)
#ifndef PF_DISABLE_GMIC
  //op_gmic.get_tree().add_op( "G'MIC Interpreter"), "gmic" );
  op_gmic.get_tree().add_op( _("Dream Smoothing"), "gmic_dream_smooth" );
  op_gmic.get_tree().add_op( _("Gradient Norm"), "gmic_gradient_norm" );
  //too generic op_gmic.get_tree().add_op( _("Convolve"), "gmic_convolve" );
  //crashes op_gmic.get_tree().add_op( _("Extract Foreground"), "gmic_extract_foreground" );
  //slow op_gmic.get_tree().add_op( _("Inpaint [patch-based]"), "gmic_inpaint" );
  //RT algorithm is better? op_gmic.get_tree().add_op( _("Despeckle"), "gmic_gcd_despeckle" );
  //crashes? op_gmic.get_tree().add_op( _("Iain's Noise Reduction"), "gmic_iain_denoise" );
  op_gmic.get_tree().add_op( _("Sharpen [richardson-lucy]"), "gmic_sharpen_rl" );
  //op_gmic.get_tree().add_op( _("Denoise"), "gmic_denoise" );
  //op_gmic.get_tree().add_op( _("Smooth [non-local means]"), "gmic_smooth_nlmeans" );
  op_gmic.get_tree().add_op( _("Smooth [anisotropic]"), "gmic_smooth_anisotropic" );
  op_gmic.get_tree().add_op( _("Smooth [bilateral]"), "gmic_blur_bilateral" );
  //op_gmic.get_tree().add_op( _("Smooth [diffusion]"), "gmic_smooth_diffusion" );
  //op_gmic.get_tree().add_op( _("Smooth [mean-curvature]"), "gmic_smooth_mean_curvature" );
  //op_gmic.get_tree().add_op( _("Smooth [median]"), "gmic_smooth_median" );
  //op_gmic.get_tree().add_op( _("Smooth [patch-based]"), "gmic_denoise" );
  //op_gmic.get_tree().add_op( _("Smooth [selective gaussian]"), "gmic_smooth_selective_gaussian" );
  //op_gmic.get_tree().add_op( _("Smooth [total variation]"), "gmic_smooth_total_variation" );
  //op_gmic.get_tree().add_op( _("Smooth [wavelets]"), "gmic_smooth_wavelets_haar" );
  //op_gmic.get_tree().add_op( _("Smooth [guided]"), "gmic_smooth_guided" );
  //op_gmic.get_tree().add_op( _("Tone mapping"), "gmic_tone_mapping" );
  op_gmic.get_tree().add_op( _("Transfer colors [advanced]"), "gmic_transfer_colors" );
#endif


  op_mask.get_tree().add_op( _("Uniform Fill"), "uniform");
  op_mask.get_tree().add_op( _("Invert"), "invert" );
  op_mask.get_tree().add_op( _("Threshold"), "threshold" );
  op_mask.get_tree().add_op( _("Curves"), "curves" );
  op_mask.get_tree().add_op( _("Gradient"), "gradient");
  op_mask.get_tree().add_op( _("Path"), "path_mask");
  op_mask.get_tree().add_op( _("H/S/L Mask"), "hsl_mask" );
  op_mask.get_tree().add_op( _("Gaussian blur"), "gaussblur" );
  //#if !defined(__APPLE__) && !defined(__MACH__)
#ifndef PF_DISABLE_GMIC
  op_gmic.get_tree().add_op( _("G'MIC Interpreter"), "gmic" );
  op_mask.get_tree().add_op( _("Gradient Norm"), "gmic_gradient_norm" );
#endif
  op_mask.get_tree().add_op( _("Draw"), "draw" );
  op_mask.get_tree().add_op( _("Clone layer"), "clone" );



  op_misc.get_tree().add_op( _("Draw"), "draw" );
  //op_misc.get_tree().add_op( _("Clone stamp"), "clone_stamp" );
  op_misc.get_tree().add_op( _("Clone layer"), "clone" );
  op_misc.get_tree().add_op( _("Buffer layer"), "buffer" );
  op_misc.get_tree().add_op( _("Digital watermark"), "gmic_watermark_fourier" );

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
  std::cout<<"OperationsTreeDialog::on_button_clicked: id="<<id<<std::endl;
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

  std::cout<<"OperationsTreeDialog::add_layer(): page="<<page<<std::endl;

  Gtk::Widget* w = notebook.get_nth_page(page);
  PF::OperationsTreeWidget* tw = dynamic_cast<PF::OperationsTreeWidget*>(w);
  std::cout<<"OperationsTreeDialog::add_layer(): w="<<w<<"  tw="<<tw<<std::endl;

  if( tw ) {
    op_tree = &(tw->get_tree());
  }
  /*
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
    op_tree = &(op_mask.get_tree());
    break;
  case 5:
    op_tree = &(op_gmic.get_tree());
    break;
  case 6:
    op_tree = &(op_misc.get_tree());
    break;
  default:
    return;
  }
  */
  std::cout<<"OperationsTreeDialog::add_layer(): op_tree="<<op_tree<<std::endl;
  if( !op_tree ) return;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = op_tree->get_selection();
  Gtk::TreeStore::iterator iter = refTreeSelection->get_selected();
  if( !iter ) return;

  PF::OperationsTreeColumns& columns = op_tree->get_columns();

  std::cout<<"OperationsTreeDialog::add_layer(): image="<<image<<std::endl;
  if( !image ) return;

  PF::LayerManager& layer_manager = image->get_layer_manager();
  PF::Layer* layer = layer_manager.new_layer();
  std::cout<<"OperationsTreeDialog::add_layer(): layer="<<layer<<std::endl;
  if( !layer ) return;


  std::string op_type = (*iter)[columns.col_nickname];
  std::cout<<"Adding layer of type \""<<(*iter)[columns.col_name]<<"\""
     <<" ("<<op_type<<")"<<std::endl;

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
      PF::OperationConfigGUI* dialog = dynamic_cast<PF::OperationConfigGUI*>( ui );
      if(dialog) {
        if( dialog ) {
          //processor->get_par()->set_config_ui( dialog );
          //dialog->update();
          dialog->open();
          //dialog->enable_editing();
        }
      }
    }
  }
}
