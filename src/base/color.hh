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


#ifndef PF_COLOR__HH
#define PF_COLOR__HH

#ifndef MIN
#define MIN( a, b ) ((a<b) ? a : b)
#endif
#define MIN3( a, b, c ) MIN(a,MIN(b,c))

#ifndef MAX
#define MAX( a, b ) ((a>b) ? a : b)
#endif
#define MAX3( a, b, c ) MAX(a,MAX(b,c))

#ifndef CLIPF
#define CLIPF(a) ((a)>0.0f?((a)<1.0f?(a):1.0f):0.0f)
#endif

#include <math.h>

#include "format_info.hh"

namespace PF
{


  template<class T>
  void to_float( const T& in, float& out )
  {
    float f = static_cast<float>(in);
    out= (f+FormatInfo<T>::MIN)/FormatInfo<T>::RANGE;
  }
  template<>
    void to_float<float>( const float& in, float& out );

  template<class T>
  void from_float( const float& in, T& out )
  {
    out = static_cast<T>( (in*FormatInfo<T>::RANGE)-FormatInfo<T>::MIN );
  }
  template<>
    void from_float<float>( const float& in, float& out );


  void Lab_pf2lcms(float* p);
  void Lab_pf2lcms(float* pin, float* pout);

  void Lab2LCH(float* pin, float* pout, int n);
  void LCH2Lab(float* pin, float* pout, int n);

  // 
  template<class T>
  void rgb2hsv(const T& r, const T& g, const T& b, float& h, float& s, float& v)
  {
    T min = MIN3(r, g, b);
    T max = MAX3(r, g, b);
    typename FormatInfo<T>::SIGNED c = static_cast<typename FormatInfo<T>::SIGNED>(max) - min;

    to_float( max, v );
    s = h = 0;

    if( c == 0 ) return;

    s = static_cast<float>(c)/max;
    float fr = static_cast<float>(r);
    float fg = static_cast<float>(g);
    float fb = static_cast<float>(b);

    if( r == max ) h = (fg-fb)/c;
    else if( g == max ) h = (fb-fr)/c + 2.0f;
    else if( b == max ) h = (fr-fg)/c + 4.0f;
    h *= 60;
    if( h < 0 ) h += 360;
  }


  // 
  template<class T>
  void hsv2rgb( const float& h, const float& s, const float& v, T& r, T& g, T& b )
  {
    int i;
    float sect, f, p, q, t, c;

    if( s == 0 ) {
      // achromatic (grey)
      from_float( v, r );
      g = b =r;
      return;
    }

    c = v * s;
    sect = h/60;			// sector 0 to 5
    i = floor( sect );
    f = sect - i;			// factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );

    switch( i ) {
		case 0:
			from_float( v, r );
			from_float( t, g );
			from_float( p, b );
			break;
		case 1:
			from_float( q, r );
			from_float( v, g );
			from_float( p, b );
			break;
		case 2:
			from_float( p, r );
			from_float( v, g );
			from_float( t, b );
			break;
		case 3:
			from_float( p, r );
			from_float( q, g );
			from_float( v, b );
			break;
		case 4:
			from_float( t, r );
			from_float( p, g );
			from_float( v, b );
			break;
		default:		// case 5:
			from_float( v, r );
			from_float( p, g );
			from_float( q, b );
			break;
    }
  }



  //
  // Converts an HSV color to an RGB color, according to the algorithm described at http://en.wikipedia.org/wiki/HSL_and_HSV
  // Code taken from here: http://wiki.beyondunreal.com/HSV-RGB_Conversion
  template<class T>
  void hsv2rgb2( const float& h, const float& s, const float& v, T& r, T& g, T& b )
  {
    float Min;
    float Chroma;
    float Hdash;
    float X;
    float R, G, B;
    int i;
 
    if( s == 0 ) {
      // achromatic (grey)
      from_float( v, r );
      g = b = r;
      return;
    }

    R = G = B = 0;
    Chroma = s * v;
    Hdash = h / 60.0;
    X = Chroma * (1.0f - fabs(fmod(Hdash,2) - 1.0f));
    i = floor( Hdash );
 
    switch( i ) {
		case 0:
      R = Chroma;
      G = X;
      break;
    case 1:
      R = X;
      G = Chroma;
      break;
    case 2:
      G = Chroma;
      B = X;
      break;
    case 3:
      G = X;
      B = Chroma;
      break;
    case 4:
      R = X;
      B = Chroma;
      break;
    default:
      R = Chroma;
      B = X;
      break;
    }
    
    Min = v - Chroma;
 
    R += Min;
    G += Min;
    B += Min;
    if(R>1) R=1; else if(R<0) R=0;
    if(G>1) G=1; else if(G<0) G=0;
    if(B>1) B=1; else if(B<0) B=0;
    from_float(R, r);
    from_float(G, g);
    from_float(B, b);
  }




  //
   template<class T>
   void rgb2hsl(const T& r, const T& g, const T& b, float& h, float& s, float& l)
   {
     float fr, fg, fb; to_float( r, fr ); to_float( g, fg ); to_float( b, fb );
     float min = MIN3(fr, fg, fb);
     float max = MAX3(fr, fg, fb);
     float sum = max+min;
     float delta = max-min;

     l = sum/2.0f;

     s = h = 0;

     if( delta == 0 ) return;

     if( l <= 0.5 )
       s = (max-min)/sum;
     else
       s = (max-min)/(2.0f-max-min);

     if( fr == max ) h = (fg-fb)/delta;
     else if( fg == max ) h = (fb-fr)/delta + 2.0f;
     else if( fb == max ) h = (fr-fg)/delta + 4.0f;
     h *= 60;
     if( h < 0 ) h += 360;
   }



   float hsl_value( float n1, float n2, float hue);

   //
   template<class T>
   void hsl2rgb( const float& h, const float& s, const float& l, T& r, T& g, T& b )
   {
     int i;
     float H = h/60.0f;

     if( s == 0 ) {
       // achromatic (grey)
       from_float( l, r );
       g = b = r;
       return;
     }

     float m1, m2;
     if( l <= 0.5 )
       m2 = l * (s + 1);
     else
       m2 = l + s - l * s;

     m1 = l * 2.0f - m2;

     float fr = hsl_value( m1, m2, H + 2.0f );
     float fg = hsl_value( m1, m2, H );
     float fb = hsl_value( m1, m2, H - 2.0f );

     from_float( fr, r );
     from_float( fg, g );
     from_float( fb, b );
   }



  // Computes the luminance value of a given RGB triplet
  template<class T>
  T luminance(const T& r, const T& g, const T& b)
  {
    return( r*0.3 + g*0.59 + b*0.11 );
  }


  // Clips the RGB values resulting from a luminosity/color blend
  template<class T>
  void clip_lc_blend(const typename FormatInfo<T>::SIGNED& rin, 
                     const typename FormatInfo<T>::SIGNED& gin, 
                     const typename FormatInfo<T>::SIGNED& bin, 
                     T& rout, T& gout, T& bout)
  {
    typename FormatInfo<T>::SIGNED lumi = luminance( rin, gin, bin );
    typename FormatInfo<T>::SIGNED  min = MIN3( rin, gin, bin );
    typename FormatInfo<T>::SIGNED  max = MAX3( rin, gin, bin );
    //std::cout<<"clip_lc_blend(): lumi="<<lumi<<"  min="<<min
    //         <<"  max="<<max<<std::endl;

    rout = rin;
    gout = gin;
    bout = bin;
    if( min < FormatInfo<T>::MIN ) {
      rout = (T)(lumi + (((rin-lumi)*lumi)/(lumi-min)));
      gout = (T)(lumi + (((gin-lumi)*lumi)/(lumi-min)));
      bout = (T)(lumi + (((bin-lumi)*lumi)/(lumi-min)));
    }
    if( max > FormatInfo<T>::MAX ) {
      rout = (T)(lumi + (((rin-lumi)*(FormatInfo<T>::MAX-lumi))/(max-lumi)));
      gout = (T)(lumi + (((gin-lumi)*(FormatInfo<T>::MAX-lumi))/(max-lumi)));
      bout = (T)(lumi + (((bin-lumi)*(FormatInfo<T>::MAX-lumi))/(max-lumi)));
    }
    //std::cout<<"                  input:  "<<rin<<" "<<gin<<" "<<bin<<std::endl
    //         <<"                  output: "<<rout<<" "<<gout<<" "<<bout<<std::endl;
  }


  // Blends a given RGB triplet and a given luminance value.
  // The input RGB values are replaced by the blended ones.
  template<class T>
  void lc_blend(T& red, T& green, T& blue, const T& lumi)
  {
    typename FormatInfo<T>::SIGNED diff = 
      ((typename FormatInfo<T>::SIGNED)lumi) - luminance( red, green, blue );
    //std::cout<<"Luminance top="<<lumi<<"  bottom="<<luminance( red, green, blue )<<"  diff="<<diff<<std::endl;
    typename FormatInfo<T>::SIGNED newred = ((typename FormatInfo<T>::SIGNED)red) + diff;
    typename FormatInfo<T>::SIGNED newgreen = ((typename FormatInfo<T>::SIGNED)green) + diff;
    typename FormatInfo<T>::SIGNED newblue = ((typename FormatInfo<T>::SIGNED)blue) + diff;
    clip_lc_blend( newred, newgreen, newblue, red, green, blue );
    //std::cout<<"red="<<red<<" new="<<newred<<"  green="<<green<<" new="<<newgreen<<"  blue="<<blue<<" new="<<newblue<<std::endl;
  }
}

#endif
