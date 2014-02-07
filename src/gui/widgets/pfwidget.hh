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

#ifndef PF_WIDGET_HH
#define PF_WIDGET_HH

#include "../../base/image.hh"

namespace PF {

  class PFWidget
  {
    bool inhibit;
    OperationConfigUI* dialog;
    std::string pname;
    PropertyBase* property;

  public:
    PFWidget(OperationConfigUI* d, std::string n): 
      inhibit(false), dialog( d ), pname( n ), property( NULL )
    {
    }

    void set_inhibit( bool val ) { inhibit = val; }

    PropertyBase* get_prop() { return property; }

    void init()
    {
      Layer* layer = dialog->get_layer();
      Image* image = layer ? layer->get_image() : NULL;
      ProcessorBase* processor = layer ? layer->get_processor() : NULL;
      if( dialog && dialog->get_layer() && dialog->get_layer()->get_image() && 
	  dialog->get_layer()->get_processor() &&
	  dialog->get_layer()->get_processor()->get_par() ) {
	OpParBase* par = dialog->get_layer()->get_processor()->get_par();
	property = par->get_property( pname );
	inhibit = true;
	get_value();
	inhibit = false;
      }
    }

    virtual void get_value() = 0;
    virtual void set_value() = 0;

    void changed()
    {
      if( property && !inhibit ) {
	set_value();
	dialog->get_layer()->set_dirty( true );
	std::cout<<"  updating image"<<std::endl;
	dialog->get_layer()->get_image()->update();
      }
    }
  };


}

#endif
