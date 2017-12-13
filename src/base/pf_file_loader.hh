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

#ifndef PF_FILE_LOADER_HH
#define PF_FILE_LOADER_HH


#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>


#include "image.hh"


namespace PF
{

  /*
  static Image* image = NULL;
  static std::stack<Layer*> layers_stack;
  static Layer* current_layer = NULL;
  static OpParBase* current_op = NULL;

  // The handler functions.

  void start_element (GMarkupParseContext *context,
		      const gchar         *element_name,
		      const gchar        **attribute_names,
		      const gchar        **attribute_values,
		      gpointer             user_data,
		      GError             **error);

  void text(GMarkupParseContext *context,
	    const gchar         *text,
	    gsize                text_len,
	    gpointer             user_data,
	    GError             **error);

  void end_element (GMarkupParseContext *context,
		    const gchar         *element_name,
		    gpointer             user_data,
		    GError             **error);
  */

  bool load_pf_image( std::string filename, Image* image );
  void insert_pf_preset( std::string filename, PF::Image* img, PF::Layer* previous, std::list<PF::Layer*>* list, bool map_flag );


}



#endif
