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
    std::cout << "LENSFUN, scanning cameras:" << std::endl;
  std::map<Glib::ustring, std::set<Glib::ustring>> camnames;
  auto camlist = rtengine::LFDatabase::getInstance()->getCameras();
  for (auto &c : camlist) {
    camnames[c.getMake()].insert(c.getModel());

      std::cout << "  found: " << c.getDisplayString().c_str() << std::endl;
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
    std::cout << "LENSFUN, scanning lenses:" << std::endl;
  std::map<Glib::ustring, std::set<Glib::ustring>> lenses;
  auto lenslist = rtengine::LFDatabase::getInstance()->getLenses();
  for (auto &l : lenslist) {
    auto name = l.getLens();
    auto make = l.getMake();
    lenses[make].insert(name);

      std::cout << "  found: " << l.getDisplayString().c_str() << std::endl;
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

void PF::LFComboBox::get_preferred_width_vfunc (int &minimum_width, int &natural_width) const
{
    natural_width = (naturalWidth > 10) ? naturalWidth : 10;
    minimum_width = (minimumWidth > 10) ? naturalWidth : 10;
}
void PF::LFComboBox::get_preferred_width_for_height_vfunc (int height, int &minimum_width, int &natural_width) const
{
    natural_width = (naturalWidth > 10) ? naturalWidth : 10;
    minimum_width = (minimumWidth > 10) ? naturalWidth : 10;
}
