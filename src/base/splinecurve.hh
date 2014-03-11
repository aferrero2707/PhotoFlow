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


#ifndef SPLINE_CURVE_H
#define SPLINE_CURVE_H

#include "property.hh"
#include "curve.hh"


namespace PF
{

  class SplineCurve: public Curve
  {
    std::vector< std::pair<float,float> > points;

    double* ypp;

  public:
    SplineCurve();
    ~SplineCurve();

    int add_point( float x, float y );

    bool remove_point( unsigned int id );

    void clear_points() { points.clear(); }

    bool set_point( unsigned int id, float& x, float& y );

    std::vector< std::pair<float,float> > get_points() { return points; }

    void update_spline();

    // Get the output value corresponding to an input value x (normalized to the [0,1] range)
    float get_value( float x );

    // Fill a vector of equally-spaced points with input-output value pairs
    void get_values( std::vector< std::pair<float,float> >& vec );

    // Fill a vector of equally-spaced points with input-output value deltas
    void get_deltas( std::vector< std::pair<float,float> >& vec );
  };



  //template<>
  //void set_gobject_property<SplineCurve>(gpointer object, const std::string name, const std::string& value);

  template<>
  class Property<SplineCurve>: public PropertyBase
  {
    SplineCurve curve;
  public:
    Property(std::string name, OpParBase* par): PropertyBase(name, par), curve() {}
    //void set(const SplineCurve& newval) { value = newval; }
    SplineCurve& get() { return curve; }
    void from_stream(std::istream& str)
    {
      int npoints;
      str>>npoints;
      if( npoints > 0 ) curve.clear_points();
      for( int i = 0; i < npoints; i++ ) {
	float x, y;
	str>>x>>y;
	curve.add_point( x, y );
      }
      //str>>value;
    }
    void to_stream(std::ostream& str)
    {
      str<<curve.get_points().size();
      for( unsigned int i = 0; i < curve.get_points().size(); i++ )
	str<<" "<<curve.get_points()[i].first<<" "<<curve.get_points()[i].second;
      //str<<value;
    }

    void set_gobject(gpointer object)
    {
      //g_object_set( object, get_name().c_str(), value, NULL );
      //set_gobject_property( object, get_name(), value );
    }
   };
}


#endif
