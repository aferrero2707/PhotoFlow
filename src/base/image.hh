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

#ifndef IMAGE_H
#define IMAGE_H

#include <sigc++/sigc++.h>

#include "layermanager.hh"
#include "view.hh"


namespace PF
{


  class Image: public sigc::trackable
  {
    LayerManager layer_manager;
    std::vector<View> views;

  public:
    Image() {}

    sigc::signal<void> signal_modified;

    LayerManager& get_layer_manager() { return layer_manager; }

    void add_view( VipsBandFormat fmt, int level )
    {
      views.push_back( View( fmt, level ) );
    }

    View* get_view(unsigned int n) 
    {
      if( n >= views.size() ) return NULL;
      return(&views[n]);
    }

    void update();
  };

}


#endif /*VIPS_PARITHMETIC_H*/


