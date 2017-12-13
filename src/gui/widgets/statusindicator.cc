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


#include <vips/vips.h>
#include "statusindicator.hh"


static gboolean pf_status_indicator_update ( PF::StatusIndicatorWidget * indicator)
{
  indicator->update();

  return FALSE;
}




#ifdef GTKMM_2
bool PF::StatusIndicatorLed::on_expose_event(GdkEventExpose* event)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if( !window )
    return true;

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();

  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

#endif
#ifdef GTKMM_3
bool PF::StatusIndicatorLed::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
#endif

  cr->save();
  switch( status ) {
  case 0: // ready
    cr->set_source_rgba(0., 1., 0., 1.0);
    break;
  case 1: // caching
    cr->set_source_rgba(1., 0.5, 0., 1.0);
    break;
  case 2: // processing
    cr->set_source_rgba(1., 0., 0., 1.0);
    break;
  case 3: // updating
    cr->set_source_rgba(0., 0., 1., 1.0);
    break;
  }
  cr->paint();
  cr->restore();

  return true;
}



PF::StatusIndicatorWidget::StatusIndicatorWidget(): cur_status(0)
{
  led.set_size_request(10,10);
  pack_start( label, Gtk::PACK_SHRINK, 0 );

  led_alignment.add( led );
  led_alignment.set( 0, 0.5, 0, 0 );
  pack_start( led_alignment, Gtk::PACK_SHRINK, 10 );

  mutex = vips_g_mutex_new();

  dispatcher.connect(sigc::mem_fun(*this, &StatusIndicatorWidget::update));

  show_all();
}


PF::StatusIndicatorWidget::~StatusIndicatorWidget()
{
  if(mutex) vips_g_mutex_free( mutex );
}


void PF::StatusIndicatorWidget::set_status( std::string l, int s )
{
  g_mutex_lock( mutex );
  cur_label = l;
  cur_status = s;
  g_mutex_unlock( mutex );
  //gdk_threads_add_idle ((GSourceFunc) pf_status_indicator_update, this);
  dispatcher.emit();
}


void PF::StatusIndicatorWidget::update()
{
  g_mutex_lock( mutex );
  label.set_text( cur_label.c_str() );
  led.set_status( cur_status );
  g_mutex_unlock( mutex );
}

