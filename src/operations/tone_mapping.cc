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

#include "tone_mapping.hh"


//const int PF::GamutMapNYPoints = 1000;


#define Float_t float
#define Double_t float



float PF::HD_filmic(float x, float *par)
{
  Float_t HD_fog = 0.0;
  Float_t HD_max = 4.0;
  Float_t HD_lin_slope = par[0];
  Float_t HD_lin_pivot = par[1];
  Float_t HD_lin_Dmin = par[2];
  Float_t HD_lin_Dmax = par[3];

  Float_t HD_lin_min = -1.0f * (HD_lin_Dmax - HD_lin_pivot) / (HD_max * HD_lin_slope);
  Float_t HD_lin_max = -1.0f * (HD_lin_Dmin - HD_lin_pivot) / (HD_max * HD_lin_slope);

  //std::cout<<HD_lin_min<<" "<<HD_lin_max<<"\n";

  Float_t LogH = x; //0.1f * val / HD_midgray;
  Double_t result;

  if( LogH > HD_lin_max ) {
    Float_t A = (HD_lin_Dmin - HD_fog) * 2.0f + 1e-2;
    Float_t k = -4.0f * HD_lin_slope * HD_max / A;
    //std::cout<<A<<" "<<k<<"\n";
    result = HD_lin_Dmin - (A * 1.0f/(1 + exp(k*(LogH-HD_lin_max))) - A/2);
  } else if( LogH < HD_lin_min ) {
    Float_t A = (HD_max - HD_lin_Dmax) * 2.0f;
    Float_t k = -4.0f * HD_lin_slope * HD_max / A;
    result = HD_lin_Dmax - (A * 1.0f/(1 + exp(k*(LogH-HD_lin_min))) - A/2);
  } else {
    result = -1.0f * LogH * HD_lin_slope * HD_max + HD_lin_pivot;
  }

  return result;
}


float PF::HD_filmic2(float x, float *par)
{
  float HD_midgray = 0.1814;//pow(0.5,2.45);

  float LogH = log10( x / HD_midgray );
  float D = PF::HD_filmic( LogH, par );
  float result = pow(10,-D);
  //std::cout<<"filmic2: "<<x<<" "<<LogH<<" "<<D<<" "<<result<<"\n\n";
  return result;
}



PF::ToneMappingPar::ToneMappingPar():
  OpParBase(),
  method("method",this,PF::TONE_MAPPING_LIN_POW,"TONE_MAPPING_LIN_POW","linear + log"),
  exposure("exposure",this,1),
  gamma("gamma",this,2.2),
  gamma_pivot("gamma_pivot",this,1),
  filmic_A("filmic_A",this,0.22),
  filmic_B("filmic_B",this,0.30),
  filmic_C("filmic_C",this,0.10),
  filmic_D("filmic_D",this,0.20),
  filmic_E("filmic_E",this,0.01),
  filmic_F("filmic_F",this,0.30),
  filmic_W("filmic_W",this,11.2),
  filmic2_preserve_midgray("filmic2_preserve_midgray",this,false),
  filmic2_gamma("filmic2_gamma",this,0),
  filmic2_TS("filmic2_TS",this,0.5),
  filmic2_TL("filmic2_TL",this,0.5),
  filmic2_SS("filmic2_SS",this,2.0),
  filmic2_SL("filmic2_SL",this,0.5),
  filmic2_SA("filmic2_SA",this,1.0),
  AL_Lmax("AL_Lmax",this,2),
  AL_b("AL_b",this,0.85),
  AL_Tsize("AL_Tsize",this,0.5),
  AL_Tlength("AL_Tlength",this,10),
  AL_Tstrength("AL_Tstrength",this,0.5),
  LP_compression("LP_compression",this,2.7),
  LP_slope("LP_slope",this,1.09),
  LP_lin_max("LP_lin_max",this,0.5),
  LP_knee_strength("LP_knee_strength",this,1.2),
  LP_shoulder_smoothness("LP_shoulder_smoothness",this,0),
  HD_slope("HD_slope",this,1.1),
  HD_toe_range("HD_toe_range",this,0.00001),
  HD_shoulder_range("HD_shoulder_range",this,0.2),
  gamut_compression("gamut_compression",this,0),
  gamut_compression_exponent("gamut_compression_exponent",this,1),
  lumi_blend_frac("lumi_blend_frac",this,0),
  icc_data( NULL )
{
  method.add_enum_value(PF::TONE_MAPPING_LIN_POW,"TONE_MAPPING_LIN_POW","linear + log");
  method.add_enum_value(PF::TONE_MAPPING_HD,"TONE_MAPPING_HD","reversible film");
  method.add_enum_value(PF::TONE_MAPPING_FILMIC2,"TONE_MAPPING_FILMIC2","filmic new");
  method.add_enum_value(PF::TONE_MAPPING_FILMIC,"TONE_MAPPING_FILMIC","filmic");
  method.add_enum_value(PF::TONE_MAPPING_EXP_GAMMA,"TONE_MAPPING_EXP_GAMMA","gamma");
  method.add_enum_value(PF::TONE_MAPPING_REINHARD,"TONE_MAPPING_REINHARD","Reinhard");
  method.add_enum_value(PF::TONE_MAPPING_HEJL,"TONE_MAPPING_HEJL","Hejl-Dawson");
  method.add_enum_value(PF::TONE_MAPPING_ADAPTIVE_LOG,"TONE_MAPPING_ADAPTIVE_LOG","adaptive log");
  set_type("tone_mapping" );

  set_default_name( _("tone mapping") );
}



VipsImage* PF::ToneMappingPar::build(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  if( in.size()<1 || in[0]==NULL ) return NULL;

  exponent = 1.f/gamma.get();
  filmic2_exponent = (filmic2_gamma.get() >= 0) ? 1.f/(filmic2_gamma.get()+1) : (1.f-filmic2_gamma.get());

  ICCProfile* new_icc_data = PF::get_icc_profile( in[0] );
  if( false && new_icc_data && (new_icc_data != icc_data) ) {
    ICCProfile* labprof = ICCStore::Instance().get_Lab_profile();
    ICCTransform rgb2lab, lab2rgb;
    rgb2lab.init(new_icc_data, labprof, VIPS_FORMAT_FLOAT);
    lab2rgb.init(labprof, new_icc_data, VIPS_FORMAT_FLOAT);

    // re-compute the gamut boundaries
    int NYsteps = GamutMapNYPoints;
    float Ydelta = 1.0f / (NYsteps);
    float Y = Ydelta;
    for(int h = 0; h < 360; h++) {
      gamut_boundary[0][h] = gamut_boundary[NYsteps][h] = 0;
    }
    for(int y = 1; y < NYsteps; y++) {
      Y = Ydelta*y;
      float L = new_icc_data->linear2perceptual(Y)*100;
      for(int h = 0; h < 360; h++) {
        float C = 128, Cmin = 0, Cmax = 256;
        bool found = false;
        float Lab[3], LCH[3], RGB[3];
        float delta = 1.0e-5;
        gamut_boundary[y][h] = 256;
        int iter = 0;
        while( !found ) {
          LCH[0] = L; LCH[1] = C; LCH[2] = h*M_PI/180.f;
          PF::LCH2Lab(LCH, Lab, 1);
          lab2rgb.apply(Lab,RGB);
          //std::cout<<"Gamut: LCh="<<LCH[0]<<" "<<LCH[1]<<" "<<LCH[2]<<" "<<std::endl;
          //std::cout<<"Gamut: Lab="<<Lab[0]<<" "<<Lab[1]<<" "<<Lab[2]<<" "<<std::endl;
          //std::cout<<"Gamut: RGB="<<RGB[0]<<" "<<RGB[1]<<" "<<RGB[2]<<" "<<std::endl;
          if( RGB[0] < (1.0f-delta) && RGB[1] < (1.0f-delta) && RGB[2] < (1.0f-delta) &&
              RGB[0] > delta && RGB[1] > delta && RGB[2] > delta ) {
            // we are still within gamut, increase C
            Cmin = C; C = (Cmax+Cmin)/2;
          } else if( RGB[0] > (1.0f+delta) || RGB[1] > (1.0f+delta) || RGB[2] > (1.0f+delta) ||
              RGB[0] < -delta || RGB[1] < -delta || RGB[2] < -delta ) {
            // at least one of the components is too much out-of-gamut, decrease C
            Cmax = C; C = (Cmax+Cmin)/2;
          } else found = true;
          if(found) {
            gamut_boundary[y][h] = C;
          }
          iter++;
          if(iter == 100) {break;}
        }
        std::cout<<"Y="<<Y<<"  L="<<L<<"  h="<<h<<"  C="<<C<<"  (min="<<Cmin<<" max="<<Cmax<<")  RGB="<<RGB[0]<<" "<<RGB[1]<<" "<<RGB[2]<<std::endl;
      }
      //std::cout<<"Y="<<Y<<" done."<<std::endl;
    }
    //getchar();
  }
  icc_data = new_icc_data;

  VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
  return out;
}
