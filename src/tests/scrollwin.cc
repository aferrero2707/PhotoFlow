/* Tiny display-an-image demo program. 
 *
 * This is not supposed to be a complete image viewer, it's just supposed to 
 * show how to display a VIPS image (or the result of a VIPS computation) in a
 * window.
 *
 * 8-bit RGB images only, though it would be easy to fix this.
 *
 * Compile with:

	g++ -g -Wall gtkdisp2.cc \
		`pkg-config vipsCC-7.26 gtkmm-2.4 --cflags --libs` 

 */

#include <stdio.h>
#include <iostream>

#include <gtkmm.h>

/* Subclass DrawingArea to make a widget that displays a VImage.
 */
class ImageArea : public Gtk::DrawingArea
{
private:
  virtual bool on_expose_event (GdkEventExpose * event)
  {
    guchar *buf = new guchar[get_width()*get_height()*3];
    int lsk = get_width()*3;

    guchar *ptr = buf;
    for(int i = 0; i < get_height(); i++) {
      for(int j = 0; j < get_width(); j++) {
        ptr[0] = 255; ptr[1] = 0; ptr[2] = 0;
        ptr += 3;
      }
    }

    //std::cout<<"width="<<get_width()<<"  height="<<get_height()<<std::endl;

    get_window()->draw_rgb_image (get_style()->get_white_gc (),
           0, 0, get_width(), get_height(),
           Gdk::RGB_DITHER_MAX, buf, lsk);

    delete buf;
    return TRUE;
  }

public:
  ImageArea ()
  {
  }

  virtual ~ ImageArea ()
  {
  }

};

int
main (int argc, char **argv)
{
  Gtk::Main kit (argc, argv);

  Gtk::Window window;
  window.set_default_size (500, 500);
  Gtk::ScrolledWindow scrolled_window;
  scrolled_window.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

  window.add (scrolled_window);

  Gtk::EventBox eb;
  Gdk::Color bg; bg.set_rgb_p(0.1,0.1,0.1);
  eb.modify_bg(Gtk::STATE_NORMAL,bg);
  scrolled_window.add(eb);

  Gtk::VBox vbox;
  eb.add (vbox);
  Gtk::HBox hbox;
  vbox.pack_start( hbox, Gtk::PACK_EXPAND_PADDING );

ImageArea area;
  area.set_size_request (100, 100);
  //area.set_halign(Gtk::ALIGN_CENTER);

  hbox.pack_start( area, Gtk::PACK_EXPAND_PADDING );
  window.show_all ();

  Gtk::Main::run (window);

  return 0;
}
