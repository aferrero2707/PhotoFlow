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

#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <map>
#include <glib-object.h>

namespace PF
{

  class OpParBase;

  class PropertyBase
  {
    std::string name;

    // Values for enum types:
    // first -> integer representation
    // second.first -> literal representation
    // second.second -> human-readable representation
    std::pair< int, std::pair<std::string,std::string> > enum_value;
    std::map< int, std::pair<std::string,std::string> > enum_values;

  public:
    PropertyBase(std::string n, OpParBase* par);//: name(n) {}
    PropertyBase(std::string n, OpParBase* par, int val, std::string strval, std::string valname);//: name(n) {}

    std::string get_name() { return name; }

    bool is_enum() { return( !enum_values.empty() ); }

    std::map< int, std::pair<std::string,std::string> > get_enum_values() { return enum_values; }
    std::pair< int, std::pair<std::string,std::string> > get_enum_value() { return enum_value; }
    void add_enum_value(int val, std::string valstr, std::string nickname)
    {
      std::map< int, std::pair<std::string,std::string> >::iterator i = 
				enum_values.find( val );
      if( i == enum_values.end() ) {
				enum_values.insert( make_pair( val, make_pair( valstr, nickname ) ) );
      } else {
				(*i).second.first = valstr;
				(*i).second.second = nickname;
      }
    }

    void set_enum_value(int val)
    {
      std::map< int, std::pair<std::string,std::string> >::iterator i = 
				enum_values.find( val );
      if( i != enum_values.end() ) {
				enum_value = (*i);
      }
    }

    virtual void set_str(const std::string& val);
    virtual std::string get_str();

    virtual void from_stream(std::istream& str);
    virtual void to_stream(std::ostream& str);

    virtual void set_gobject(gpointer object);

    void update(const std::string& val) { set_str( val ); }

    virtual bool import(PropertyBase* pin);

    template<typename T> void get(T& val)
    {
      std::istringstream istr( get_str() );
      istr>>val;
    }

    template<typename T> void update(const T& newval)
    {
      std::ostringstream ostr;
      ostr<<newval;
      //std::cout<<"PropertyBase::update(): newval="<<ostr.str()<<std::endl;
      //std::istringstream str( ostr.str() );
      //set_from_stream(str);
      set_str( ostr.str() );
    }

    /*
			template<typename T> void update(std::string str)
			{
      std::cout<<"PropertyBase::update(): newval="<<str<<std::endl;
      //std::istringstream str( ostr.str() );
      //set_from_stream(str);
      set_str( str );
			}
    */
  };

  std::istream& operator >>(std::istream& str, PropertyBase& p);
  std::ostream& operator <<(std::ostream& str, PropertyBase& p);


  template<typename T>
  void set_gobject_property(gpointer object, const std::string name, const T& value)
  {
    g_object_set( object, name.c_str(), value, NULL );
  }


  template<>
  void set_gobject_property<std::string>(gpointer object, const std::string name, const std::string& value);



  template< typename T >
  class Property: public PropertyBase
  {
    T value;
  public:
    Property(std::string name, OpParBase* par): PropertyBase(name, par), value() {}
    Property(std::string name, OpParBase* par, const T& v): PropertyBase(name, par), value(v) {}
    void set(const T& newval) { value = newval; }
    T& get() { return value; }
    void from_stream(std::istream& str)
    {
      str>>value;
    }
    void to_stream(std::ostream& str)
    {
      str<<value;
    }

    void set_gobject(gpointer object)
    {
      //g_object_set( object, get_name().c_str(), value, NULL );
      set_gobject_property( object, get_name(), value );
    }

    bool import(PropertyBase* pin)
    {
      if( is_enum() ) {
				return PropertyBase::import( pin );
      } else {
				Property<T>* pin2 = dynamic_cast< Property<T>* >( pin );
				if( pin2 ) {
					set( pin2->get() );
				} else {
					set_str( pin->get_str() );
				}
				return true;
      }
    }
	};


  template<>
  class Property<std::string>: public PropertyBase
  {
    std::string value;
  public:
    Property(std::string name, OpParBase* par): PropertyBase(name, par), value() {}
    Property(std::string name, OpParBase* par, const std::string& v): PropertyBase(name, par), value(v) {}
    void set(const std::string& newval) { value = newval; }
    std::string& get() { return value; }
    void from_stream(std::istream& str)
    {
			getline( str, value );
      //str>>value;
    }
    void to_stream(std::ostream& str)
    {
      str<<value;
    }

    void set_gobject(gpointer object)
    {
      //g_object_set( object, get_name().c_str(), value, NULL );
      set_gobject_property( object, get_name(), value );
    }

    bool import(PropertyBase* pin)
    {
      if( is_enum() ) {
				return PropertyBase::import( pin );
      } else {
				Property<std::string>* pin2 = dynamic_cast< Property<std::string>* >( pin );
				if( pin2 ) {
					set( pin2->get() );
				} else {
					set_str( pin->get_str() );
				}
				return true;
      }
    }
	};
}


#endif
