#include <sstream>
#include "property.hh"


std::istream& operator >>(std::istream& str, PF::PropertyBase& p)
{
  p.set(str);
  return str;
}
std::ostream& operator <<(std::ostream& str, PF::PropertyBase& p)
{
  p.get(str);
  return str;
}




void PF::PropertyBase::set(const std::string& val)
{
  std::istringstream str(val);
  set(str);
}

std::string PF::PropertyBase::get()
{
  std::ostringstream str;
  get(str);
  return str.str();
}
