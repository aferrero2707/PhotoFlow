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

#ifndef PF_PROPERTY_H
#define PF_PROPERTY_H


namespace PF
{

  class OpParBase;

  class PropertyBase
  {
    std::string name;
  public:
    PropertyBase(std::string n, OpParBase* par);//: name(n) {}

    std::string get_name() { return name; }

    virtual void set_str(const std::string& val);
    virtual std::string get_str();

    virtual void set_str(std::istream& str) = 0;
    virtual void get_str(std::ostream& str) = 0;
  };

  std::istream& operator >>(std::istream& str, PropertyBase& p);
  std::ostream& operator <<(std::ostream& str, PropertyBase& p);


  template< typename T >
  class Property: public PropertyBase
  {
    T value;
  public:
    Property(std::string name, OpParBase* par): PropertyBase(name, par), value() {}
    Property(std::string name, OpParBase* par, const T& v): PropertyBase(name, par), value(v) {}
    void set(const T& newval) { value = newval; }
    T& get() { return value; }
    void set_str(std::istream& str)
    {
      str>>value;
    }
    void get_str(std::ostream& str)
    {
      str<<value;
    }
  };

}


#endif
