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

#ifndef PF_MULTIRAW_DEVELOPER_H
#define PF_MULTIRAW_DEVELOPER_H

#include <string>

//#include <libraw/libraw.h>

#include "raw_developer.hh"

namespace PF 
{

  class MultiRawDeveloperPar: public OpParBase
  {
    std::vector< std::pair<PF::ProcessorBase*,PF::ProcessorBase*> > developers;

		bool caching_enabled;

  public:
    MultiRawDeveloperPar();

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
          creation of an intensity map is not allowed
       2. the operation can work without an input image;
          the blending will be set in this case to "passthrough" and the image
	  data will be simply linked to the output
     */
    bool has_intensity() { return false; }
    bool has_opacity() { return false; }
    bool needs_input() { return false; }
    bool needs_caching() { return caching_enabled; }

    void set_caching( bool flag ) { caching_enabled = flag; }

    void add_image( std::string fname );

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class MultiRawDeveloper
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
		}
	};




  ProcessorBase* new_multiraw_developer();
}

#endif 


