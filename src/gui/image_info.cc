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
  update->info->set_text( *(update->text) );
  delete(update->text);
  g_free (update);
  return FALSE;
}



PF::ImageInfo::ImageInfo( Pipeline* v ):
      PipelineSink( v ), Gtk::Frame()
{
  add(textview);
  textview.property_pixels_above_lines().set_value(3);
  textview.property_pixels_below_lines().set_value(3);
  textview.property_left_margin().set_value(5);

  show_all();
}

PF::ImageInfo::~ImageInfo ()
{
}


void PF::ImageInfo::set_text(const Glib::ustring& text)
{
  Glib::RefPtr< Gtk::TextBuffer >  buf = textview.get_buffer ();
  buf->set_text( text );
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

  std::ostringstream info_text;
  info_text << exif_data->camera_makermodel << std::endl;
  info_text << exif_data->exif_lens << " (at " << exif_data->exif_focal_length << "mm)" << std::endl;
  info_text << "f/" << exif_data->exif_aperture << "  ";
  if( exif_data->exif_exposure >= 1 )
    info_text << exif_data->exif_exposure << "s  ";
  else
    info_text << "1/" << 1.f / exif_data->exif_exposure << "s  ";
  info_text << "ISO" << exif_data->exif_iso;

  Update * update = g_new (Update, 1);
  update->info = this;
  update->text = new Glib::ustring(info_text.str());
  gdk_threads_add_idle ((GSourceFunc) queue_draw_cb, update);
}

