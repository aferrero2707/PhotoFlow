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
#include "layer.hh"
//#include "../vips/vips_layer.h"

int
vips_layer( int n, VipsImage **out, 
            PF::ProcessorBase* proc,
            VipsImage* imap, VipsImage* omap, 
            VipsDemandStyle demand_hint,
	    int width, int height, int nbands, ... );





void PF::OperationConfigUI::open()
{
  get_layer()->get_processor()->get_par()->save_properties(initial_params);
}




PF::OpParBase::OpParBase():
	render_mode(PF_RENDER_PREVIEW), 
  map_flag( false ),
  editing_flag( false ),
  modified_flag(false),
  intensity("intensity",this,1),
  grey_target_channel("grey_target_channel",this,-1,"Grey","Grey"),
  rgb_target_channel("rgb_target_channel",this,-1,"RGB","RGB"),
  lab_target_channel("lab_target_channel",this,-1,"Lab","Lab"),
  cmyk_target_channel("cmyk_target_channel",this,-1,"CMYK","CMYK")
{
  //blend_mode.set_internal(true);
  intensity.set_internal(true);
  //opacity.set_internal(true);
  grey_target_channel.set_internal(true);
  rgb_target_channel.set_internal(true);
  lab_target_channel.set_internal(true);
  cmyk_target_channel.set_internal(true);
  
  processor = NULL;
  //out = NULL;
  config_ui = NULL;
  //blend_mode = PF_BLEND_PASSTHROUGH;
  //blend_mode = PF_BLEND_NORMAL;
  //blend_mode.set_enum_value( PF_BLEND_PASSTHROUGH );
  demand_hint = VIPS_DEMAND_STYLE_THINSTRIP;
  bands = 1;
  xsize = 100; ysize = 100;

  rgb_target_channel.add_enum_value(0,"R","R");
  rgb_target_channel.add_enum_value(1,"G","G");
  rgb_target_channel.add_enum_value(2,"B","B");

  lab_target_channel.add_enum_value(0,"L","L");
  lab_target_channel.add_enum_value(1,"a","a");
  lab_target_channel.add_enum_value(2,"b","b");

  cmyk_target_channel.add_enum_value(0,"C","C");
  cmyk_target_channel.add_enum_value(1,"M","M");
  cmyk_target_channel.add_enum_value(2,"Y","Y");
  cmyk_target_channel.add_enum_value(3,"K","K");

  //PF::PropertyBase* prop;
  //prop = new PF::Property<float>("intensity",&intensity);
}


PF::PropertyBase* PF::OpParBase::get_property(std::string name)
{
  std::list<PropertyBase*>::iterator pi;

  // Look into mapped properties first
  for(pi = mapped_properties.begin(); pi != mapped_properties.end(); pi++) {
    //std::cout<<"(*pi)->get_name(): "<<(*pi)->get_name()<<"    name: "<<name<<std::endl;
    if( (*pi)->get_name() == name ) return( *pi );
  }
  
  // If nothing is found, look into our own properties
  for(pi = properties.begin(); pi != properties.end(); pi++) {
    //std::cout<<"(*pi)->get_name(): "<<(*pi)->get_name()<<"    name: "<<name<<std::endl;
    if( (*pi)->get_name() == name ) return( *pi );
  }
  return NULL;
}


void PF::OpParBase::save_properties(std::list<std::string>& plist)
{
  std::list<PropertyBase*>::iterator pi;
  for(pi = mapped_properties.begin(); pi != mapped_properties.end(); pi++) {
    std::string str = (*pi)->get_str();
    plist.push_back(str);
  }
  for(pi = properties.begin(); pi != properties.end(); pi++) {
    std::string str = (*pi)->get_str();
    plist.push_back(str);
  }
}


void PF::OpParBase::restore_properties(const std::list<std::string>& plist)
{
  std::list<PropertyBase*>::iterator pi;
  std::list<std::string>::const_iterator si;
  for(pi = mapped_properties.begin(), si = plist.begin(); 
      (pi != mapped_properties.end()) && (si != plist.end()); 
      pi++, si++) {
    (*pi)->set_str(*si);
  }
  for(pi = properties.begin(); 
      (pi != properties.end()) && (si != plist.end()); 
      pi++, si++) {
    (*pi)->set_str(*si);
  }
}


void PF::OpParBase::clear_modified() 
{ 
  modified_flag = false; 
  std::list<PropertyBase*>::iterator pi;
  for(pi = mapped_properties.begin();
      pi != mapped_properties.end(); 
      pi++) {
    (*pi)->clear_modified();
  }
  for(pi = properties.begin(); 
      pi != properties.end(); 
      pi++) {
    (*pi)->clear_modified();
  }
}

void PF::OpParBase::set_image_hints(int w, int h, VipsInterpretation interpr)
{
  xsize = w;
  ysize = h;
  coding = VIPS_CODING_NONE;
  interpretation = interpr;
}



void PF::OpParBase::set_image_hints(int w, int h, colorspace_t cs)
{
  xsize = w;
  ysize = h;
  coding = VIPS_CODING_NONE;
  switch(cs) {
  case PF_COLORSPACE_GRAYSCALE:
    interpretation = VIPS_INTERPRETATION_B_W; break;
  case PF_COLORSPACE_RGB:
    interpretation = VIPS_INTERPRETATION_RGB; break;
  case PF_COLORSPACE_LAB:
    interpretation = VIPS_INTERPRETATION_LAB; break;
  case PF_COLORSPACE_CMYK:
    interpretation = VIPS_INTERPRETATION_CMYK; break;
  default:
    interpretation = VIPS_INTERPRETATION_MULTIBAND; break;
  }
}


bool PF::OpParBase::import_settings( OpParBase* pin )
{
  if( !pin )
    return false;

  std::list<PropertyBase*>& propin = pin->get_properties();
  std::list<PropertyBase*>::iterator pi=propin.begin(), pj=properties.begin();
  for( ; pi != propin.end(); pi++, pj++ ) {
    if( !(*pj)->import( *pi ) )
      return false;
  }

  std::list<PropertyBase*>& mpropin = pin->get_mapped_properties();
  std::list<PropertyBase*>::iterator mpi=mpropin.begin(), mpj=mapped_properties.begin();
  for( ; mpi != mpropin.end(); mpi++, mpj++ ) {
    if( !(*mpj)->import( *mpi ) )
      return false;
  }

  set_demand_hint( pin->get_demand_hint() );
  set_map_flag( pin->is_map() );
  set_editing_flag( pin->is_editing() );
  set_image_hints( pin->get_xsize(), pin->get_ysize(), 
		   pin->get_interpretation() );
  set_nbands( pin->get_nbands() );
  set_coding( pin->get_coding() );
  set_format( pin->get_format() );
  return true;
}


VipsImage* PF::OpParBase::build(std::vector<VipsImage*>& in, int first, 
				VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  VipsImage* outnew = NULL;
  VipsImage* invec[100];
  int n = 0;
  for(int i = 0; i < in.size(); i++) {
    if( !in[i] ) continue;
    invec[n] = in[i];
    n++;
  }
  if(n > 100) n = 100;
  switch( n ) {
  case 0:
    vips_layer( n, &outnew, processor, imap, omap, 
		get_demand_hint(), get_xsize(), get_ysize(), get_nbands(),
		NULL );
    break;
  case 1:
    vips_layer( n, &outnew, processor, imap, omap, 
		get_demand_hint(), get_xsize(), get_ysize(), get_nbands(),
		"in0", invec[0], NULL );
    break;
  case 2:
    vips_layer( n, &outnew, processor, imap, omap, 
		get_demand_hint(), get_xsize(), get_ysize(), get_nbands(),
		"in0", invec[0], "in1", invec[1], NULL );
    break;
  default:
    break;
  }

#ifndef NDEBUG    
  std::cout<<"OpParBase::build(): type="<<type<<"  format="<<get_format()<<std::endl
	   <<"input images:"<<std::endl;
  for(int i = 0; i < n; i++) {
    std::cout<<"  "<<(void*)invec[i]<<"   ref_count="<<G_OBJECT( invec[i] )->ref_count<<std::endl;
  }
  std::cout<<"imap: "<<(void*)imap<<std::endl<<"omap: "<<(void*)omap<<std::endl;
  std::cout<<"out: "<<(void*)outnew<<std::endl<<std::endl;
#endif

  //set_image( outnew );
#ifndef NDEBUG    
  std::cout<<"OpParBase::build(): outnew refcount ("<<(void*)outnew<<") = "<<G_OBJECT(outnew)->ref_count<<std::endl;
#endif
  return outnew;
}



bool PF::OpParBase::save( std::ostream& ostr, int level )
{
  for(int i = 0; i < level; i++) ostr<<"  ";
  ostr<<"<operation type=\""<<get_type()<<"\">"<<std::endl;

  for( std::list<PropertyBase*>::iterator pi = properties.begin();
       pi != properties.end(); pi++ ) {
    for(int i = 0; i < level+1; i++) ostr<<"  ";
    ostr<<"<property name=\""<<(*pi)->get_name()<<"\" value=\"";
    (*pi)->to_stream( ostr );
    ostr<<"\">"<<std::endl;
    for(int i = 0; i < level+1; i++) ostr<<"  ";
    ostr<<"</property>"<<std::endl;
  }
  
  for( std::list<PropertyBase*>::iterator pi = mapped_properties.begin();
       pi != mapped_properties.end(); pi++ ) {
    for(int i = 0; i < level+1; i++) ostr<<"  ";
    ostr<<"<property name=\""<<(*pi)->get_name()<<"\" value=\"";
    (*pi)->to_stream( ostr );
    ostr<<"\">"<<std::endl;
    for(int i = 0; i < level+1; i++) ostr<<"  ";
    ostr<<"</property>"<<std::endl;
  }
  
  for(int i = 0; i < level; i++) ostr<<"  ";
  ostr<<"</operation>"<<std::endl;

  return true;
}
