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

#ifndef VIPS_CLONE_H
#define VIPS_CLONE_H

#include <string>

#include "../base/processor.hh"

#include "blender.hh"

namespace PF 
{

  enum clone_channel {
    CLONE_CHANNEL_GREY,
    CLONE_CHANNEL_RGB,
    CLONE_CHANNEL_R,
    CLONE_CHANNEL_G,
    CLONE_CHANNEL_B,
    CLONE_CHANNEL_Lab,
    CLONE_CHANNEL_L,
    CLONE_CHANNEL_a,
    CLONE_CHANNEL_b,
    CLONE_CHANNEL_CMYK,
    CLONE_CHANNEL_C,
    CLONE_CHANNEL_M,
    CLONE_CHANNEL_Y,
    CLONE_CHANNEL_K
  }; 

  class ClonePar: public BlenderPar
  {
    PropertyBase source_channel;

    ProcessorBase* convert_format;
    ProcessorBase* convert2lab;
    ProcessorBase* convert_cs;
    ProcessorBase* desaturate;

    VipsImage* Lab2grayscale(VipsImage* in, clone_channel ch, unsigned int& level);
    VipsImage* rgb2grayscale(VipsImage* in, clone_channel ch, unsigned int& level);
    VipsImage* rgb2rgb(VipsImage* in, clone_channel ch, unsigned int& level);
    VipsImage* Lab2rgb(VipsImage* in, clone_channel ch, unsigned int& level);
    VipsImage* L2rgb(VipsImage* in, unsigned int& level);
    VipsImage* ab2rgb(VipsImage* in, clone_channel ch, unsigned int& level);

  public:
    ClonePar();

    /* Set processing hints:
       1. the intensity parameter makes no sense for an image, 
          creation of an intensity map is not allowed
     */
    bool has_intensity() { return false; }
    bool needs_input() { return false; }

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class CloneProc: public BlenderProc<OP_TEMPLATE_IMP>
  {
  };

  ProcessorBase* new_clone();
}

#endif 


