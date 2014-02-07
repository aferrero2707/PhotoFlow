#include <sstream>
#include "property.hh"
#include "operation.hh"


std::istream& operator >>(std::istream& str, PF::PropertyBase& p)
{
  p.set_from_stream(str);
  return str;
}
std::ostream& operator <<(std::ostream& str, PF::PropertyBase& p)
{
  p.get_from_stream(str);
  return str;
}


PF::PropertyBase::PropertyBase(std::string n, OpParBase* par): name(n) 
{
  par->add_property(this);
}


PF::PropertyBase::PropertyBase(std::string n, OpParBase* par, 
			       int val, std::string strval, std::string valname): name(n) 
{
  par->add_property(this);
  set_enum_value( val, strval, valname );
  enum_value.first = val;
  enum_value.second.first = strval;
  enum_value.second.second = valname;
}


void PF::PropertyBase::set_str(const std::string& val)
{
  std::istringstream str(val);
  set_from_stream(str);
}

std::string PF::PropertyBase::get_str()
{
  std::ostringstream str;
  get_from_stream(str);
  return str.str();
}


void PF::PropertyBase::set_from_stream(std::istream& str)
{
  if( !is_enum() ) return;
  std::string s;
  str>>s;
  std::map< int, std::pair<std::string,std::string> >::iterator iter;
  for( iter = enum_values.begin(); iter != enum_values.end(); iter++ ) {
    if( (*iter).second.first != s ) 
      continue;
    enum_value = (*iter);
    break;
  }
}


void PF::PropertyBase::get_from_stream(std::ostream& str)
{
  if( !is_enum() ) return;
  if( enum_value.second.first.empty() ) return;
  str<<enum_value.second.first;
}

 

void PF::PropertyBase::set_gobject(gpointer object)
{
  if( !is_enum() ) return;
  if( enum_value.second.first.empty() ) return;
  g_object_set( object, get_name().c_str(), enum_value.first, NULL );
}

