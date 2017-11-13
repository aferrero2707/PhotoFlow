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

#include "../../base/processor_imp.hh"


// Legacy operations

#include "brightness_contrast.hh"
PF::ProcessorBase* PF::new_brightness_contrast()
{ return ( new PF::Processor<PF::BrightnessContrastPar,PF::BrightnessContrast>() ); }


#include "hue_saturation.hh"
PF::ProcessorBase* PF::new_hue_saturation()
{ return new PF::Processor<PF::HueSaturationPar,PF::HueSaturation>(); }

#include "raw_developer.hh"
PF::ProcessorBase* PF::new_raw_developer_v1()
{ return new PF::Processor<PF::RawDeveloperV1Par,PF::RawDeveloperV1>(); }

#include "raw_output.hh"
PF::ProcessorBase* PF::new_raw_output_v1()
{ return new PF::Processor<PF::RawOutputV1Par,PF::RawOutputV1>(); }

#include "raw_preprocessor.hh"
PF::ProcessorBase* PF::new_raw_preprocessor_v1()
{ return new PF::Processor<PF::RawPreprocessorV1Par,PF::RawPreprocessorV1>(); }
