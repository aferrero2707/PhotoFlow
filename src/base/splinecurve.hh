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

#define SPLINE_USE_STDVEC 1


namespace PF
{

class SplineCurve: public Curve
{
#ifdef SPLINE_USE_STDVEC
  std::vector< std::pair<float,float> > points;
  std::vector< std::pair<float,float> > points2;
#else
  std::pair<float,float>* points;
  size_t npoints;
#endif

  GMutex* points_mutex;

  double* ypp;
  unsigned int ypp_size;

  bool circular;

public:
  SplineCurve();
  ~SplineCurve();

  void lock() { g_mutex_lock( points_mutex); }
  void unlock() { g_mutex_unlock( points_mutex); }

  bool is_circular() const { return circular; }
  void set_circular( bool c ) { circular = c; }

  int add_point( float x, float y );

  bool remove_point( unsigned int id );

  void clear_points() {
#ifdef SPLINE_USE_STDVEC
    points.clear();
#else
    delete[] points;
    points = NULL;
    npoints = 0;
#endif
  }

  bool set_point( unsigned int id, float& x, float& y );

#ifdef SPLINE_USE_STDVEC
  const std::vector< std::pair<float,float> >& get_points() const { return points; }
  size_t get_npoints() const { return points.size(); }
#else
  const std::pair<float,float>* get_points() const { return points; }
  size_t get_npoints() const { return npoints; }
#endif
  std::pair<float,float> get_point(int n) const { return points[n]; }

  void update_spline();

  // Get the output value corresponding to an input value x (normalized to the [0,1] range)
  float get_value( float x );

  // Get the output delta corresponding to an input value x (normalized to the [0,1] range)
  float get_delta( float x );

  // Fill a vector of equally-spaced points with input-output value pairs
  void get_values( std::vector< std::pair<float,float> >& vec );

  // Fill a vector of equally-spaced points with input-output value deltas
  void get_deltas( std::vector< std::pair<float,float> >& vec );
  SplineCurve& operator=(const SplineCurve& b)
  {
    lock();
    set_circular( b.is_circular() );
#ifdef SPLINE_USE_STDVEC
    points = b.get_points();
#else
    if( points ) delete[] points;
    npoints = b.get_npoints();
    points = new std::pair<float,float>[npoints];
    for(size_t i = 0; i < npoints; i++)
      points[i] = b.get_point(i);
#endif
    update_spline();
    unlock();
    return *this;
  }
};


inline bool operator ==(const SplineCurve& l, const SplineCurve& r)
      {
#ifdef SPLINE_USE_STDVEC
  if( l.get_points() != r.get_points() ) return false;
#else
  if( l.get_npoints() != r.get_npoints() ) return false;
  for( size_t i = 0; i < l.get_npoints(); i++ ) {
    if( l.get_point(i) != r.get_point(i) ) return false;
  }
#endif
  return true;
      }

inline bool operator !=(const SplineCurve& l, const SplineCurve& r)
      {
  return( !(l==r) );
      }



//template<>
//void set_gobject_property<SplineCurve>(gpointer object, const std::string name, const std::string& value);

template<>
class Property<SplineCurve>: public PropertyBase
{
  SplineCurve curve;
  SplineCurve default_curve;
public:
  Property(std::string name, OpParBase* par): PropertyBase(name, par), curve(), default_curve() {}

  void reset() { set(default_curve); }

  void store_default() { default_curve = curve;}

  void set(const SplineCurve& newval) {
    if( curve != newval )
      modified();
    curve = newval;
  }

  SplineCurve& get() { return curve; }

  void from_stream(std::istream& str)
  {
    //std::cout<<"Property<SplineCurve>::from_stream() called (\""<<get_name()<<"\")"<<std::endl;
    SplineCurve oldcurve = curve;
    int npoints;
    str>>npoints;
    //std::cout<<"  # of points: "<<npoints<<std::endl;
    if( npoints > 0 ) curve.clear_points();
    for( int i = 0; i < npoints; i++ ) {
      float x, y;
      str>>x>>y;
      //std::cout<<"  point #"<<i<<": "<<x<<","<<y<<std::endl;
      curve.add_point( x, y );
    }
    if( oldcurve != curve )
      modified();
    //str>>value;
  }

  void to_stream(std::ostream& str)
  {
    //std::vector<std::pair<float, float> > points = curve.get_points();
    str<<curve.get_npoints();
    for( unsigned int i = 0; i < curve.get_npoints(); i++ )
      str<<" "<<curve.get_point(i).first<<" "<<curve.get_point(i).second;
    //str<<value;
  }

  bool import(PropertyBase* pin)
  {
    //std::cout<<"Property<SplineCurve>::import() called (\""<<get_name()<<"\")"<<std::endl;
    Property<SplineCurve>* pin2 = dynamic_cast< Property<SplineCurve>* >( pin );
    if( pin2 ) {
      //std::cout<<"  gen_npoints()="<<curve.get_npoints()<<"  pin2->get_npoints()="<<pin2->get().get_npoints()<<std::endl;
      set( pin2->get() );
    } else {
      set_str( pin->get_str() );
    }
    return true;
  }

  void set_gobject(gpointer object)
  {
    //g_object_set( object, get_name().c_str(), value, NULL );
    //set_gobject_property( object, get_name(), value );
  }
};





class ClosedSplineCurve: public Curve
{
  std::vector< std::pair<float,float> > points;
  std::vector< std::pair<float,float> > points_polar;
  std::vector< std::pair<int,int> > outline;
  std::vector< std::pair<int,int> > border;
  std::vector< std::pair< std::pair<float,float>,std::pair<float,float> > > ctrl_points;

  std::pair<float,float> center;
  float Dmax;
  float border_size;

  int wd_last, ht_last;

  GMutex* points_mutex;

  double* ypp;
  unsigned int ypp_size;

  void update_polar();

public:
  ClosedSplineCurve();
  ~ClosedSplineCurve();

  void lock() { g_mutex_lock( points_mutex); }
  void unlock() { g_mutex_unlock( points_mutex); }

  float get_border_size() { return border_size; }
  void set_border_size( float sz ) { border_size = sz; }

  int add_point( int id, float x, float y );

  bool remove_point( unsigned int id );

  void clear_points() {
    points.clear();
  }

  int set_point( unsigned int id, float x, float y );

  void scale( float s );

  const std::vector< std::pair<float,float> >& get_points() const { return points; }
  const std::vector< std::pair<float,float> >& get_points_polar() const { return points_polar; }
  const std::vector< std::pair<int,int> >& get_outline() const { return outline; }
  const std::vector< std::pair<int,int> >& get_border() const { return border; }
  size_t get_npoints() const { return points.size(); }
  std::pair<float,float> get_point(int n) const { return points[n]; }
  //void set_center(float x, float y) { center.first = x; center.second = y; }
  std::pair<float,float> get_center() const { return center; }
  float get_Dmax() { return Dmax; }
  std::pair<float,float> get_outline_scaling( float w, float h ) {
    std::pair<float,float> result;
    result.first = 1; //w*Dmax/1000.0f;
    result.second = 1; //h*Dmax/1000.0f;
    return result;
  }

  void update_spline();
  void update_outline( float wd, float ht );
  int get_wd_last() { return wd_last; }
  int get_ht_last() { return ht_last; }

  void update_center();

  // Get the output value corresponding to an input value x (normalized to the [0,1] range)
  float get_value( float x );

  // Get the output delta corresponding to an input value x (normalized to the [0,1] range)
  float get_delta( float x );

  // Fill a vector of equally-spaced points with input-output value pairs
  void get_values( std::vector< std::pair<float,float> >& vec );

  // Fill a vector of equally-spaced points with input-output value deltas
  void get_deltas( std::vector< std::pair<float,float> >& vec );

  ClosedSplineCurve& operator=(const ClosedSplineCurve& b)
  {
    lock();
    points = b.get_points();
    points_polar = b.get_points_polar();
    center = b.get_center();
    update_spline();
    unlock();
    return *this;
  }
};


inline
bool operator ==(const ClosedSplineCurve& l, const ClosedSplineCurve& r)
{
  if( l.get_points() != r.get_points() ) return false;
  return true;
}

inline
bool operator !=(const ClosedSplineCurve& l, const ClosedSplineCurve& r)
{
  return( !(l==r) );
}



//template<>
//void set_gobject_property<ClosedSplineCurve>(gpointer object, const std::string name, const std::string& value);

template<>
class Property<ClosedSplineCurve>: public PropertyBase
{
  ClosedSplineCurve curve;
  ClosedSplineCurve default_curve;
public:
  Property(std::string name, OpParBase* par): PropertyBase(name, par), curve(), default_curve() {}

  void reset() { set(default_curve); }

  void store_default() { default_curve = curve;}

  void set(const ClosedSplineCurve& newval) {
    if( curve != newval )
      modified();
    curve = newval;
  }

  ClosedSplineCurve& get() { return curve; }

  void from_stream(std::istream& str)
  {
    //std::cout<<"Property<ClosedSplineCurve>::from_stream() called (\""<<get_name()<<"\")"<<std::endl;
    ClosedSplineCurve oldcurve = curve;
    int npoints;
    str>>npoints;
    //std::cout<<"  # of points: "<<npoints<<std::endl;
    float x, y;
    if( npoints > 0 ) curve.clear_points();
    for( int i = 0; i < npoints; i++ ) {
      str>>x>>y;
      //std::cout<<"  point #"<<i<<": "<<x<<","<<y<<std::endl;
      curve.add_point( i, x, y );
    }
    if( oldcurve != curve )
      modified();
    //str>>value;
  }

  void to_stream(std::ostream& str)
  {
    //std::vector<std::pair<float, float> > points = curve.get_points();
    str<<" "<<curve.get_npoints();
    for( unsigned int i = 0; i < curve.get_npoints(); i++ )
      str<<" "<<curve.get_point(i).first<<" "<<curve.get_point(i).second;
    //str<<value;
  }

  bool import(PropertyBase* pin)
  {
    //std::cout<<"Property<ClosedSplineCurve>::import() called (\""<<get_name()<<"\")"<<std::endl;
    Property<ClosedSplineCurve>* pin2 = dynamic_cast< Property<ClosedSplineCurve>* >( pin );
    if( pin2 ) {
      //std::cout<<"  gen_npoints()="<<curve.get_npoints()<<"  pin2->get_npoints()="<<pin2->get().get_npoints()<<std::endl;
      set( pin2->get() );
    } else {
      set_str( pin->get_str() );
    }
    return true;
  }

  void set_gobject(gpointer object)
  {
    //g_object_set( object, get_name().c_str(), value, NULL );
    //set_gobject_property( object, get_name(), value );
  }
};
}


#endif
