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
#include <gtkmm.h>

#include "../../base/exif_data.hh"
#include "../../operations/lensfun.hh"

#include "lensfun_config.hh"


PF::LensFunConfigGUI::LensFunConfigGUI( PF::Layer* layer ):
  OperationConfigGUI( layer, "Optical corrections" ),
  auto_matching_checkbox( this, "auto_matching", _("auto matching"), true ),
  auto_crop_checkbox( this, "auto_crop", _("auto cropping"), true ),
  lf_selector( this, "camera_maker", "camera_model", "lens" ),
  enable_distortion_button( this, "enable_distortion", _("distortion"), false ),
  enable_tca_button( this, "enable_tca", _("chromatic aberrations"), false ),
  enable_vignetting_button( this, "enable_vignetting", _("vignetting"), false ),
  enable_all_button( this, "enable_all", _("all corrections"), false )
{
  controlsBox.pack_start( lf_selector );
  
  hbox.pack_start( auto_matching_checkbox );
  hbox.pack_start( enable_all_button );
  controlsBox.pack_start( hbox );
  controlsBox.pack_start( auto_crop_checkbox );

  controlsBox.pack_start( enable_vignetting_button );
  controlsBox.pack_start( enable_distortion_button );
  controlsBox.pack_start( enable_tca_button );

  add_widget( controlsBox );
}


void PF::LensFunConfigGUI::do_update()
{
  std::cout<<"LensFunConfigGUI::do_update() called"<<std::endl;
  if( !(get_layer()) || !(get_layer()->get_image()) ||
      !(get_layer()->get_processor()) ||
      !(get_layer()->get_processor()->get_par()) ) {
    OperationConfigGUI::do_update();
    return;
  }

  PF::LensFunPar* par =
      dynamic_cast<PF::LensFunPar*>(get_layer()->get_processor()->get_par());
  if( !par ) return;

  PF::Image* image = get_layer()->get_image();
  PF::Pipeline* pipeline = image->get_pipeline(0);
  PF::PipelineNode* node = NULL;
  PF::PipelineNode* inode = NULL;
  PF::ProcessorBase* processor = NULL;
  std::string maker, model;
  if( pipeline ) node = pipeline->get_node( get_layer()->get_id() );
  if( node ) inode = pipeline->get_node( node->input_id );
  if( node ) processor = node->processor;

  PF::exif_data_t* exif_data = NULL;
  if( inode && inode->image) {
    size_t blobsz;
    if( PF_VIPS_IMAGE_GET_BLOB( inode->image, PF_META_EXIF_NAME, &exif_data, &blobsz ) ||
        blobsz != sizeof(PF::exif_data_t) ) {
      exif_data = NULL;
    }
  }

    //if( custom_cam_maker.empty() && custom_cam_model.empty() && custom_lens_model.empty() ) {
    custom_cam_maker = par->camera_maker();
    custom_cam_model = par->camera_model();
    custom_lens_model = par->lens();
    std::cout<<"RawDeveloperConfigGUI::do_update(): camera=\""<<custom_cam_maker<<"\", \""<<custom_cam_model<<"\""<<std::endl;
    std::cout<<"RawDeveloperConfigGUI::do_update(): lens=\""<<custom_lens_model<<"\""<<std::endl;
  //}
  if( auto_matching_checkbox.get_active() ) {
    lf_selector.disable();
    if( exif_data ) {
      Glib::ustring cam_make, cam_model, lens_model;
      cam_make = exif_data->exif_maker;
      cam_model = exif_data->exif_model;
      lens_model = exif_data->exif_lens;
      lf_selector.set_cam( cam_make, cam_model );
      lf_selector.set_lens( lens_model );
    }
  } else {
    if( custom_cam_maker.empty() && custom_cam_model.empty() && custom_lens_model.empty() ) {
      if( exif_data ) {
        Glib::ustring cam_make, cam_model, lens_model;
        cam_make = exif_data->exif_maker;
        cam_model = exif_data->exif_model;
        lens_model = exif_data->exif_lens;
        lf_selector.set_cam( cam_make, cam_model );
        lf_selector.set_lens( lens_model );
        lf_selector.set_value();
      }
    } else {
      lf_selector.get_value();
    }
    lf_selector.enable();
  }

  OperationConfigGUI::do_update();
}
