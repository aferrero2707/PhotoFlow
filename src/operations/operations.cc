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

#include "../base/processor_imp.hh"
#include "operations.hh"
#include "no_demosaic.hh"
#include "amaze_demosaic.hh"
#include "basic_adjustments.hh"
#include "color_correction.hh"
#include "blender.hh"
#include "buffer.hh"
#include "ca_correct.hh"
#include "channel_mixer.hh"
#include "clip.hh"
#include "clipping_warning.hh"
#include "clone_stamp.hh"
#include "clone.hh"
#include "convert_colorspace.hh"
#include "convertformat.hh"
#include "crop.hh"
#include "curves.hh"
#include "defringe.hh"
#include "denoise.hh"
#include "desaturate.hh"
#include "desaturate_luminance.hh"
//#include "dynamic_range_compressor.hh"
#include "draw.hh"
#include "false_color_correction.hh"
#include "fast_demosaic.hh"
#include "fast_demosaic_xtrans.hh"
#include "gaussblur_sii.hh"
#include "gaussblur.hh"
#include "gradient.hh"
#include "hotpixels.hh"
#include "hsl_mask.hh"
#include "icc_transform.hh"
#include "igv_demosaic.hh"
#include "image_reader.hh"
#include "impulse_nr.hh"
#include "invert.hh"
#include "lensfun.hh"
#include "levels.hh"
#include "lmmse_demosaic.hh"
#include "maxrgb.hh"
#include "multiraw_developer.hh"
#include "nlmeans.hh"
#include "path_mask.hh"
#include "perspective.hh"
#include "raw_developer.hh"
#include "raw_loader.hh"
#include "raw_output.hh"
#include "raw_preprocessor.hh"
#include "rcd_demosaic.hh"
#include "scale.hh"
#include "shadows_highlights.hh"
#include "sharpen.hh"
#include "subtr_image.hh"
#include "threshold.hh"
#include "tone_mapping.hh"
#include "tone_mapping_v2.hh"
#include "local_contrast.hh"
#include "noise_generator.hh"
#include "trcconv.hh"


PF::ProcessorBase* PF::new_no_demosaic()
{ return( new PF::Processor<PF::NoDemosaicPar,PF::NoDemosaicProc>() ); }

PF::ProcessorBase* PF::new_amaze_demosaic()
{ return( new PF::Processor<PF::AmazeDemosaicPar,PF::AmazeDemosaicProc>() ); }

PF::ProcessorBase* PF::new_basic_adjustments()
{ return new PF::Processor<PF::BasicAdjustmentsPar,PF::BasicAdjustments>(); }

PF::ProcessorBase* PF::new_color_correction()
{ return new PF::Processor<PF::ColorCorrectionPar,PF::ColorCorrection>(); }

PF::ProcessorBase* PF::new_blender()
{ return( new PF::Processor<PF::BlenderPar,PF::BlenderProc>() ); }

PF::ProcessorBase* PF::new_buffer()
{ return( new PF::Processor<PF::BufferPar,PF::BufferProc>() ); }

PF::ProcessorBase* PF::new_ca_correct()
{ return( new PF::Processor<PF::CACorrectPar,PF::CACorrectProc>() ); }

PF::ProcessorBase* PF::new_channel_mixer()
{ return ( new PF::Processor<PF::ChannelMixerPar,PF::ChannelMixer>() ); }

PF::ProcessorBase* PF::new_clip()
{ return( new PF::Processor<PF::ClipPar,PF::ClipOp>() ); }

PF::ProcessorBase* PF::new_clipping_warning()
{ return( new PF::Processor<PF::ClippingWarningPar,PF::ClippingWarning>() ); }

PF::ProcessorBase* PF::new_clone_stamp()
{ return( new PF::Processor<PF::CloneStampPar,PF::CloneStampProc>() ); }

PF::ProcessorBase* PF::new_clone()
{ return( new PF::Processor<PF::ClonePar,PF::CloneProc>() ); }

PF::ProcessorBase* PF::new_convert_colorspace()
{ return new PF::Processor<PF::ConvertColorspacePar,PF::ConvertColorspace>(); }

PF::ProcessorBase* PF::new_gamut_warning()
{ return new PF::Processor<PF::GamutWarningPar,PF::GamutWarningProc>(); }

PF::ProcessorBase* PF::new_convert_format()
{ return new PF::Processor<PF::ConvertFormatPar,PF::ConvertFormatProc>(); }

PF::ProcessorBase* PF::new_crop()
{ return( new PF::Processor<PF::CropPar,PF::CropProc>() ); }

PF::ProcessorBase* PF::new_curves()
{ return( new PF::Processor<PF::CurvesPar,PF::Curves>() ); }

PF::ProcessorBase* PF::new_defringe()
{ return new PF::Processor<PF::DefringePar,PF::DefringeProc>(); }

PF::ProcessorBase* PF::new_defringe_algo()
{ return new PF::Processor<PF::DefringeAlgoPar,PF::DefringeAlgoProc>(); }

PF::ProcessorBase* PF::new_denoise()
{ return( new PF::Processor<PF::DenoisePar,PF::DenoiseProc>() ); }

PF::ProcessorBase* PF::new_desaturate_luminance()
{ return( new PF::Processor<PF::DesaturateLuminancePar,PF::DesaturateLuminanceProc>() ); }
PF::ProcessorBase* PF::new_desaturate_luminosity()
{ return( new PF::Processor<PF::PixelProcessorPar,PF::DesaturateLuminosity>() ); }
PF::ProcessorBase* PF::new_desaturate_lightness()
{ return( new PF::Processor<PF::PixelProcessorPar,PF::DesaturateLightness>() ); }
PF::ProcessorBase* PF::new_desaturate_average()
{ return( new PF::Processor<PF::PixelProcessorPar,PF::DesaturateAverage>() ); }
PF::ProcessorBase* PF::new_desaturate()
{ return( new PF::Processor<PF::DesaturatePar,PF::DesaturateProc>() ); }

//PF::ProcessorBase* PF::new_dynamic_range_compressor()
//{ return( new PF::Processor<PF::DynamicRangeCompressorPar,PF::DynamicRangeCompressorProc>() ); }

PF::ProcessorBase* PF::new_draw()
{ return( new PF::Processor<PF::DrawPar,PF::DrawProc>() ); }

PF::ProcessorBase* PF::new_false_color_correction()
{ return( new PF::Processor<PF::FalseColorCorrectionPar,PF::FalseColorCorrectionProc>() ); }

PF::ProcessorBase* PF::new_fast_demosaic()
{ return( new PF::Processor<PF::FastDemosaicPar,PF::FastDemosaicProc>() ); }

PF::ProcessorBase* PF::new_fast_demosaic_xtrans()
{ return( new PF::Processor<PF::FastDemosaicXTransPar,PF::FastDemosaicXTransProc>() ); }

PF::ProcessorBase* PF::new_gaussblur_sii()
{ return( new PF::Processor<PF::GaussBlurSiiPar,PF::GaussBlurSiiProc>() ); }
PF::ProcessorBase* PF::new_gaussblur()
{ return( new PF::Processor<PF::GaussBlurPar,PF::GaussBlurProc>() ); }

PF::ProcessorBase* PF::new_gradient()
{ return( new PF::Processor<PF::GradientPar,PF::Gradient>() ); }

PF::ProcessorBase* PF::new_hotpixels()
{ return new PF::Processor<PF::HotPixelsPar,PF::HotPixels>(); }

PF::ProcessorBase* PF::new_hsl_mask()
{ return new PF::Processor<PF::HSLMaskPar,PF::HSLMask>(); }

PF::ProcessorBase* PF::new_icc_transform()
{ return new PF::Processor<PF::ICCTransformPar,PF::ICCTransformProc>(); }

PF::ProcessorBase* PF::new_igv_demosaic()
{ return( new PF::Processor<PF::IgvDemosaicPar,PF::IgvDemosaicProc>() ); }

PF::ProcessorBase* PF::new_image_reader()
{ return new PF::Processor<PF::ImageReaderPar,PF::ImageReader>(); }

PF::ProcessorBase* PF::new_impulse_nr_algo()
{ return new PF::Processor<PF::ImpulseNR_RTAlgo_Par,PF::ImpulseNR_RTAlgo_Proc>(); }
PF::ProcessorBase* PF::new_impulse_nr()
{ return new PF::Processor<PF::ImpulseNRPar,PF::ImpulseNRProc>(); }

PF::ProcessorBase* PF::new_invert()
{ return( new PF::Processor<PF::InvertPar,PF::Invert>() ); }

PF::ProcessorBase* PF::new_lensfun_step()
{ return new PF::Processor<PF::LensFunParStep,PF::LensFunProc>(); }
PF::ProcessorBase* PF::new_lensfun()
{ return new PF::Processor<PF::LensFunPar,PF::LensFunProc>(); }

PF::ProcessorBase* PF::new_levels()
{ return new PF::Processor<PF::LevelsPar,PF::Levels>(); }

PF::ProcessorBase* PF::new_lmmse_demosaic()
{ return( new PF::Processor<PF::LMMSEDemosaicPar,PF::LMMSEDemosaicProc>() ); }

PF::ProcessorBase* PF::new_maxrgb()
{ return( new PF::Processor<PF::MaxRGBPar,PF::MaxRGBProc>() ); }

PF::ProcessorBase* PF::new_multiraw_developer()
{ return new PF::Processor<PF::MultiRawDeveloperPar,PF::MultiRawDeveloper>(); }

PF::ProcessorBase* PF::new_nlmeans_algo()
{ return( new PF::Processor<PF::NonLocalMeans_DTAlgo_Par,PF::NonLocalMeans_DTAlgo_Proc>() ); }
PF::ProcessorBase* PF::new_nlmeans()
{ return( new PF::Processor<PF::NonLocalMeansPar,PF::NonLocalMeansProc>() ); }

PF::ProcessorBase* PF::new_path_mask()
{ return( new PF::Processor<PF::PathMaskPar,PF::PathMask>() ); }

PF::ProcessorBase* PF::new_perspective()
{ return( new PF::Processor<PF::PerspectivePar,PF::PerspectiveProc>() ); }

PF::ProcessorBase* PF::new_raw_developer()
{ return new PF::Processor<PF::RawDeveloperPar,PF::RawDeveloper>(); }

PF::ProcessorBase* PF::new_raw_loader()
{ return new PF::Processor<PF::RawLoaderPar,PF::RawLoader>(); }

PF::ProcessorBase* PF::new_raw_output()
{ return new PF::Processor<PF::RawOutputPar,PF::RawOutput>(); }

PF::ProcessorBase* PF::new_raw_preprocessor()
{ return new PF::Processor<PF::RawPreprocessorPar,PF::RawPreprocessor>(); }

PF::ProcessorBase* PF::new_rcd_demosaic()
{ return( new PF::Processor<PF::RCDDemosaicPar,PF::RCDDemosaicProc>() ); }

PF::ProcessorBase* PF::new_scale()
{ return( new PF::Processor<PF::ScalePar,PF::ScaleProc>() ); }

PF::ProcessorBase* PF::new_shadows_highlights()
{ return new PF::Processor<PF::ShadowsHighlightsPar,PF::ShadowsHighlightsProc>(); }

PF::ProcessorBase* PF::new_sharpen()
{ return new PF::Processor<PF::SharpenPar,PF::SharpenProc>(); }

PF::ProcessorBase* PF::new_subtrimg_algo()
{ return new PF::Processor<PF::SubtrImgAlgoPar,PF::SubtrImgAlgoProc>(); }
PF::ProcessorBase* PF::new_subtrimg()
{ return new PF::Processor<PF::SubtrImgPar,PF::SubtrImgProc>(); }

PF::ProcessorBase* PF::new_threshold()
{ return( new PF::Processor<PF::ThresholdPar,PF::Threshold>() ); }

PF::ProcessorBase* PF::new_tone_mapping()
{ return new PF::Processor<PF::ToneMappingPar,PF::ToneMapping>(); }

PF::ProcessorBase* PF::new_tone_mapping_v2()
{ return new PF::Processor<PF::ToneMappingParV2,PF::ToneMappingV2>(); }

PF::ProcessorBase* PF::new_local_contrast()
{ return new PF::Processor<PF::LocalContrastPar,PF::LocalContrastProc>(); }

PF::ProcessorBase* PF::new_noise_generator()
{ return new PF::Processor<PF::NoiseGeneratorPar,PF::NoiseGeneratorProc>(); }

PF::ProcessorBase* PF::new_trcconv()
{ return( new PF::Processor<PF::TRCConvPar,PF::TRCConvProc>() ); }

#include "uniform.hh"
PF::ProcessorBase* PF::new_uniform()
{ return( new PF::Processor<PF::UniformPar,PF::Uniform>() ); }

#include "unsharp_mask.hh"
PF::ProcessorBase* PF::new_unsharp_mask()
{ return ( new PF::Processor<PF::UnsharpMaskPar,PF::UnsharpMask>() ); }

#include "vips_operation.hh"
PF::ProcessorBase* PF::new_vips_operation()
{ return( new PF::Processor<PF::VipsOperationPar,PF::VipsOperationProc>() ); }

#include "volume.hh"
PF::ProcessorBase* PF::new_volume()
{ return new PF::Processor<PF::VolumePar,PF::VolumeProc>(); }

#include "wavdec.hh"
PF::ProcessorBase* PF::new_wavdec_algo()
{ return new PF::Processor<PF::WavDecAlgoPar,PF::WavDecAlgoProc>(); }
PF::ProcessorBase* PF::new_wavdec()
{ return new PF::Processor<PF::WavDecPar,PF::WavDecProc>(); }

#include "white_balance.hh"
PF::ProcessorBase* PF::new_white_balance()
{ return new PF::Processor<PF::WhiteBalancePar,PF::WhiteBalance>(); }

#include "xtrans_demosaic.hh"
PF::ProcessorBase* PF::new_xtrans_demosaic()
{ return( new PF::Processor<PF::XTransDemosaicPar,PF::XTransDemosaicProc>() ); }
