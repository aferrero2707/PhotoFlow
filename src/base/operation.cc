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

#include "operation.hh"

int
vips_layer( VipsImage **in, int n, VipsImage **out, int first, 
	    PF::ProcessorBase* proc,
	    VipsImage* imap, VipsImage* omap, 
	    VipsDemandStyle demand_hint);


PF::OpParBase::OpParBase()
{
  processor = NULL;
  out = NULL;
  blend_mode = PF_BLEND_PASSTHROUGH;
  intensity = 1;
  opacity = 1;
  demand_hint = VIPS_DEMAND_STYLE_THINSTRIP;

  PF::PropertyBase* prop;
  prop = new PF::Property<float>("intensity",&intensity);
}



void PF::OpParBase::build(std::vector<VipsImage*>& in, int first, VipsImage* imap, VipsImage* omap)
{
  VipsImage* outnew;

  /*
  VipsArea *area = NULL;
  if( !in.empty() ) {
    VipsImage **array; 
    area = vips_area_new_array_object( in.size() );
    array = (VipsImage **) area->data;
    for( int i = 0; i < in.size(); i++ ) {
      array[i] = in[i];
      g_object_ref( array[i] );
    }
  }
  if (vips_call("layer", area, &outnew, first, processor, imap, omap, get_demand_hint() ))
    verror ();
  if(area) vips_area_unref( area );
  */
  /**/
  VipsImage* invec[100];
  int n = in.size(); if(n >100) n = 100;
  for(int i = 0; i < n; i++) {
    invec[i] = in[i];
  }
  vips_layer( invec, n, &outnew, first, processor, imap, omap, get_demand_hint() );
  /**/

  if(out)
    g_object_unref( out );
  out = outnew;
}
