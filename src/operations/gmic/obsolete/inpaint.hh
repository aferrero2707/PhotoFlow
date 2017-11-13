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

#ifndef PF_GMIC_INPAINT_H
#define PF_GMIC_INPAINT_H

#include <iostream>

#include "../../base/format_info.hh"
#include "../../base/operation.hh"
#include "../../base/processor.hh"
#include "../../base/rawbuffer.hh"
#include "../../base/rawbuffer.hh"

#include "../draw.hh"
#include "../uniform.hh"
#include "gmic_untiled_op.hh"


namespace PF 
{

  enum gmic_inpaint_display_mode_t {
    GMIC_INPAINT_DRAW_MASK,
    GMIC_INPAINT_DRAW_OUTPUT,
  };

  class GmicInpaintPar: public GmicUntiledOperationPar
  {
    Property<int> patch_size;
    Property<float> lookup_size;
    Property<float> lookup_factor;
    Property<float> blend_size;
    Property<float> blend_threshold;
    Property<float> blend_decay;
    Property<int> blend_scales;
    Property<int> allow_outer_blending;
    Property<int> pen_size;
    Property< std::list< Stroke<Pencil> > > strokes;
    PropertyBase display_mode;

    Processor<PF::DrawPar,PF::DrawProc>* draw_op1;
    Processor<PF::DrawPar,PF::DrawProc>* draw_op2;
    PF::Processor<PF::BlenderPar,PF::BlenderProc>* maskblend;
    Processor<PF::UniformPar,PF::Uniform>* black;
    Processor<PF::UniformPar,PF::Uniform>* uniform;
    ProcessorBase* invert;

    unsigned int scale_factor;

    Pencil pen;

  public:
    GmicInpaintPar();
    ~GmicInpaintPar();

    bool has_intensity() { return false; }
    bool needs_input() { return false; }

    Pencil& get_pen() { return pen; }

    unsigned int get_scale_factor() { return scale_factor; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);

    void start_stroke()
    {
      start_stroke( pen_size.get() );
    }
    void start_stroke( unsigned int pen_size );
    void end_stroke();

    Property< std::list< Stroke<Pencil> > >& get_strokes() { return strokes; }

    void draw_point( unsigned int x, unsigned int y, VipsRect& update );
  };

  

  template < OP_TEMPLATE_DEF > 
  class GmicInpaintProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap, 
                VipsRegion* oreg, GmicInpaintPar* par)
    {
    }
  };


  ProcessorBase* new_gmic_inpaint();
}

#endif 


