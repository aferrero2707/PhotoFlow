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
  OperationConfigGUI( layer, "Optical corrections" )
{
  label1.set_text( "Camera maker: " );
  hbox1.pack_start( label1 );
  hbox1.pack_start( makerEntry );

  label2.set_text( "Camera model: " );
  hbox2.pack_start( label2 );
  hbox2.pack_start( modelEntry );
  label3.set_text( "Lens model: " );
  hbox3.pack_start( label3 );
  hbox3.pack_start( lensEntry );

  controlsBox.pack_start( hbox1 );
  controlsBox.pack_start( hbox2 );
  controlsBox.pack_start( hbox3 );
  
  makerEntry.set_editable( false );
  modelEntry.set_editable( false );
  lensEntry.set_editable( false );

  add_widget( controlsBox );

#ifdef PF_HAS_LENSFUN
  ldb = lf_db_new();
#if (BUNDLED_LENSFUN == 1)
  Glib::ustring lfdb = PF::PhotoFlow::Instance().get_lensfun_db_dir();
  ldb->LoadDirectory( lfdb.c_str() );
  std::cout<<"LensFun database loaded from "<<lfdb<<std::endl;
#else
  //char* lfdb_env = getenv("PF_LENSFUN_DATA_DIR");
  //if( lfdb_env ) {
  //  ldb->LoadDirectory( lfdb.c_str() );
  //  std::cout<<"LensFun database loaded from "<<lfdb_env<<std::endl;
  //} else {
    ldb->Load ();
    std::cout<<"LensFun database loaded from default location"<<std::endl;
  //}
#endif //(BUNDLED_LENSFUN == 1)
#endif

  makerEntry.signal_activate().
    connect(sigc::mem_fun(*this,
			  &LensFunConfigGUI::on_maker_changed));
}


void PF::LensFunConfigGUI::do_update()
{
  std::cout<<"LensFunConfigGUI::do_update() called"<<std::endl;
  if( get_layer() && get_layer()->get_image() &&
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {

    // Retrieve the image the layer belongs to
    PF::Image* image = get_layer()->get_image();

    // Retrieve the pipeline #0 (full resolution preview)
    PF::Pipeline* pipeline = image->get_pipeline( 0 );
    if( pipeline ) {

      // Find the pipeline node associated to the current layer
      PF::PipelineNode* node = pipeline->get_node( get_layer()->get_id() );
      if( node && node->image ) {

        size_t blobsz;
        PF::exif_data_t* exif_data;
        if( !vips_image_get_blob( node->image, PF_META_EXIF_NAME,
            (void**)&exif_data, &blobsz ) ) {
          if( blobsz == sizeof(PF::exif_data_t) ) {

#ifdef PF_HAS_LENSFUN
            const lfCamera** cameras = ldb->FindCameras( exif_data->exif_maker,
                exif_data->exif_model );
            if( !cameras ) {
              g_print ("Cannot find the camera `%s %s' in database\n",
                  exif_data->exif_maker, exif_data->exif_model);
            } else {
              g_print("Camera `%s %s' found in database\n",
                  exif_data->exif_maker, exif_data->exif_model);
              const lfCamera *camera = cameras[0];
              lf_free (cameras);

              makerEntry.set_text( camera->Maker );
              modelEntry.set_text( camera->Model );

              const lfLens **lenses = ldb->FindLenses (camera, NULL, exif_data->exif_lens);
              if (!lenses) {
                g_print ("Cannot find the lens `%s' in database\n", exif_data->exif_lens);
              } else {
                const lfLens *lens = lenses[0];
                lf_free (lenses);

                lensEntry.set_text( lens->Model );
              }
            }
#endif
          }
        }
      }
    }
  }
  OperationConfigGUI::do_update();
}


void PF::LensFunConfigGUI::on_maker_changed()
  {
  if( get_layer() && get_layer()->get_image() && 
      get_layer()->get_processor() &&
      get_layer()->get_processor()->get_par() ) {
    std::string maker = makerEntry.get_text();
    std::cout<<"New image file name: "<<maker<<std::endl;
    PF::LensFunPar* par =
      dynamic_cast<PF::LensFunPar*>(get_layer()->get_processor()->get_par());
    if( par && !maker.empty() ) {
      //par->set_file_name( maker );
      //get_layer()->set_dirty( true );
      //std::cout<<"  updating image"<<std::endl;
      //get_layer()->get_image()->update();
    }
  }
}
