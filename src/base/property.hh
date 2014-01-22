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

  class PropertyBase
  {
    std::string name;
  public:
    PropertyBase(std::string n): name(n) {}

    std::string get_name() { return name; }

    virtual void set(const std::string& val);
    virtual std::string get();

    virtual void set(std::istream& str) = 0;
    virtual void get(std::ostream& str) = 0;
  };

  std::istream& operator >>(std::istream& str, PropertyBase& p);
  std::ostream& operator <<(std::ostream& str, PropertyBase& p);


  template< typename T >
  class Property: public PropertyBase
  {
    T* ptr;
  public:
    Property(std::string name, T* p): PropertyBase(name), ptr(p) {}
    void set(std::istream& str)
    {
      str>>(*ptr);
    }
    void get(std::ostream& str)
    {
      str<<(*ptr);
    }
  };

}


#endif
