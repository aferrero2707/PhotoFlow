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


#ifndef M_LOG2E
#define M_LOG2E 1.44269504088896340736 // log2(e)
#endif
#define HALF_MAX 65504.0f
extern const float POW_2_16, POW_2_15;

inline float log2f(const float& x){
  return log(x) * M_LOG2E;
}

inline float lin_to_ACEScc( float in)
{
  if (in <= 0)
    return -0.3584474886; // =(log2( pow(2.,-16.))+9.72)/17.52
  else if (in < POW_2_15)
    return (log2f( POW_2_16 + in * 0.5) + 9.72) / 17.52;
  else // (in >= pow(2.,-15))
    return (log2f(in) + 9.72) / 17.52;
}

inline float ACEScc_to_lin( float in)
{
  if (in < -0.3013698630) // (9.72-15)/17.52
    return (powf( 2., in*17.52-9.72) - powf( 2.,-16.))*2.0;
  else if ( in < (log2f(HALF_MAX)+9.72)/17.52 )
    return powf( 2., in*17.52-9.72);
  else // (in >= (log2(HALF_MAX)+9.72)/17.52)
    return HALF_MAX;
}

/*
* Jzazbz conversion code from https://github.com/quag/JzAzBz
* Copyright (c) 2018 Jonathan Wright (quag)
* released under the MIT license
*/

namespace jabz {
struct jabz1 { double j, a, b; };
struct jchz1 { double j, c, h; };
struct srgb1 { double r, g, b; };

struct jzczhz { double jz, cz, hz; };
struct jzazbz { double jz, az, bz; };
struct izazbz { double iz, az, bz; };
struct lms1 { double l, m, s; };
struct lms1_ { double l, m, s; };
struct xyz100_ { double x_, y_, z_; };
struct xyz100 { double x, y, z; };
struct rgb1 { double r, g, b; };

namespace jab_rgb {
constexpr auto b = 1.15;
constexpr auto g = 0.66;
constexpr auto d = -0.56;
constexpr auto d0 = 1.6295499532821566e-11;

constexpr auto c1 = 3424.0/4096.0;
constexpr auto c2 = 2413.0/128.0;
constexpr auto c3 = 2392.0/128.0;
constexpr auto n = 2610.0/16384.0;
constexpr auto p = 1.7*2523.0/32.0;

constexpr double dot(double a0, double a1, double b0, double b1, double c=0) {
  return a0*b0 + a1*b1 + c;
}

constexpr double dot(const double a0, const double a1, const double a2, const double b0, const double b1, const double b2) {
  return a0*b0 + a1*b1 + a2*b2;
}

template <class T> constexpr T matmul3(double x, double y, double z, double m00, double m01, double m02, double m10, double m11, double m12, double m20, double m21, double m22) {
  return {
    dot(x, y, z, m00, m01, m02),
        dot(x, y, z, m10, m11, m12),
        dot(x, y, z, m20, m21, m22),
  };
}

template <class T> constexpr T _f_(T x) {
  T y = pow(x/10000, n);
  return pow((c1 + c2*y)/(1 + c3*y), p);
};

static constexpr jzazbz forth(const xyz100& xyz) {
  /*xyz100 xyz = matmul3<xyz100>(rgb.r, rgb.g, rgb.b,
              100*0x1.a64c2f52ea72dp-2, 100*0x1.6e2eb1f1be0c8p-2, 100*0x1.71a9fdd4910cdp-3,
              100*0x1.b3679fbabb7e3p-3, 100*0x1.6e2eb13cc6544p-1, 100*0x1.27bb34179021ap-4,
              100*0x1.3c362381906d4p-6, 100*0x1.e83e4d9c14333p-4, 100*0x1.e6a7f1325153ep-1);
   */
   xyz100_ xyz_ = {
       xyz.x*b + xyz.z*(1-b),
       xyz.y*g + xyz.x*(1-g),
       xyz.z,
   };

   lms1 lms = matmul3<lms1>(xyz_.x_, xyz_.y_, xyz_.z_,
       0.41478972, 0.579999, 0.0146480,
       -0.2015100, 1.120649, 0.0531008,
       -0.0166008, 0.264800, 0.6684799);

   lms1 lms_ = {
       _f_(lms.l),
       _f_(lms.m),
       _f_(lms.s),
   };

   izazbz iab = matmul3<izazbz>(lms_.l, lms_.m, lms_.s,
       0.5, 0.5, 0,
       3.524000, -4.066708, 0.542708,
       0.199076, 1.096799, -1.295875);

   return {
     ((1 + d)*iab.iz)/(1 + d*iab.iz) - d0,
         iab.az,
         iab.bz,
   };
}

template <class T> constexpr T _finv_(T x) {
  if (x < 0.) {
    return 0.;
  }
  T y = pow(x, 1/p);
  T result = 10000*pow((c1 - y)/(c3*y - c2), 1/n);
  //std::cout<<"_finv_: x="<<x<<"  y="<<y<<"  result="<<result<<std::endl;
  return result; //10000*pow((c1 - y)/(c3*A2() - c2)(y), 1/n);
};


static
//constexpr
xyz100 back(const jzazbz& jab) {
  //std::cout<<"jabz::jab_rgb::back: Jz="<<jab.jz<<std::endl;
  auto iz = (jab.jz + d0) / (d + 1.0f - d*(jab.jz + d0));
  //std::cout<<"jabz::jab_rgb::back: iz="<<iz<<std::endl;

  izazbz iab = {
      iz,
      jab.az,
      jab.bz,
  };

  lms1 lms = {
      _finv_(dot(iab.az, iab.bz, 0x1.1bdcf5ff4b9ffp-3, 0x1.db860b905af44p-5, iab.iz)),
      _finv_(dot(iab.az, iab.bz, -0x1.1bdcf5ff4b9fep-3, -0x1.db860b905af4fp-5, iab.iz)),
      _finv_(dot(iab.az, iab.bz, -0x1.894b7904a2cf8p-4, -0x1.9fb04b6ae56fdp-1, iab.iz)),
  };
  //std::cout<<"jabz::jab_rgb::back: lms="<<lms.l<<" "<<lms.m<<" "<<lms.s<<" "<<std::endl;

  xyz100_ xyz_ = matmul3<xyz100_>(lms.l, lms.m, lms.s,
      0x1.ec9a1a8bce714p+0, -0x1.013a11a9de8acp+0, 0x1.3470b79eb8366p-5,
      0x1.66b96ff1c1292p-2, 0x1.73f557d230e47p-1, -0x1.0bd08963ad7e9p-4,
      -0x1.74aa645ab6306p-4, -0x1.403bd8515285fp-2, 0x1.85d407843f9bep+0);
  //std::cout<<"jabz::jab_rgb::back: xyz_="<<xyz_.x_<<" "<<xyz_.y_<<" "<<xyz_.z_<<" "<<std::endl;

  auto x = xyz_.z_ * ((b-1)/b) + (xyz_.x_/b);

  xyz100 xyz = {
      x,
      x * ((g-1)/g) + (xyz_.y_/g),
      xyz_.z_,
  };
  //std::cout<<"jabz::jab_rgb::back: xyz="<<xyz.x<<" "<<xyz.y<<" "<<xyz.z<<" "<<std::endl;

  return xyz;
}
};
};
/*
* Eend of Jzazbz conversion code from https://github.com/quag/JzAzBz
*/

}

#endif
