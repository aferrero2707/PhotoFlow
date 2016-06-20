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


#include "gaussblur.hh"
#include "wavdec.hh"
#include "split_details.hh"


  class SplitDetailsLevelPar: public PF::OpParBase
  {
    int blur_type;
    float blur_radius;
    int initial_Lev;
    PF::ProcessorBase* gauss;
    PF::ProcessorBase* wavdec;

  public:
    SplitDetailsLevelPar();
    ~SplitDetailsLevelPar() { std::cout<<"~SplitDetailsLevelPar() called."<<std::endl; }

    void set_blur_radius(float r) { blur_radius = r; }
    void set_initial_Lev(int r) { initial_Lev = r; }
    void set_blur_type(int r) { blur_type = r; }

    std::vector<VipsImage*> build_many(std::vector<VipsImage*>& in, int first,
        VipsImage* imap, VipsImage* omap,
        unsigned int& level);
  };


  SplitDetailsLevelPar::SplitDetailsLevelPar():
    PF::OpParBase()
  {
    blur_type = PF::SPLIT_DETAILS_BLUR_GAUSS;
    
    gauss = PF::new_gaussblur();
    wavdec = PF::new_wavdec();

    set_type("decompose_level");
  }



  std::vector<VipsImage*> SplitDetailsLevelPar::build_many(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap, unsigned int& level)
  {
    VipsImage* srcimg = NULL;
    if( in.size() > 0 ) srcimg = in[0];
    std::vector<VipsImage*> outvec;

    if( !srcimg ) return outvec;

    std::vector<VipsImage*> in2;
    
    if (blur_type == PF::SPLIT_DETAILS_BLUR_GAUSS) { 
      PF::GaussBlurPar* gausspar = dynamic_cast<PF::GaussBlurPar*>( gauss->get_par() );
      if( gausspar ) {
        in2.push_back( srcimg );
        
        gausspar->set_radius( blur_radius );
        
        gausspar->set_image_hints( srcimg );
        gausspar->set_format( get_format() );
        VipsImage* smoothed = gausspar->build( in2, 0, NULL, NULL, level );
  
        in2.clear();
        in2.push_back( smoothed );
        in2.push_back( srcimg );
        VipsImage* out = PF::OpParBase::build( in2, 0, NULL, NULL, level );
  
        outvec.push_back( smoothed );
        outvec.push_back( out );
      }
    }
    else if (blur_type == PF::SPLIT_DETAILS_BLUR_WAVELETS) { 
      PF::WavDecPar* wavdecpar = dynamic_cast<PF::WavDecPar*>( wavdec->get_par() );
      if( wavdecpar ) {
        in2.push_back( srcimg );
        
        wavdecpar->set_initial_Lev( initial_Lev );
        wavdecpar->set_numScales( 1 );
        wavdecpar->set_currScale( 2 );
        wavdecpar->set_blendFactor( .5f );
      
        wavdecpar->set_image_hints( srcimg );
        wavdecpar->set_format( get_format() );
        VipsImage* smoothed = wavdecpar->build( in2, 0, NULL, NULL, level );
  
        in2.clear();
        in2.push_back( smoothed );
        in2.push_back( srcimg );
        VipsImage* out = PF::OpParBase::build( in2, 0, NULL, NULL, level );
  
        outvec.push_back( smoothed );
        outvec.push_back( out );
      }
    }
    
    return outvec;
  }


  template < OP_TEMPLATE_DEF >
  class SplitDetailsLevelProc
  {
  public:
    void render(VipsRegion** ireg, int n, int in_first,
                VipsRegion* imap, VipsRegion* omap,
                VipsRegion* oreg, PF::OpParBase* par)
    {
      if( n != 2 ) return;
      if( ireg[0] == NULL ) return;
      if( ireg[1] == NULL ) return;

      Rect *r = &oreg->valid;
      int line_size = r->width * oreg->im->Bands;
      int height = r->height;

      T* porig;
      T* pblur;
      T* pout;
      typename PF::FormatInfo<T>::SIGNED diff;
//      float grey, ngrey, intensity;
      int x, y/*, pos*/;

      for( y = 0; y < height; y++ ) {
        pblur = (T*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
        porig = (T*)VIPS_REGION_ADDR( ireg[1], r->left, r->top + y );
        pout = (T*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

        for( x = 0; x < line_size; x++ ) {
          diff = static_cast< typename PF::FormatInfo<T>::SIGNED >(porig[x]) -
              static_cast< typename PF::FormatInfo<T>::SIGNED >(pblur[x]);
          pout[x] = diff/2 + PF::FormatInfo<T>::HALF;
        }
      }
    }
  };




PF::SplitDetailsPar::SplitDetailsPar():
  PF::OpParBase(),
  blur_type("blur_type", this, PF::SPLIT_DETAILS_BLUR_GAUSS, "BLUR_GAUSS", _("gaussian")),
  prop_nscales("nscales",this,1),
  prop_base_scale("base_scale",this,5)
{	
  blur_type.add_enum_value( PF::SPLIT_DETAILS_BLUR_WAVELETS, "BLUR_WAVELETS", _("wavelets") );
  
  set_type( "split_details");
  set_default_name( _("split details") );
}


std::vector<VipsImage*> PF::SplitDetailsPar::build_many(std::vector<VipsImage*>& in, int first,
    VipsImage* imap, VipsImage* omap,
    unsigned int& level)
{
  VipsImage* srcimg = NULL;
  if( in.size() > 0 ) srcimg = in[0];
  std::vector<VipsImage*> outvec, scalevec;

  // Fill the vector of level decomposing operations with as many elemens as the number of
  // requested scales. If the vector already contains a sufficient number of entries,
  // nothing happens
  for( int i = levels.size(); i < prop_nscales.get(); i++ ) {
    PF::ProcessorBase* l = new PF::Processor<SplitDetailsLevelPar,SplitDetailsLevelProc>();
    levels.push_back( l );
  }

  if( !srcimg ) return outvec;

//  std::cout<<"SplitDetailsPar::build_many(): number of scales = "<<prop_nscales.get()<<std::endl;
  float blur_radius = prop_base_scale.get();
  VipsImage* prev_scale = srcimg;
  PF_REF( prev_scale, "SplitDetailsPar::build_many(): initial prev_scale ref" );
  
  for( int i = 0; i < prop_nscales.get(); i++ ) {
    PF::ProcessorBase* l = levels[i];
    if( !l ) {
      std::cout<<"SplitDetailsPar::build_many(): NULL operation for scale "<<i<<std::endl;
      return outvec;
    }
    
    SplitDetailsLevelPar* lpar = dynamic_cast<SplitDetailsLevelPar*>( l->get_par() );
    
    lpar->set_blur_type( blur_type.get_enum_value().first );
    
    // gasussian
    lpar->set_blur_radius( blur_radius );
    // wavelets
    lpar->set_initial_Lev( i );
    
    lpar->set_image_hints( prev_scale );
    lpar->set_format( get_format() );
    
    std::vector<VipsImage*> in2, lout;
    in2.push_back( prev_scale );
    lout = lpar->build_many( in2, 0, NULL, NULL, level );
    if( lout.size() != 2 || lout[0] == NULL || lout[1] == NULL ) {
      std::cout<<"SplitDetailsPar::build_many(): bad output from decomposition step "<<i<<std::endl;
      return outvec;
    }
    PF_UNREF( prev_scale, "SplitDetailsPar::build_many(): prev_scale unref" );
    
    prev_scale = lout[0];
    scalevec.push_back( lout[1] );
    
    // gaussian
    blur_radius *= 2;
  }

  // Add the base level as first element of the output vector
  outvec.push_back( prev_scale );

  // Add scale levels in reverse order
  for( int i = scalevec.size()-1; i >= 0; i-- ) {
    outvec.push_back( scalevec[i] );
  }

//  std::cout<<"SplitDetailsPar::build_many(): outvec.size()="<<outvec.size()<<std::endl;
  return outvec;
}


PF::ProcessorBase* PF::new_split_details()
{
  return( new PF::Processor<PF::SplitDetailsPar,PF::SplitDetailsProc>() );
}
