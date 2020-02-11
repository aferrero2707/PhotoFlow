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
#include "../base/exif_data.hh"
#include "image_info.hh"




gboolean PF::ImageInfo::queue_draw_cb (PF::ImageInfo::Update * update)
{
  //update->sampler->set_values(update->val, update->lch, update->type);
  //std::cout<<"update->histogram->queue_draw() called"<<std::endl;
  update->info->set_text( *(update->text1), *(update->text2), *(update->text3) );
  delete(update->text1);
  delete(update->text2);
  delete(update->text3);
  g_free (update);
  return FALSE;
}



PF::ImageInfo::ImageInfo( Pipeline* v ):
      PipelineSink( v ), Gtk::Frame()
{
  /*
  add(textview);
  textview.property_pixels_above_lines().set_value(3);
  textview.property_pixels_below_lines().set_value(3);
  textview.property_left_margin().set_value(5);
  textview.set_editable(false);
  */
  add(vbox);
  a1.set(Gtk::ALIGN_START, Gtk::ALIGN_CENTER, 0.0, 1.0); a1.add(l1);
  a2.set(Gtk::ALIGN_START, Gtk::ALIGN_CENTER, 0.0, 1.0); a2.add(l2);
  a3.set(Gtk::ALIGN_START, Gtk::ALIGN_CENTER, 0.0, 1.0); a3.add(l3);
  vbox.set_spacing(0);
  vbox.set_border_width(10);
  vbox.pack_start( a1, Gtk::PACK_SHRINK, 0 );
  vbox.pack_start( a2, Gtk::PACK_SHRINK, 0 );
  vbox.pack_start( a3, Gtk::PACK_SHRINK, 0 );
  show_all();
}

PF::ImageInfo::~ImageInfo ()
{
}


void PF::ImageInfo::set_text(const Glib::ustring& text1, const Glib::ustring& text2, const Glib::ustring& text3)
{
  Glib::RefPtr< Gtk::TextBuffer >  buf = textview.get_buffer ();
  buf->set_text( text1 );

  l1.set_text(text1);
  l2.set_text(text2);
  l3.set_text(text3);
}


void PF::ImageInfo::update( VipsRect* area )
{
  //PF::Pipeline* pipeline = pf_image->get_pipeline(0);

#ifndef NDEBUG
  std::cout<<"PF::ImageInfo::update(): called"<<std::endl;
#endif
  if( !get_pipeline() ) {
    std::cout<<"ImageInfo::update(): error: NULL pipeline"<<std::endl;
    return;
  }
  if( !get_pipeline()->get_output() ) {
    std::cout<<"ImageInfo::update(): error: NULL image"<<std::endl;
    return;
  }

  VipsImage* image = get_pipeline()->get_output();
  if( !image ) return;

  PF::exif_data_t* exif_data = PF::get_exif_data( image );
  if( !exif_data ) return;

  std::ostringstream info_text1;
  if( exif_data->camera_makermodel[0] != '\0' )
    info_text1 << exif_data->camera_makermodel;
  else
    info_text1 << _("UNKNOWN CAMERA");
  info_text1 << std::endl;
  std::ostringstream info_text2;
  if( exif_data->exif_lens[0] != '\0' )
    info_text2 << exif_data->exif_lens;
  else
    info_text2 << _("UNKNOWN LENS");
  if( exif_data->exif_focal_length > 0 )
    info_text2 << " (at " << exif_data->exif_focal_length << "mm)";
  info_text2 << std::endl;
  std::ostringstream info_text3;
  if( exif_data->exif_aperture > 0 )
    info_text3 << "f/" << exif_data->exif_aperture << "  ";
  else
    info_text3 << "f/--  ";
  if( exif_data->exif_exposure >= 1 )
    info_text3 << exif_data->exif_exposure << "s  ";
  else if( exif_data->exif_exposure > 0 )
    info_text3 << "1/" << 1.f / exif_data->exif_exposure << "s  ";
  else
    info_text3 << "--s  ";
  if( exif_data->exif_iso > 0 )
    info_text3 << "ISO" << exif_data->exif_iso;
  else
    info_text3 << "ISO--";

  Update * update = g_new (Update, 1);
  update->info = this;
  update->text1 = new Glib::ustring(info_text1.str());
  update->text2 = new Glib::ustring(info_text2.str());
  update->text3 = new Glib::ustring(info_text3.str());
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
}

