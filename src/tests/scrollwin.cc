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
  /*
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
*/
public:
  ImageArea ()
  {
  }

  virtual ~ ImageArea ()
  {
  }

};


class ExampleWindow : public Gtk::Window
{
public:
  ExampleWindow();
  virtual ~ExampleWindow();

protected:
  //Signal handlers:
  void on_checkbutton_snap();
  void on_checkbutton_numeric();
  void on_spinbutton_digits_changed();
  void on_button_close();

  enum enumValueFormats
  {
    VALUE_FORMAT_INT,
    VALUE_FORMAT_FLOAT
  };
  void on_button_getvalue(enumValueFormats display);

  //Child widgets:
  Gtk::Frame m_Frame_NotAccelerated, m_Frame_Accelerated;
  Gtk::Box m_HBox_NotAccelerated, m_HBox_Accelerated,
    m_HBox_Buttons;
  Gtk::Box m_VBox_Main, m_VBox, m_VBox_Day, m_VBox_Month, m_VBox_Year,
    m_VBox_Accelerated, m_VBox_Value, m_VBox_Digits;
  Gtk::Label m_Label_Day, m_Label_Month, m_Label_Year,
    m_Label_Value, m_Label_Digits,
    m_Label_ShowValue;
  Glib::RefPtr<Gtk::Adjustment> m_adjustment_day, m_adjustment_month, m_adjustment_year,
    m_adjustment_value, m_adjustment_digits;
  Gtk::SpinButton m_SpinButton_Day, m_SpinButton_Month, m_SpinButton_Year,
    m_SpinButton_Value, m_SpinButton_Digits;
  Gtk::CheckButton m_CheckButton_Snap, m_CheckButton_Numeric;
  Gtk::Button m_Button_Int, m_Button_Float, m_Button_Close;
};


ExampleWindow::ExampleWindow()
:
  m_Frame_NotAccelerated("Not accelerated"),
  m_Frame_Accelerated("Accelerated"),
  m_VBox_Main(Gtk::ORIENTATION_VERTICAL, 5),
  m_VBox(Gtk::ORIENTATION_VERTICAL),
  m_VBox_Day(Gtk::ORIENTATION_VERTICAL),
  m_VBox_Month(Gtk::ORIENTATION_VERTICAL),
  m_VBox_Year(Gtk::ORIENTATION_VERTICAL),
  m_VBox_Accelerated(Gtk::ORIENTATION_VERTICAL),
  m_VBox_Value(Gtk::ORIENTATION_VERTICAL),
  m_VBox_Digits(Gtk::ORIENTATION_VERTICAL),
  m_Label_Day("Day: "),
  m_Label_Month("Month: "),
  m_Label_Year("Year: "),
  m_Label_Value("Value: "),
  m_Label_Digits("Digits: "),
  m_adjustment_day( Gtk::Adjustment::create(1.0, 1.0, 31.0, 1.0, 5.0, 0.0) ),
  m_adjustment_month( Gtk::Adjustment::create(1.0, 1.0, 12.0, 1.0, 5.0, 0.0) ),
  m_adjustment_year( Gtk::Adjustment::create(2012.0, 1.0, 2200.0, 1.0, 100.0, 0.0) ),
  m_adjustment_value( Gtk::Adjustment::create(0.0, -10000.0, 10000.0, 0.5, 100.0, 0.0) ),
  m_adjustment_digits( Gtk::Adjustment::create(2.0, 1.0, 5.0, 1.0, 1.0, 0.0) ),
  m_SpinButton_Day(m_adjustment_day),
  m_SpinButton_Month(m_adjustment_month),
  m_SpinButton_Year(m_adjustment_year),
  m_SpinButton_Value(m_adjustment_value, 1.0, 2),
  m_SpinButton_Digits(m_adjustment_digits),
  m_CheckButton_Snap("Snap to 0.5-ticks"),
  m_CheckButton_Numeric("Numeric only input mode"),
  m_Button_Int("Value as Int"),
  m_Button_Float("Value as Float"),
  m_Button_Close("Close")
{
  set_title("SpinButton");

  m_VBox_Main.set_border_width(10);
  add(m_VBox_Main);

  m_VBox_Main.pack_start(m_Frame_NotAccelerated);

  m_VBox.set_border_width(5);
  m_Frame_NotAccelerated.add(m_VBox);

  /* Day, month, year spinners */

  m_VBox.pack_start(m_HBox_NotAccelerated, Gtk::PACK_EXPAND_WIDGET, 5);

  m_Label_Day.set_alignment(Gtk::ALIGN_START);
  m_VBox_Day.pack_start(m_Label_Day);

  m_SpinButton_Day.set_wrap();

  m_VBox_Day.pack_start(m_SpinButton_Day);

  m_HBox_NotAccelerated.pack_start(m_VBox_Day, Gtk::PACK_EXPAND_WIDGET, 5);

  m_Label_Month.set_alignment(Gtk::ALIGN_START);
  m_VBox_Month.pack_start(m_Label_Month);

  m_SpinButton_Month.set_wrap();
  m_VBox_Month.pack_start(m_SpinButton_Month);

  m_HBox_NotAccelerated.pack_start(m_VBox_Month, Gtk::PACK_EXPAND_WIDGET, 5);

  m_Label_Year.set_alignment(Gtk::ALIGN_START);
  m_VBox_Year.pack_start(m_Label_Year);

  m_SpinButton_Year.set_wrap();
  m_SpinButton_Year.set_size_request(55, -1);
  m_VBox_Year.pack_start(m_SpinButton_Year);

  m_HBox_NotAccelerated.pack_start(m_VBox_Year, Gtk::PACK_EXPAND_WIDGET, 5);

  //Accelerated:
  m_VBox_Main.pack_start(m_Frame_Accelerated);

  m_VBox_Accelerated.set_border_width(5);
  m_Frame_Accelerated.add(m_VBox_Accelerated);

  m_VBox_Accelerated.pack_start(m_HBox_Accelerated, Gtk::PACK_EXPAND_WIDGET, 5);

  m_HBox_Accelerated.pack_start(m_VBox_Value, Gtk::PACK_EXPAND_WIDGET, 5);

  m_Label_Value.set_alignment(Gtk::ALIGN_START);
  m_VBox_Value.pack_start(m_Label_Value);

  m_SpinButton_Value.set_wrap();
  m_SpinButton_Value.set_size_request(100, -1);
  m_VBox_Value.pack_start(m_SpinButton_Value);

  m_HBox_Accelerated.pack_start(m_VBox_Digits, Gtk::PACK_EXPAND_WIDGET, 5);

  m_Label_Digits.set_alignment(Gtk::ALIGN_START);
  m_VBox_Digits.pack_start(m_Label_Digits);

  m_SpinButton_Digits.set_wrap();
  m_adjustment_digits->signal_value_changed().connect( sigc::mem_fun(*this,
              &ExampleWindow::on_spinbutton_digits_changed) );

  m_VBox_Digits.pack_start(m_SpinButton_Digits);


  //CheckButtons:
  m_VBox_Accelerated.pack_start(m_CheckButton_Snap);
  m_CheckButton_Snap.set_active();
  m_CheckButton_Snap.signal_clicked().connect( sigc::mem_fun(*this,
              &ExampleWindow::on_checkbutton_snap) );

  m_VBox_Accelerated.pack_start(m_CheckButton_Numeric);
  m_CheckButton_Numeric.set_active();
  m_CheckButton_Numeric.signal_clicked().connect( sigc::mem_fun(*this,
              &ExampleWindow::on_checkbutton_numeric) );


  //Buttons:
  m_VBox_Accelerated.pack_start (m_HBox_Buttons, Gtk::PACK_SHRINK, 5);

  m_Button_Int.signal_clicked().connect( sigc::bind( sigc::mem_fun(*this,
                  &ExampleWindow::on_button_getvalue), VALUE_FORMAT_INT) );
  m_HBox_Buttons.pack_start(m_Button_Int, Gtk::PACK_EXPAND_WIDGET, 5);

  m_Button_Float.signal_clicked().connect( sigc::bind( sigc::mem_fun(*this,
                  &ExampleWindow::on_button_getvalue), VALUE_FORMAT_FLOAT) );
  m_HBox_Buttons.pack_start(m_Button_Float, Gtk::PACK_EXPAND_WIDGET, 5);

  m_VBox_Accelerated.pack_start(m_Label_ShowValue);
  m_Label_ShowValue.set_text("0");

  //Close button:
  m_Button_Close.signal_clicked().connect( sigc::mem_fun(*this,
              &ExampleWindow::on_button_close) );
  m_VBox_Main.pack_start(m_Button_Close, Gtk::PACK_SHRINK);

  show_all_children();
}

ExampleWindow::~ExampleWindow()
{
}


void ExampleWindow::on_button_close()
{
  hide();
}

void ExampleWindow::on_checkbutton_snap()
{
  m_SpinButton_Value.set_snap_to_ticks( m_CheckButton_Snap.get_active() );
}

void ExampleWindow::on_checkbutton_numeric()
{
  m_SpinButton_Value.set_numeric( m_CheckButton_Numeric.get_active() );
}

void ExampleWindow::on_spinbutton_digits_changed()
{
  m_SpinButton_Value.set_digits( m_SpinButton_Digits.get_value_as_int() );
}

void ExampleWindow::on_button_getvalue(enumValueFormats display)
{
  gchar buf[32];

  if (display == VALUE_FORMAT_INT)
    sprintf (buf, "%d", m_SpinButton_Value.get_value_as_int());
  else
    sprintf (buf, "%0.*f", m_SpinButton_Value.get_digits(),
            m_SpinButton_Value.get_value());

  m_Label_ShowValue.set_text(buf);
}



int
main (int argc, char **argv)
{
  //Gtk::Main kit (argc, argv);
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

/*
  Gtk::Window window;
  window.set_default_size (500, 500);
  Gtk::SpinButton spinbutton;
  window.add(spinbutton);
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
*/

  ExampleWindow window;

    //Shows the window and returns when it is closed.
    return app->run(window);

  //Gtk::Main::run (window);

  return 0;
}
