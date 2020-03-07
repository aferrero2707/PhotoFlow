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
#include "../../rt/rtengine/rtlensfun.h"
#include "lensfun_selector.hh"

static PF::LFDbHelper *lf;


PF::LFCamSelector::LFCamSelector( OperationConfigGUI* dialog, std::string pname, std::string l, int val, int width ):
      Gtk::HBox(),
      PF::PFWidget( dialog, pname )
{
  if (!lf) {
      lf = new PF::LFDbHelper();
  }

  cbox.set_model(lf->lensfunCameraModel);
  cbox.pack_start(lf->lensfunModelCam.model);
  if( !cbox.get_cells().empty() ) {
    Gtk::CellRendererText* cellRenderer =
        dynamic_cast<Gtk::CellRendererText*>( *(cbox.get_cells().begin()) );
    if( cellRenderer ) {
      cellRenderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;
      cellRenderer->property_ellipsize_set() = true;
    }
  }
  cbox.setPreferredWidth(50, 120);
  cbox.set_width(150, 300);


  label.set_text( l.c_str() );

  pack_end( cbox, Gtk::PACK_SHRINK );
  pack_end( label, Gtk::PACK_SHRINK );

  cbox.signal_changed().
      connect(sigc::mem_fun(*this,
          &PFWidget::changed));

  show_all_children();
}


PF::LFCamSelector::LFCamSelector( OperationConfigGUI* dialog, PF::ProcessorBase* processor, std::string pname, std::string l, int val, int width ):
      Gtk::HBox(),
      PF::PFWidget( dialog, processor, pname )
{
  if (!lf) {
      lf = new PF::LFDbHelper();
  }

  cbox.set_model(lf->lensfunCameraModel);
  cbox.pack_start(lf->lensfunModelCam.model);
  if( !cbox.get_cells().empty() ) {
    Gtk::CellRendererText* cellRenderer =
        dynamic_cast<Gtk::CellRendererText*>( *(cbox.get_cells().begin()) );
    if( cellRenderer ) {
      cellRenderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;
      cellRenderer->property_ellipsize_set() = true;
    }
  }
  cbox.setPreferredWidth(50, 120);

  label.set_text( l.c_str() );

  pack_start( label, Gtk::PACK_SHRINK );
  pack_start( cbox, Gtk::PACK_SHRINK );

  if( width > 0 ) {
    Glib::ListHandle< Gtk::CellRenderer* > cells = cbox.get_cells();
    Glib::ListHandle< Gtk::CellRenderer* >::iterator ci = cells.begin();
    for( ci = cells.begin(); ci != cells.end(); ci++ ) {
      (*ci)->set_fixed_size( width, -1 );
    }
  }

  //pack_start( vbox, Gtk::PACK_SHRINK );

  cbox.signal_changed().
      connect(sigc::mem_fun(*this,
          &PFWidget::changed));

  show_all_children();
}


void PF::LFCamSelector::get_value()
{
  if( !get_prop() ) return;
  if( !get_prop()->is_enum() ) return;

}


void PF::LFCamSelector::set_value()
{
  if( !get_prop() ) return;

  Gtk::TreeModel::iterator iter = cbox.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      //Glib::ustring value = row[columns.col_value];
      Glib::ustring value = row[lf->lensfunModelCam.model];

#ifndef NDEBUG
      std::cout << "selected value=" << value << std::endl;
#endif
      std::string str = value.c_str();
      get_prop()->update(str);
    }
  }
}




PF::LFLensSelector::LFLensSelector( OperationConfigGUI* dialog, std::string pname, std::string l, int val, int width ):
      Gtk::HBox(),
      PF::PFWidget( dialog, pname )
{
  if (!lf) {
      lf = new PF::LFDbHelper();
  }

  cbox.set_model(lf->lensfunLensModel);
  cbox.pack_start(lf->lensfunModelLens.prettylens);
  if( !cbox.get_cells().empty() ) {
    Gtk::CellRendererText* cellRenderer =
        dynamic_cast<Gtk::CellRendererText*>( *(cbox.get_cells().begin()) );
    if( cellRenderer ) {
      cellRenderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;
      cellRenderer->property_ellipsize_set() = true;
    }
  }
#ifdef GTKMM_3
  cbox.setPreferredWidth(50, 120);
  cbox.set_font_size(9);
#else
  cbox.set_width(150, 300);
  cbox.set_font_size(9);
#endif


  label.set_text( l.c_str() );

  pack_end( cbox, Gtk::PACK_SHRINK );
  pack_end( label, Gtk::PACK_SHRINK );

  cbox.signal_changed().
      connect(sigc::mem_fun(*this,
          &PFWidget::changed));

  show_all_children();
}


PF::LFLensSelector::LFLensSelector( OperationConfigGUI* dialog, PF::ProcessorBase* processor, std::string pname, std::string l, int val, int width ):
      Gtk::HBox(),
      PF::PFWidget( dialog, processor, pname )
{
  if (!lf) {
      lf = new PF::LFDbHelper();
  }

  cbox.set_model(lf->lensfunLensModel);
  cbox.pack_start(lf->lensfunModelLens.prettylens);
  if( !cbox.get_cells().empty() ) {
    Gtk::CellRendererText* cellRenderer =
        dynamic_cast<Gtk::CellRendererText*>( *(cbox.get_cells().begin()) );
    if( cellRenderer ) {
      cellRenderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;
      cellRenderer->property_ellipsize_set() = true;
    }
  }
  cbox.setPreferredWidth(50, 120);
  //cbox.set_width(150);
  //cbox.set_font_size(8);

  label.set_text( l.c_str() );

  pack_start( label, Gtk::PACK_SHRINK );
  pack_start( cbox, Gtk::PACK_SHRINK );

  cbox.signal_changed().
      connect(sigc::mem_fun(*this,
          &PFWidget::changed));

  show_all_children();
}


void PF::LFLensSelector::get_value()
{
  if( !get_prop() ) return;
  if( !get_prop()->is_enum() ) return;
}


void PF::LFLensSelector::set_value()
{
  if( !get_prop() ) return;

  Gtk::TreeModel::iterator iter = cbox.get_active();
  if( iter ) {
    Gtk::TreeModel::Row row = *iter;
    if( row ) {
      //Get the data for the selected row, using our knowledge of the tree
      //model:
      //Glib::ustring value = row[columns.col_value];
      Glib::ustring value = row[lf->lensfunModelLens.lens];

#ifndef NDEBUG
      std::cout << "selected value=" << value << std::endl;
#endif
      std::string str = value.c_str();
      get_prop()->update(str);
    }
  }
}



PF::LFSelector::LFSelector( OperationConfigGUI* dialog, std::string pname, std::string pn2, std::string pn3 ): Gtk::HBox(),
    PF::PFWidget( dialog, pname ), pname2(pn2), property2(NULL), pname3(pn3), property3(NULL)
{
  if (!lf) {
    std::cout<<"LFSelector::LFSelector(): creating new LF DB helper"<<std::endl;
      lf = new PF::LFDbHelper();
  }

  cam_label.set_tooltip_text(_("choose the camera model"));
  lens_label.set_tooltip_text(_("choose the lens model"));

  cam_label.set_text("----");
  cam_label.set_size_request( -1, 30 );
  cam_frame.add( cam_label );
  cam_ebox.add( cam_frame );
  vbox.pack_start(cam_ebox);

  lens_label.set_text("----");
  lens_label.set_size_request( -1, 30 );
  lens_frame.add( lens_label );
  lens_ebox.add( lens_frame );
  vbox.pack_start(lens_ebox);

  cb_hbox.set_tooltip_text(_("only list lenses compatible with the selected camera model"));
  cb_hbox.pack_end(cb, Gtk::PACK_SHRINK);
  cb_hbox.pack_end(cb_label, Gtk::PACK_SHRINK);
  cb_label.set_text(_("match camera"));
  cb.set_active(false);
  //vbox.pack_start(cb_hbox);

  vbox.set_spacing(4);

  pack_start(vbox, Gtk::PACK_EXPAND_WIDGET, 0);

  fill_cam_menu();
  fill_lens_menu_full();

  cam_ebox.signal_button_release_event().
      connect( sigc::mem_fun(*this, &PF::LFSelector::my_cam_button_release_event) );
  lens_ebox.signal_button_release_event().
      connect( sigc::mem_fun(*this, &PF::LFSelector::my_lens_button_release_event) );

  show_all_children();
}


void PF::LFSelector::init()
{
  if( get_processor() && get_processor()->get_par() ) {
    property2 = get_processor()->get_par()->get_property( pname2 );
    property3 = get_processor()->get_par()->get_property( pname3 );
  }

  PF::PFWidget::init();

  return;
}


void PF::LFSelector::update_cam( Glib::ustring maker_new, Glib::ustring model_new )
{
  bool modified = false;
  if( cam_maker_name != maker_new ) modified = true;
  if( cam_model_name != model_new ) modified = true;
  cam_maker_name = maker_new;
  cam_model_name = model_new;
  if( enabled && modified ) changed();
}


void PF::LFSelector::update_lens( Glib::ustring lens_new )
{
  bool modified = false;
  if( lens_name != lens_new ) modified = true;
  lens_name = lens_new;
  if( enabled && modified ) changed();
}


bool PF::LFSelector::my_cam_button_release_event( GdkEventButton* event )
{
  std::cout<<"LFSelector::my_cam_button_release_event(): enable="<<enabled<<std::endl;
  if( !enabled ) return false;
  cam_menu.popup(event->button, event->time);
  return false;
}

bool PF::LFSelector::my_lens_button_release_event( GdkEventButton* event )
{
  if( !enabled ) return false;
  if( cb.get_active() ) {
    fill_lens_menu();
    lens_menu.popup(event->button, event->time);
  } else {
    lens_menu_full.popup(event->button, event->time);
  }
  return false;
}



void PF::LFSelector::on_cam_item_clicked(Glib::ustring make_, Glib::ustring model_)
{
  update_cam( make_, model_ );
  std::cout<<"LFSelector::on_item_clicked(): model \""
      <<cam_maker_name + " " + cam_model_name<<"\" selected"<<std::endl;
  cam_label.set_text( cam_model_name );
  cam_label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  cam_label.set_tooltip_text( cam_maker_name + " " + cam_model_name );
}

void PF::LFSelector::on_lens_item_clicked(Glib::ustring lens, Glib::ustring prettylens)
{
  std::cout<<"LFSelector::on_item_clicked(): lens \""<<lens<<"\" selected"<<std::endl;
  lens_label.set_text( prettylens );
  lens_label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  lens_label.set_tooltip_text( lens );
  update_lens( prettylens );
}


void PF::LFSelector::set_cam(Glib::ustring cam_make, Glib::ustring cam_model)
{
  cam_label.set_text( _("Unknown camera") );
  cam_label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  cam_label.set_tooltip_text( _("Unknown camera") );

  rtengine::LFCamera lfcamera = rtengine::LFDatabase::getInstance()->findCamera(cam_make, cam_model);
  if( lfcamera ) {
    update_cam( lfcamera.getMake(), lfcamera.getModel() );
      //std::cout<<"LFSelector::set_cam(): model \""
      //    <<cam_maker_name + " " + cam_model_name<<"\" selected"<<std::endl;
      cam_label.set_text( cam_model_name );
      cam_label.set_tooltip_text( cam_maker_name + " " + cam_model_name );
  } else {
    //std::cout<<"LFSelector::set_lens(): cannot find camera \""<<cam_model<<"\" in database"<<std::endl;
  }
}


void PF::LFSelector::set_lens(Glib::ustring lens)
{
  lens_label.set_text( _("Unknown lens") );
  lens_label.set_ellipsize( Pango::ELLIPSIZE_MIDDLE );
  lens_label.set_tooltip_text( _("Unknown lens") );

  rtengine::LFCamera lfcamera = rtengine::LFDatabase::getInstance()->findCamera(cam_maker_name, cam_model_name);
  rtengine::LFLens lflens;
  if( lfcamera ) {
    bool relax = !(cb.get_active());
    //if( !enabled ) relax = true;
    lflens  = rtengine::LFDatabase::getInstance()->findLens(lfcamera, lens, relax);
    if( lflens ) {
      Glib::ustring lens_make = lflens.getMake();
      Glib::ustring lens_model = lflens.getLens();
      Glib::ustring lpretty;
      if (lens_model.find(lens_make, lens_make.size()+1) == lens_make.size()+1) {
        lpretty = lens_model.substr(lens_make.size()+1);
      } else {
        lpretty = lens_model;
      }
      //std::cout<<"LFSelector::set_lens(): lens \""<<lpretty<<"\" (\""<<lens_model<<"\") selected"<<std::endl;
      lens_label.set_text( lpretty );
      lens_label.set_tooltip_text( lens_model );
      update_lens( lens_model );
    } else {
      //std::cout<<"LFSelector::set_lens(): cannot find lens \""<<lens<<"\" in database"<<std::endl;
      update_lens( "" );
    }
  } else {
    //std::cout<<"LFSelector::set_lens(): cannot find camera \""
    //    <<cam_maker_name<<" / "<<cam_model_name<<"\" in database"<<std::endl;
    update_lens( "" );
  }
}


void PF::LFSelector::fill_cam_menu()
{
  //std::cout << "LENSFUN, scanning cameras:" << std::endl;
  std::map<Glib::ustring, std::set<Glib::ustring>> camnames;
  auto camlist = rtengine::LFDatabase::getInstance()->getCameras();
  for (auto &c : camlist) {
    camnames[c.getMake()].insert(c.getModel());

    //std::cout << "  found: " << c.getDisplayString().c_str() << std::endl;
  }
  for (auto &p : camnames) {
    Gtk::MenuItem* item = new Gtk::MenuItem(p.first);
    cam_menu.append( *item );
    Gtk::Menu* cmenu = new Gtk::Menu;
    item->set_submenu( *cmenu );
    for (auto &c : p.second) {
      Gtk::MenuItem* citem = new Gtk::MenuItem( c );
      cmenu->append( *citem );

      citem->signal_activate().connect(
          sigc::bind<Glib::ustring>(
              sigc::mem_fun(*this,&LFSelector::on_cam_item_clicked), p.first, c ) );

      //citem->signal_activate().connect( sigc::mem_fun(*this, &PFWidget::changed) );
    }
  }
  cam_menu.show_all();
}


void PF::LFSelector::fill_lens_menu()
{
  rtengine::LFCamera lfcamera = rtengine::LFDatabase::getInstance()->findCamera(cam_maker_name, cam_model_name);

  std::vector< Gtk::Widget* > children = lens_menu.get_children();
  for(unsigned int i = 0; i < children.size(); i++) {
    Gtk::Widget* w = children[i];
    lens_menu.remove(*w);
    delete w;
  }

  if( !lfcamera ) return;

  //std::cout << "LENSFUN, scanning lenses:" << std::endl;
  std::map<Glib::ustring, std::set<Glib::ustring>> lenses;
  auto lenslist = rtengine::LFDatabase::getInstance()->getLenses();
  for (auto &l : lenslist) {
    auto name = l.getLens();
    auto make = l.getMake();
    lenses[make].insert(name);
    //std::cout << "  found: " << l.getDisplayString().c_str() << " ("<<l.getLens()<<")" << std::endl;
  }
  for (auto &p : lenses) {
    Gtk::Menu* cmenu = NULL;
    for (auto &c : p.second) {
      rtengine::LFLens lflens;
      lflens  = rtengine::LFDatabase::getInstance()->findLens(lfcamera, c, false);
      //std::cout<<"camera: "<<lfcamera.getMake()<<" - "<<lfcamera.getModel()<<std::endl;
      //std::cout<<"  lflens \""<<c<<"\": "<<lflens.getLens()<<std::endl;
      if( !lflens ) continue;
      if( !cmenu ) {
        Gtk::MenuItem* item = new Gtk::MenuItem(p.first);
        lens_menu.append( *item );
        cmenu = new Gtk::Menu;
        item->set_submenu( *cmenu );
      }
      Gtk::MenuItem* citem;
      Glib::ustring cpretty;
      if (c.find(p.first, p.first.size()+1) == p.first.size()+1) {
        cpretty = c.substr(p.first.size()+1);
      } else {
        cpretty = c;
      }
      citem = new Gtk::MenuItem( cpretty );
      cmenu->append( *citem );

      citem->signal_activate().connect(
          sigc::bind<Glib::ustring>(
              sigc::mem_fun(*this,&LFSelector::on_lens_item_clicked), c, cpretty ) );
    }
  }
  lens_menu.show_all();
}


void PF::LFSelector::fill_lens_menu_full()
{
  //std::cout << "LENSFUN, scanning lenses:" << std::endl;
  std::map<Glib::ustring, std::set<Glib::ustring>> lenses;
  auto lenslist = rtengine::LFDatabase::getInstance()->getLenses();
  for (auto &l : lenslist) {
    auto name = l.getLens();
    auto make = l.getMake();
    lenses[make].insert(name);

      //std::cout << "  found: " << l.getDisplayString().c_str() << std::endl;
          //<< " ("<<l.getLens()<<")" << std::endl;
  }
  for (auto &p : lenses) {
    Gtk::Menu* cmenu = NULL;
    Gtk::MenuItem* item = new Gtk::MenuItem(p.first);
    lens_menu_full.append( *item );
    cmenu = new Gtk::Menu;
    item->set_submenu( *cmenu );
    for (auto &c : p.second) {
      Gtk::MenuItem* citem;
      Glib::ustring cpretty;
      if (c.find(p.first, p.first.size()+1) == p.first.size()+1) {
        cpretty = c.substr(p.first.size()+1);
      } else {
        cpretty = c;
      }
      citem = new Gtk::MenuItem( cpretty );
      cmenu->append( *citem );

      citem->signal_activate().connect(
          sigc::bind<Glib::ustring>(
              sigc::mem_fun(*this,&LFSelector::on_lens_item_clicked), c, cpretty ) );
    }
  }
  lens_menu_full.show_all();
}



void PF::LFSelector::get_value()
{
  if( !get_prop() ) return;
  if( !property2 ) return;
  if( !property3 ) return;

  PF::Property<std::string>* strprop = dynamic_cast< PF::Property<std::string>* >( get_prop() );
  if( !strprop ) return;

  PF::Property<std::string>* strprop2 = dynamic_cast< PF::Property<std::string>* >( property2 );
  if( !strprop2 ) return;

  PF::Property<std::string>* strprop3 = dynamic_cast< PF::Property<std::string>* >( property3 );
  if( !strprop2 ) return;

  std::string str = strprop->get();
  cam_maker_name = str;
  str = strprop2->get();
  cam_model_name = str;

  set_cam(cam_maker_name, cam_model_name);

  str = strprop3->get();
  lens_name = str;

  //std::cout<<"LFSelector::get_value(): lens_name=\""<<lens_name<<"\""<<std::endl;
  set_lens(lens_name);
}


void PF::LFSelector::set_value()
{
  if( !get_prop() ) return;
  if( !property2 ) return;
  if( !property3 ) return;

  std::string str = cam_maker_name.c_str();
  get_prop()->update(str);
  str = cam_model_name.c_str();
  property2->update(str);

  std::cout<<"LFSelector::set_value(): camera properties set to \""
      <<cam_maker_name<<" / "<<cam_model_name<<"\""<<std::endl;

  str = lens_name.c_str();
  property3->update(str);

  std::cout<<"LFSelector::set_value(): lens property set to \""<<lens_name<<"\""<<std::endl;
}


//-----------------------------------------------------------------------------
// LFDbHelper
//-----------------------------------------------------------------------------

PF::LFDbHelper::LFDbHelper()
{
  std::cout<<"LFDbHelper::LFDbHelper() called"<<std::endl;
  lensfunCameraModel = Gtk::TreeStore::create(lensfunModelCam);
  fillLensfunCameras();
  lensfunLensModel = Gtk::TreeStore::create(lensfunModelLens);
  fillLensfunLenses();
}

void PF::LFDbHelper::fillLensfunCameras()
{
  //std::cout << "LENSFUN, scanning cameras:" << std::endl;
  std::map<Glib::ustring, std::set<Glib::ustring>> camnames;
  auto camlist = rtengine::LFDatabase::getInstance()->getCameras();
  for (auto &c : camlist) {
    camnames[c.getMake()].insert(c.getModel());

    //std::cout << "  found: " << c.getDisplayString().c_str() << std::endl;
  }
  for (auto &p : camnames) {
    Gtk::TreeModel::Row row = *(lensfunCameraModel->append());
    row[lensfunModelCam.make] = p.first;
    row[lensfunModelCam.model] = p.first;
    for (auto &c : p.second) {
      Gtk::TreeModel::Row child = *(lensfunCameraModel->append(row.children()));
      child[lensfunModelCam.make] = p.first;
      child[lensfunModelCam.model] = c;
    }
  }
}


void PF::LFDbHelper::fillLensfunLenses()
{
  //std::cout << "LENSFUN, scanning lenses:" << std::endl;
  std::map<Glib::ustring, std::set<Glib::ustring>> lenses;
  auto lenslist = rtengine::LFDatabase::getInstance()->getLenses();
  for (auto &l : lenslist) {
    auto name = l.getLens();
    auto make = l.getMake();
    lenses[make].insert(name);

    //std::cout << "  found: " << l.getDisplayString().c_str() << std::endl;
  }
  for (auto &p : lenses) {
    Gtk::TreeModel::Row row = *(lensfunLensModel->append());
    row[lensfunModelLens.lens] = p.first;
    row[lensfunModelLens.prettylens] = p.first;
    for (auto &c : p.second) {
      Gtk::TreeModel::Row child = *(lensfunLensModel->append(row.children()));
      child[lensfunModelLens.lens] = c;
      if (c.find(p.first, p.first.size()+1) == p.first.size()+1) {
        child[lensfunModelLens.prettylens] = c.substr(p.first.size()+1);
      } else {
        child[lensfunModelLens.prettylens] = c;
      }
    }
  }
}


PF::LFComboBox::LFComboBox ()
{
    minimumWidth = naturalWidth = 70;
}

void PF::LFComboBox::setPreferredWidth (int minimum_width, int natural_width)
{
    if (natural_width == -1 && minimum_width == -1) {
        naturalWidth = minimumWidth = 70;
    } else if (natural_width == -1) {
        naturalWidth =  minimumWidth = minimum_width;
    } else if (minimum_width == -1) {
        naturalWidth = natural_width;
        minimumWidth = (naturalWidth/2 > 20) ? naturalWidth : 20;
        minimumWidth = (naturalWidth < minimumWidth) ? naturalWidth : minimumWidth;
    } else {
        naturalWidth = natural_width;
        minimumWidth = minimum_width;
    }
}


void PF::LFComboBox::set_width(int width, int child_width)
{
  if( width <= 0 ) return;
  Glib::ListHandle< Gtk::CellRenderer* > cells = get_cells();
  Glib::ListHandle< Gtk::CellRenderer* >::iterator ci = cells.begin();
  for( ci = cells.begin(); ci != cells.end(); ci++ ) {
    std::cout<<"LFComboBox::set_width(): is_expander: "<<(*ci)->property_is_expander()<<std::endl;
    if( (*ci)->property_is_expander() ) {
      std::cout<<"LFComboBox::set_width(): expander width set to "<<width<<std::endl;
      (*ci)->set_fixed_size( width, -1 );
    } else {
      (*ci)->set_fixed_size( child_width, -1 );
    }
  }
}


void PF::LFComboBox::set_font_size(int size)
{
  Glib::ListHandle< Gtk::CellRenderer* > cells = get_cells();
  Glib::ListHandle< Gtk::CellRenderer* >::iterator ci = cells.begin();
  for( ci = cells.begin(); ci != cells.end(); ci++ ) {
    Gtk::CellRendererText* cellRendererText =
    dynamic_cast<Gtk::CellRendererText*>(*ci);
    if( cellRendererText )
      cellRendererText->property_size_points() = size;
  }
}


void PF::LFComboBox::get_preferred_width_vfunc (int &minimum_width, int &natural_width) const
{
    natural_width = (naturalWidth > 10) ? naturalWidth : 10;
    minimum_width = (minimumWidth > 10) ? minimumWidth : 10;
    std::cout<<"LFComboBox::get_preferred_width_vfunc(): "<<minimum_width<<" "<<natural_width<<std::endl;
}
void PF::LFComboBox::get_preferred_width_for_height_vfunc (int height, int &minimum_width, int &natural_width) const
{
    natural_width = (naturalWidth > 10) ? naturalWidth : 10;
    minimum_width = (minimumWidth > 10) ? minimumWidth : 10;
    std::cout<<"LFComboBox::get_preferred_width_for_height_vfunc(): "<<minimum_width<<" "<<natural_width<<std::endl;
}
