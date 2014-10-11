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

#ifndef CIMG_BLUR_BILATERAL_H
#define CIMG_BLUR_BILATERAL_H

#include <assert.h>
#include <string>

#include "../base/processor.hh"


namespace PF 
{

  class GMicPar: public OpParBase
  {
    ProcessorBase* convert_format;
    ProcessorBase* convert_format2;

    Property<int> iterations;
    Property<std::string> command; 
    Property<int> padding;
    Property<float> x_scale;
    Property<float> y_scale; 

  public:
    GMicPar();

    bool has_intensity() { return false; }
    bool has_opacity() { return true; }

    void set_command( std::string cmd ) { command.set( cmd ); }
    void set_iterations( int it ) { iterations.set( it ); };
    void set_padding( int p ) { padding.set( p ); };
    void set_x_scale(float xs ) { x_scale.set( xs ); }
    void set_y_scale(float ys ) { x_scale.set( ys ); }

      

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class GMicProc
  {
  public: 
    void render(VipsRegion** ireg, int n, int in_first,
		VipsRegion* imap, VipsRegion* omap, 
		VipsRegion* oreg, OpParBase* par)
    {
			
    }
  };




  ProcessorBase* new_gmic();
}

#endif 


