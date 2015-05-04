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

#include <stdlib.h>
#include <vips/vips.h>

#include "splinecurve.hh"

PF::SplineCurve::SplineCurve(): 
  PF::Curve(),
  ypp( NULL ),
  ypp_size( 0 )
{
  points_mutex = vips_g_mutex_new();
#ifdef SPLINE_USE_STDVEC
  points.push_back( std::make_pair(float(0),float(0)) );
  points.push_back( std::make_pair(float(1),float(1)) );
#else
  points = new std::pair<float,float>[2];
  points[0] = std::make_pair(float(0),float(0));
  points[1] = std::make_pair(float(1),float(1));
  npoints = 2;
#endif
  update_spline();
}



PF::SplineCurve::~SplineCurve()
{
}



int PF::SplineCurve::add_point( float x, float y )
{
  //if( (get_npoints()>0) && (x<=points[0].first) ) return -1;
  //if( (get_npoints()>1) && (x >= points[get_npoints()-1].first) ) return -1;
  //return -1;
  lock();
#ifdef SPLINE_USE_STDVEC
#ifndef NDEBUG
  std::cout<<"PF::SplineCurve::add_point( "<<x<<", "<<y<<" ): points.size()="<<points.size()<<std::endl;
#endif
  for( unsigned int i = 0; i < points.size(); i++ ) {
    if( points[i].first <= x ) continue;
    points.insert( points.begin()+i, std::make_pair(x,y) );
#ifndef NDEBUG
    std::cout<<"PF::SplineCurve::add_point( "<<x<<", "<<y<<" ): point added before "<<points[i].first<<std::endl;
#endif
    update_spline();
    unlock();
    return i;
  }
  points.push_back( std::make_pair(x,y) );
  update_spline();
  unlock();
  return( points.size()-1 );
#else
  npoints += 1;
  std::pair<float,float>* points_new = new std::pair<float,float>[npoints];
#ifndef NDEBUG
  //std::cout<<"PF::SplineCurve::add_point( "<<x<<", "<<y<<" ): points.size()="<<points.size()<<std::endl;
  std::cout<<"PF::SplineCurve::add_point( "<<x<<", "<<y<<" ): npoints="<<npoints<<std::endl;
#endif
  //for( unsigned int i = 0; i < points.size(); i++ ) {
  for( unsigned int i = 0; i < npoints-1; i++ ) {
    if( points[i].first <= x ) {
      points_new[i] = points[i];
      continue;
    }
#ifndef NDEBUG
    std::cout<<"PF::SplineCurve::add_point( "<<x<<", "<<y<<" ): adding point before "<<points[i].first<<std::endl;
#endif
    //points.insert( points.begin()+i, std::make_pair(x,y) );
    for( size_t j = i; j < npoints-1; j++)
      points_new[j+1] = points[j];
    points_new[i] = std::make_pair(x,y);
    delete[] points;
    points = points_new;
    update_spline();
    unlock();
    return i;
  }
  //points.push_back( std::make_pair(x,y) );
  points_new[npoints-1] = std::make_pair(x,y);
  delete[] points;
  points = points_new;
  update_spline();
  unlock();
  //return( points.size()-1 );
  return -1;
#endif
}


bool PF::SplineCurve::remove_point( unsigned int id )
{
  if( id == 0 ) return false;
  if( id >= (get_npoints()-1) ) return false;
  
#ifdef SPLINE_USE_STDVEC
  points.erase( points.begin() + id );
#else
  npoints -= 1;
  std::pair<float,float>* points_new = new std::pair<float,float>[npoints];
  
  for( unsigned int i = 0; i < id; i++ ) {
    points_new[i] = points[i];
  }
  for( size_t i = id; i < npoints; i++)
    points_new[i] = points[i+1];
  delete[] points;
  points = points_new;
#endif
  update_spline();
  return true;
}


bool PF::SplineCurve::set_point( unsigned int id, float& x, float& y )
{
  //if( id >= points.size() ) return false;
  if( id >= get_npoints() ) return false;

  if( x < 0 ) x = 0;
  if( y < 0 ) y = 0;
  if( x > 1 ) x = 1;
  if( y > 1 ) y = 1;

  if( (id>0) && (x<=points[id-1].first) ) x = points[id].first;
  //if( (id<(points.size()-1)) && (x>=points[id+1].first) ) x = points[id].first;
  if( (id<(get_npoints()-1)) && (x>=points[id+1].first) ) x = points[id].first;
  points[id] = std::make_pair( x, y );
  update_spline();
  return true;
}


void PF::SplineCurve::update_spline() 
{
  //unsigned int N = points.size();
  unsigned int N = get_npoints();
  if( N < 2) return;
  double* u = new double[N-1];
  if( ypp ) delete [] ypp;
  ypp = new double [N];
  ypp_size = N;
  //std::cout<<"SplineCurve::update_spline(): ypp_size="<<ypp_size<<std::endl;
  
  ypp[0] = u[0] = 0.0;	/* set lower boundary condition to "natural" */
  
  for (int i = 1; i < N - 1; ++i) {
    double sig = (points[i].first - points[i - 1].first) / (points[i + 1].first - points[i - 1].first);
    double p = sig * ypp[i - 1] + 2.0;
    ypp[i] = (sig - 1.0) / p;
    u[i] = ((points[i + 1].second - points[i].second)
	    / (points[i + 1].first - points[i].first) - (points[i].second - points[i - 1].second) / (points[i].first - points[i - 1].first));
    u[i] = (6.0 * u[i] / (points[i + 1].first - points[i - 1].first) - sig * u[i - 1]) / p;
  }
  
  ypp[N - 1] = 0.0;
  for (int k = N - 2; k >= 0; --k)
    ypp[k] = ypp[k] * ypp[k + 1] + u[k];
  
  delete [] u;
}



#define CLIPD(a) ((a)>0.0?((a)<1.0?(a):1.0):0.0)

float PF::SplineCurve::get_value( float x )
{
  //int size = points.size();
  //unsigned int N = points.size();
  unsigned int N = get_npoints();
  //std::cout<<"size = "<<size<<std::endl;
  //return x;

  if( x <= points[0].first ) return points[0].second;
  if( x >= points[N-1].first ) return points[N-1].second;
  
  //std::cout<<"N = "<<N<<std::endl;
  // do a binary search for the right interval:
  int k_lo = 0, k_hi = N - 1;
  while (k_hi - k_lo > 1){
    int k = (k_hi + k_lo) / 2;
    //std::cout<<"  k = "<<k<<std::endl;
    if (points[k].first > x)
      k_hi = k;
    else
      k_lo = k;
  }
  
  std::pair<float,float> p_lo = points[k_lo];
  std::pair<float,float> p_hi = points[k_hi];
  double h = p_hi.first - p_lo.first;
  //std::cout<<"points.size()="<<points.size()<<"  h="<<"points["<<k_hi<<"].first - points["<<k_lo<<"].first = "<<h<<std::endl;
//std::cout<<"  points[k_hi].second="<<points[k_hi].second<<"  points[k_lo].second="<<points[k_lo].second<<std::endl;
  // linear
  if( N == 2) {
    float result = p_lo.second + (x - p_lo.first) * ( p_hi.second - p_lo.second ) / h;
    //std::cout<<"result = "<<result<<std::endl;
    return result;
  // spline curve
  } else { // if (kind==Spline) {
    if( k_hi >= ypp_size )
      std::cout<<"k_lo="<<k_lo<<"  k_hi="<<k_hi<<"  ypp_size="<<ypp_size<<"  N="<<N<<std::endl;
    double a = (points[k_hi].first - x) / h;
    double b = (x - points[k_lo].first) / h;
    double r1 = a*points[k_lo].second;
    double r2 = b*points[k_hi].second;
    double r3 = (a*a*a - a)*ypp[k_lo];
    double r4 = (b*b*b - b)*ypp[k_hi];
    double r = r1 + r2 +(r3+r4)*(h*h)/6.0;
/*
    double r =
        a*points[k_lo].second +
        b*points[k_hi].second +
        ((a*a*a - a)*ypp[k_lo] +
            (b*b*b - b)*ypp[k_hi]) *
            (h*h)/6.0;
 */
    return CLIPD(r);
  }
}


float PF::SplineCurve::get_delta( float x )
{ 
  return( get_value(x) - x );
}


void PF::SplineCurve::get_values( std::vector< std::pair<float,float> >& vec )
{ 
  for (unsigned int i=0; i<vec.size(); i++)
    vec[i].second = get_value( vec[i].first );
}


void PF::SplineCurve::get_deltas( std::vector< std::pair<float,float> >& vec )
{ 
  for (unsigned int i=0; i<vec.size(); i++) {
    float val = get_value( vec[i].first );
    vec[i].second = val - vec[i].first;
  }
}

