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

#include "raw_loader.hh"
#include "multiraw_developer.hh"


PF::MultiRawDeveloperPar::MultiRawDeveloperPar():
  OpParBase(),
	caching_enabled( true )
{
  set_type("multiraw_developer" );

  set_default_name( _("Multi-RAW developer") );
}


void PF::MultiRawDeveloperPar::add_image( std::string fname )
{
  PF::ProcessorBase* raw_loader = PF::new_raw_loader();
  g_assert(raw_loader != NULL); g_assert(raw_loader->get_par() != NULL);
  PF::RawLoaderPar* rlpar = dynamic_cast<PF::RawLoaderPar*>( raw_loader->get_par() );
  g_assert(rlpar != NULL);
  rlpar->set_file_name( fname );
  PF::ProcessorBase* raw_dev = PF::new_raw_developer();
  g_assert(raw_dev != NULL); g_assert(raw_dev->get_par() != NULL);

  developers.push_back( std::make_pair(raw_loader, raw_dev) );

  if( developers.size() == 1 )
    map_properties( raw_dev->get_par()->get_properties() );
}



VipsImage* PF::MultiRawDeveloperPar::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, 
				     unsigned int& level)
{
  VipsImage* out;
  std::vector<VipsImage*> rawvec;
  
  for( unsigned int i = 0; i < developers.size(); i++ ) {
    PF::OpParBase* par = developers[i].first->get_par();
    VipsImage* ri = par->build( in, 0, NULL, NULL, level );

    std::vector<VipsImage*> in2; in2.push_back( ri );
    PF::OpParBase* par2 = developers[i].second->get_par();
    par2->set_image_hints( ri );
    par2->set_format( VIPS_FORMAT_FLOAT );
    VipsImage* ri2 = par2->build( in2, 0, NULL, NULL, level );
    PF_UNREF( ri, "MultiRawDeveloperPar::build(): ri unref" );
    rawvec.push_back( ri2 );
  }

  if( rawvec.size() > 0 && rawvec[0] != NULL ) {
    set_image_hints( rawvec[0] );
    set_format( VIPS_FORMAT_FLOAT );
    out = OpParBase::build( rawvec, 0, NULL, NULL, level );
    for( unsigned int i = 0; i < rawvec.size(); i++ ) {
      PF_UNREF( rawvec[i], "MultiRawDeveloperPar::build(): rawvec[i] unref" );
    }
  } else {
    return NULL;
  }
  
  return out;
}


PF::ProcessorBase* PF::new_multiraw_developer()
{
  return new PF::Processor<PF::MultiRawDeveloperPar,PF::MultiRawDeveloper>();
}
