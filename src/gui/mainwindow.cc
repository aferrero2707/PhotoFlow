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

#include "mainwindow.hh"
#include <iostream>

#include "layertree.hh"
#include "../base/pf_file_loader.hh"


bool getFileExtension(const char * dir_separator, const std::string & file, std::string & ext)
{
    std::size_t ext_pos = file.rfind(".");
    std::size_t dir_pos = file.rfind(dir_separator);

    if(ext_pos>dir_pos+1)
    {
        ext.append(file.begin()+ext_pos+1,file.end());
        return true;
    }

    return false;
}


namespace PF {

  MainWindow::MainWindow(char* filename):
#ifdef GTKMM_2
    mainBox(),
    editorBox(),
    viewBox(),
    controlBox(),
#endif
#ifdef GTKMM_3
    mainBox(Gtk::ORIENTATION_HORIZONTAL),
    viewBox(Gtk::ORIENTATION_VERTICAL),
    controlBox(Gtk::ORIENTATION_VERTICAL),
#endif
    buttonOpen("Open"), buttonSave("Save"), buttonExit("Exit")
{
  set_title("Photo Flow");
  // Sets the border width of the window.
  set_border_width(0);
  //set_default_size(120,80);
  set_default_size(800,600);
  
  add(mainBox);

  mainBox.pack_start(topButtonBox, Gtk::PACK_SHRINK);
  mainBox.pack_start(editorBox);

  editorBox.pack1(viewBox);
  editorBox.pack2(controlBox,false,true);

  viewBox.pack_start(topButtonBox, Gtk::PACK_SHRINK);
  viewBox.pack_start(viewerNotebook);

  //VImage* image = new VImage("../testimages/lena512color.jpg");
  //imageArea.set_image("../testimages/lena512color-bis.jpg");
  //imageArea.set_image( filename );
  set_image( filename );

  viewerNotebook.append_page(imageArea_scrolledWindow,"Image #1");
  imageArea_scrolledWindow.add(imageArea);

  topButtonBox.pack_start(buttonOpen, Gtk::PACK_SHRINK);
  topButtonBox.pack_start(buttonSave, Gtk::PACK_SHRINK);
  topButtonBox.pack_start(buttonExit, Gtk::PACK_SHRINK);
  topButtonBox.set_border_width(5);
  topButtonBox.set_layout(Gtk::BUTTONBOX_START);

  buttonSave.signal_clicked().connect( sigc::mem_fun(*this,
						     &MainWindow::on_button_save_clicked) );

  buttonExit.signal_clicked().connect( sigc::mem_fun(*this,
						     &MainWindow::on_button_exit) );


  //imageArea.signal_configure_event().connect(sigc::mem_fun(imageArea,
  //									  &ImageArea::on_resize));
  
  //treeFrame.set_border_width(10);
  //treeFrame.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);


  controlBox.pack_start(layersWidget);

  //layersWidget.set_layer_manager( imageArea.get_layer_manager() );
  //layersWidget.set_image( imageArea.get_image() );

  /*
  controlBox.pack_start(treeNotebook);

  treeNotebook.set_tab_pos(Gtk::POS_LEFT); 

  treeFrame.add(layerTree);


  layerTree.set_reorderable();
  LayerTree* layertree = new LayerTree( imageArea.get_layer_manager() );
  layerTree.set_model(layertree->get_model());

  layerTree.append_column_editable("V", layertree->get_columns().col_visible);
  layerTree.append_column_editable("Name", layertree->get_columns().col_name);

  treeNotebook.append_page(treeFrame,"Layers");
  Widget* page = treeNotebook.get_nth_page(-1);
  Gtk::Label* label = (Gtk::Label*)treeNotebook.get_tab_label(*page);
  label->set_angle(90);
  */
  

  show_all_children();


}

  MainWindow::~MainWindow()
  {
    std::cout<<"~MainWindow(): deleting image"<<std::endl;
    delete pf_image;
  }

  void MainWindow::on_button_clicked()
  {
    std::cout << "Hello World" << std::endl;
  }

  void MainWindow::on_button_exit()
  {
    hide();
  }


  #define LOAD_PFI

  void
  MainWindow::set_image( std::string filename )
  {
    std::vector<VipsImage*> in;

    pf_image = new PF::Image();

    std::string ext;
    if( getFileExtension( "/", filename, ext ) &&
	ext == "pfi" ) {

      PF::load_pf_image( filename, pf_image );
      PF::PhotoFlow::Instance().set_image( pf_image );
      layersWidget.set_image( pf_image );
      pf_image->add_view( VIPS_FORMAT_UCHAR, 0 );

    } else {

      PF::PhotoFlow::Instance().set_image( pf_image );
      layersWidget.set_image( pf_image );

      pf_image->add_view( VIPS_FORMAT_UCHAR, 0 );

      PF::LayerManager& layer_manager = pf_image->get_layer_manager();

      PF::Processor<PF::ImageReaderPar,PF::ImageReader>* imgread = 
	new PF::Processor<PF::ImageReaderPar,PF::ImageReader>();
      imgread->get_par()->set_file_name( filename );

      PF::Layer* limg = layer_manager.new_layer();
      limg->set_processor( imgread );
      limg->set_name( "input image" );

      PF::ImageReadConfigDialog* img_config = 
	new PF::ImageReadConfigDialog( limg );
      imgread->get_par()->set_config_ui( img_config );
      //img_config->set_layer( limg );
      //img_config->set_image( pf_image );

      //layer_manager.get_layers().push_back( limg );
      layersWidget.add_layer( limg );

    }

    imageArea.set_view( pf_image->get_view(0) );
    pf_image->signal_modified.connect( sigc::mem_fun(&imageArea, &ImageArea::update_image) );
    pf_image->update();



    /*
    //PF::ProcessorBase* invert = 
    //  new PF::Processor<PF::Invert,PF::InvertPar>();

    PF::Processor<PF::BrightnessContrast,PF::BrightnessContrastPar>* bc = 
      new PF::Processor<PF::BrightnessContrast,PF::BrightnessContrastPar>();
    bc->get_par()->set_brightness(0.2);
    //bc->get_par()->set_contrast(0.5);
    bc->get_par()->set_opacity(0.5);
    bc->get_par()->set_blend_mode(PF::PF_BLEND_NORMAL);

    PF::Processor<PF::Gradient,PF::GradientPar>* gradient = 
      new PF::Processor<PF::Gradient,PF::GradientPar>();

    PF::Processor<PF::VipsOperationProc,PF::VipsOperationPar>* vips_op = 
      new PF::Processor<PF::VipsOperationProc,PF::VipsOperationPar>();
    vips_op->get_par()->set_op( "gamma" );

    PF::Layer* lbc = layer_manager.new_layer();
    lbc->set_processor( bc );
    lbc->set_name( "brightness/contrast" );

    PF::BrightnessContrastConfigDialog* bc_config = 
      new PF::BrightnessContrastConfigDialog();
    bc->get_par()->set_config_ui( bc_config );
    bc_config->set_layer( lbc );
    //bc_config->set_image( pf_image );

    PF::Layer* lgrad = layer_manager.new_layer();
    lgrad->set_processor( gradient );
    lgrad->set_name( "vertical gradient" );

    PF::Layer* linv1 = layer_manager.new_layer();
    linv1->set_processor( new PF::Processor<PF::Invert,PF::InvertPar>() );
    linv1->set_name( "invert 1" );

    PF::Layer* lvips = layer_manager.new_layer();
    lvips->set_processor( vips_op );
    lvips->set_name( "VIPS gamma adjustment" );

    PF::VipsOperationConfigDialog* vips_config = 
      new PF::VipsOperationConfigDialog();
    vips_op->get_par()->set_config_ui( vips_config );
    vips_config->set_layer( lvips );
    //vips_config->set_image( pf_image );
    vips_config->set_op( "gamma" );
    */


    //lbc->imap_insert( lgrad );
    //layer_manager.get_layers().push_back( lbc );
    //layer_manager.get_layers().push_back( linv1 );
    //layer_manager.get_layers().push_back( lvips );


    //layer_manager->build_chain( PF::PF_COLORSPACE_RGB, VIPS_FORMAT_UCHAR, 100,100 );

    //layer_manager->rebuild_all( PF::PF_COLORSPACE_RGB, VIPS_FORMAT_UCHAR, 100,100 );
    //update_image();
    /*
    display_image = im_open( "display_image", "p" );

    VipsImage* out = layer_manager->get_output();
    if(out) {
      if (vips_sink_screen (out, display_image, NULL,
          64, 64, (2000/64)*(2000/64), 0, sink_notify, this))
  verror ();
      
      
      region = vips_region_new (display_image);
      std::cout<<"Image size: "<<display_image->Xsize<<","<<display_image->Ysize<<std::endl;
      set_size_request (display_image->Xsize, display_image->Ysize);
    }
    */
  }


  void PF::MainWindow::on_button_save_clicked()
  {
    Gtk::FileChooserDialog dialog("Save image as...",
				  Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);
  
    //Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

    //Show the dialog and wait for a user response:
    int result = dialog.run();

    //Handle the response:
    switch(result) {
    case(Gtk::RESPONSE_OK): 
      {
	std::cout << "Save clicked." << std::endl;

	//Notice that this is a std::string, not a Glib::ustring.
	std::string filename = dialog.get_filename();
	std::cout << "File selected: " <<  filename << std::endl;
	pf_image->save( filename );
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


}
