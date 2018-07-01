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

#ifndef PF_SHARPEN_H
#define PF_SHARPEN_H


namespace PF 
{

  enum sharpen_method_t
  {
    SHARPEN_USM,
    SHARPEN_DECONV,
    SHARPEN_TEXTURE
  };

  class SharpenPar: public OpParBase
  {
    PropertyBase method;
    Property<float> usm_radius;
    Property<float> rl_sigma;
    Property<int> rl_iterations;
    Property<float> texture_radius;
    Property<float> texture_strength;
    ProcessorBase* usm;
    ProcessorBase* rl;
    ProcessorBase* texture;
  public:
    SharpenPar();
    ~SharpenPar();

    bool has_intensity() { return false; }
    bool needs_caching();
    bool has_target_channel() { return true; }

    void set_usm_radius(float val) { usm_radius.update(val); }
      
    void compute_padding( VipsImage* full_res, unsigned int id, unsigned int level );
    void propagate_settings();

    VipsImage* build(std::vector<VipsImage*>& in, int first, 
		     VipsImage* imap, VipsImage* omap, 
		     unsigned int& level);
  };

  

  template < OP_TEMPLATE_DEF > 
  class SharpenProc
  {
  public: 
    void render(VipsRegion** in, int n, int in_first,
								VipsRegion* imap, VipsRegion* omap, 
								VipsRegion* out, OpParBase* par) 
    {
    }
  };



  ProcessorBase* new_sharpen();

}

#endif 


