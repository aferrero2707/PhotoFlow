#include <sstream>
#include "property.hh"
#include "operation.hh"


std::istream& operator >>(std::istream& str, PF::PropertyBase& p)
{
  p.set_str(str);
  return str;
}
std::ostream& operator <<(std::ostream& str, PF::PropertyBase& p)
{
  p.get_str(str);
  return str;
}


PF::PropertyBase::PropertyBase(std::string n, OpParBase* par): name(n) 
{
  //par->add_property(this);
}


void PF::PropertyBase::set_str(const std::string& val)
{
  std::istringstream str(val);
  set_str(str);
}

std::string PF::PropertyBase::get_str()
{
  std::ostringstream str;
  get_str(str);
  return str.str();
}
