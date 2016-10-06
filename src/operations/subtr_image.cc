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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../base/new_operation.hh"
#include "subtr_image.hh"


PF::SubtrImgPar::SubtrImgPar():
OpParBase(),
blendFactor("blendFactor",this,0.5f)
{
  subtrimg_algo = new PF::Processor<PF::SubtrImgAlgoPar,PF::SubtrImgAlgoProc>();

  set_type("subtrimg" );
  set_default_name( _("Subtract Image") );
}

VipsImage* PF::SubtrImgPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( (in.size()<2) || (in[0]==NULL)  || (in[1]==NULL) )
    return NULL;
//  VipsImage* srcimg = in[0];
  
  std::vector<VipsImage*> in2;

  SubtrImgAlgoPar* subtr_img_par = dynamic_cast<SubtrImgAlgoPar*>( subtrimg_algo->get_par() );

  subtr_img_par->set_blendFactor( get_blendFactor() );
  
  subtr_img_par->set_image_hints( in[0] );
  subtr_img_par->set_format( get_format() );
  in2.clear();
  in2.push_back( in[0] );
  in2.push_back( in[1] );
  VipsImage* subtrimg = subtr_img_par->build( in2, 0, NULL, NULL, level );
  
  set_image_hints( subtrimg );
  
  return subtrimg;

}

PF::ProcessorBase* PF::new_subtrimg()
{
  return new PF::Processor<PF::SubtrImgPar,PF::SubtrImgProc>();
}

