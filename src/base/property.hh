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
#include <list>
#include <vector>
#include <map>
#include <glib-object.h>
#include <sigc++/sigc++.h>

namespace PF
{

  class OpParBase;


  /*
  template<class T1, class T2>
  bool operator==(const std::pair<T1,T2>& lhs, const std::pair<T1,T2>& rhs)
  {
    if( lhs.first != rhs.first ) return false;
    if( lhs.second != rhs.second ) return false;
    return true;
  }

  template<class T1, class T2>
  bool operator!=(const std::pair<T1,T2>& lhs, const std::pair<T1,T2>& rhs)
  {
    return( !(lhs == rhs) );
  }
  */


  template<class T1, class T2>
  std::istream& operator >>( std::istream& str, std::pair<T1,T2>& pair )
  {
    str>>pair.first>>pair.second;
    return str;
  }

  template<class T1, class T2>
  std::ostream& operator <<( std::ostream& str, const std::pair<T1,T2>& pair )
  {
    str<<pair.first<<" "<<pair.second<<" ";
    return str;
  }



  template<class T>
  bool operator==(const std::list<T>& lhs, const std::list<T>& rhs)
  {
    if( lhs.size() != rhs.size() ) return false;
    for( typename std::list<T>::const_iterator i1=lhs.begin(), i2=rhs.begin();
         i1!=lhs.end() && i2!=rhs.end(); i1++,i2++ ) {
      if( (*i1) != (*i2) ) return false;
    }
    return true;
  }

  template<class T>
  bool operator!=(const std::list<T>& lhs, const std::list<T>& rhs)
  {
    return( !(lhs == rhs) );
  }



  template<class T>
  std::istream& operator >>( std::istream& str, std::list<T>& list )
  {
    list.clear();
    int nelt;
    str>>nelt;
    for( int i = 0; i < nelt; i++ ) {
      list.push_back( T() );
      T& val = list.back();
      str>>val;
    }
    return str;
  }

  template<class T>
  std::ostream& operator <<( std::ostream& str, const std::list<T>& list )
  {
    str<<list.size()<<" ";
    typename std::list<T>::const_iterator i;
    for( i = list.begin(); i != list.end(); i++ ) {
      str<<(*i);
    }
    return str;
  }


  template<class T>
  bool operator==(const std::vector<T>& lhs, const std::vector<T>& rhs)
  {
    if( lhs.size() != rhs.size() ) return false;
    for( typename std::vector<T>::const_iterator i1=lhs.begin(), i2=rhs.begin();
         i1!=lhs.end() && i2!=rhs.end(); i1++,i2++ ) {
      if( (*i1) != (*i2) ) return false;
    }
    return true;
  }

  template<class T>
  bool operator!=(const std::vector<T>& lhs, const std::vector<T>& rhs)
  {
    return( !(lhs == rhs) );
  }


  template<class T>
  std::istream& operator >>( std::istream& str, std::vector<T>& vector )
  {
    vector.clear();
    int nelt;
    str>>nelt;
    for( int i = 0; i < nelt; i++ ) {
      vector.push_back( T() );
      T& val = vector.back();
      str>>val;
    }
    return str;
  }

  template<class T>
  std::ostream& operator <<( std::ostream& str, const std::vector<T>& vector )
  {
    str<<vector.size()<<" ";
    typename std::vector<T>::const_iterator i;
    for( i = vector.begin(); i != vector.end(); i++ ) {
      str<<(*i);
    }
    return str;
  }



  class PropertyBase: public sigc::trackable
  {
    std::string name;

    // Values for enum types:
    // first -> integer representation
    // second.first -> literal representation
    // second.second -> human-readable representation
    std::pair< int, std::pair<std::string,std::string> > default_enum_value;
    std::pair< int, std::pair<std::string,std::string> > enum_value;
    std::map< int, std::pair<std::string,std::string> > enum_values;

    bool internal;

    bool modified_flag;

  public:
    sigc::signal<void> signal_modified;

    PropertyBase(std::string n, OpParBase* par);//: name(n) {}
    PropertyBase(std::string n, OpParBase* par, int val, std::string strval, std::string valname);//: name(n) {}

    std::string get_name() { return name; }

    virtual void reset()
    {
      if( is_enum() ) set_enum_value( default_enum_value.first );
    }

    virtual void store_default()
    {
      if( is_enum() ) default_enum_value = enum_value;
    }

    bool is_enum() { return( !enum_values.empty() ); }

    bool is_internal() { return internal; }
    void set_internal(bool i) { internal = i; }

    bool is_modified() { return modified_flag; }
    void set_modified() { modified_flag = true; }
    void clear_modified() { modified_flag = false; }

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
#ifndef NDEBUG
      std::cout<<"Property \""<<name<<"\": setting enum value to "<<val<<std::endl;
#endif
      std::map< int, std::pair<std::string,std::string> >::iterator i = 
				enum_values.find( val );
      if( i != enum_values.end() ) {
        if(enum_value.first != (*i).first)
          modified();
				enum_value = (*i);
#ifndef NDEBUG
        std::cout<<"... done (\""<<enum_value.second.first<<"\")."<<std::endl;
#endif
      }
    }

    virtual void set_str(const std::string& val);
    virtual std::string get_str();
    std::string get_enum_value_str();

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

    virtual void modified() { set_modified(); signal_modified.emit(); }
  };

  std::istream& operator >>(std::istream& str, PropertyBase& p);
  std::ostream& operator <<(std::ostream& str, PropertyBase& p);


  template<typename T>
  void set_gobject_property(gpointer object, const std::string name, const T& value)
  {
    g_object_set( object, name.c_str(), value, NULL );
  }


  template<typename T>
  void set_gobject_property(gpointer /*object*/, const std::string /*name*/, const std::list<T>& /*value*/)
  {
    //g_object_set( object, name.c_str(), value, NULL );
  }


  template<typename T>
  void set_gobject_property(gpointer /*object*/, const std::string /*name*/, const std::vector<T>& /*value*/)
  {
    //g_object_set( object, name.c_str(), value, NULL );
  }


  template<>
  void set_gobject_property<std::string>(gpointer object, const std::string name, const std::string& value);



  template< typename T >
  class Property: public PropertyBase
  {
    T default_value;
    T value;
  public:
    Property(std::string name, OpParBase* par): PropertyBase(name, par), value(), default_value() {}
    Property(std::string name, OpParBase* par, const T& v): PropertyBase(name, par), value(v), default_value(v) {}

    void reset() { set(default_value); }

    void store_default() { default_value = value;}

    void store_default(const T& newval) { default_value = newval;}

    void set(const T& newval) 
    { 
      if( value != newval )
        modified();
      value = newval; 
    }
    T& get() { return value; }
    void from_stream(std::istream& str)
    {
      T oldvalue = value;
      str>>value;
      if( value != oldvalue )
        modified();
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
    void set(const std::string& newval) 
    { 
      if( value != newval )
        modified();
      value = newval; 
    }
    std::string& get() { return value; }
    void from_stream(std::istream& str)
    {
      std::string old = value;
			getline( str, value );
      //std::cout<<"Property<std::string>::from_stream() called: new value = "<<value<<std::endl;
      if( value != old )
        modified();
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

  template <class T> std::string to_string( const T& val )
  {
	std::ostringstream str;
	str << std::dec << val;
	return str.str();
  }
}


#endif
