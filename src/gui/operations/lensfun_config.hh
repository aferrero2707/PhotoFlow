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

#ifndef LENSFUN_CONFIG_DIALOG_HH
#define LENSFUN_CONFIG_DIALOG_HH

#include <gtkmm.h>

#ifdef PF_HAS_LENSFUN
#include <lensfun.h>
#endif

#include "../operation_config_dialog.hh"
#include "../../operations/lensfun.hh"


namespace PF {

class LensFunConfigDialog: public OperationConfigDialog
{
  //#ifdef GTKMM_2
  Gtk::VBox controlsBox;
  Gtk::HBox hbox1, hbox2, hbox3;

  Gtk::Label label1, label2, label3;
  Gtk::Entry makerEntry, modelEntry, lensEntry;
  //#endif

#ifdef PF_HAS_LENSFUN
  lfDatabase* ldb;
#endif

public:
  LensFunConfigDialog( Layer* l );

  void on_maker_changed();

  void do_update();
};

}

#endif
