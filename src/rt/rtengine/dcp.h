/*
*  This file is part of RawTherapee.
*
*  Copyright (c) 2012 Oliver Duis <www.oliverduis.de>
*
*  RawTherapee is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  RawTherapee is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PF_DCPSTORE_HH
#define PF_DCPSTORE_HH

#include <map>
#include <vector>
#include <array>
#include <memory>

#include <glibmm.h>

//#include "imagefloat.h"
#include "curves.h"
//#include "colortemp.h"
//#include "../rtgui/threadutils.h"

namespace rtengine
{

constexpr double xyz_prophoto[3][3] = {
    {0.7976749,  0.1351917,  0.0313534},
    {0.2880402,  0.7118741,  0.0000857},
    {0.0000000,  0.0000000,  0.8252100}
};

constexpr double prophoto_xyz[3][3] = {
    {1.3459433, -0.2556075, -0.0511118},
    { -0.5445989,  1.5081673,  0.0205351},
    {0.0000000,  0.0000000,  1.2118128}
};


template<typename ENUM>
typename std::underlying_type<ENUM>::type toUnderlying(ENUM value)
{
    return static_cast<typename std::underlying_type<ENUM>::type>(value);
}

class ColorTemp
{
  double temp;
public:
  ColorTemp(double t): temp(t) {}
  double getTemp() const {return temp;}
  double getTemp() { return temp;}
};


class DCPProfile
{
public:
  class ApplyState
  {
  public:
      ApplyState();
      ~ApplyState();

  private:
      struct Data;

      std::unique_ptr<Data> data;

      friend class DCPProfile;
  };

  struct Illuminants {
      short light_source_1;
      short light_source_2;
      double temperature_1;
      double temperature_2;
      bool will_interpolate;
  };

  struct HsdTableInfo {
      int hue_divisions;
      int sat_divisions;
      int val_divisions;
      int hue_step;
      int val_step;
      unsigned int array_count;
      bool srgb_gamma;
      struct {
          float h_scale;
          float s_scale;
          float v_scale;
          int max_hue_index0;
          int max_sat_index0;
          int max_val_index0;
          int hue_step;
          int val_step;
      } pc;
  };

  struct HsbModify {
      float hue_shift;
      float sat_scale;
      float val_scale;
  };

  using Triple = std::array<double, 3>;
  using Matrix = std::array<Triple, 3>;

    DCPProfile(const Glib::ustring& fname);
    ~DCPProfile();

    explicit operator bool() const;

    bool getHasToneCurve() const;
    bool getHasLookTable() const;
    bool getHasHueSatMap() const;
    bool getHasBaselineExposureOffset() const;

    const HsdTableInfo& get_delta_info() const { return delta_info; }
    const HsdTableInfo& get_look_info() const { return look_info; }
    const AdobeToneCurve& get_tone_curve() const { return tone_curve; }

    Illuminants getIlluminants() const;

    /*
    void apply(
        Imagefloat* img,
        int preferred_illuminant,
        const Glib::ustring& working_space,
        const ColorTemp& white_balance,
        const Triple& pre_mul,
        const Matrix& cam_wb_matrix,
        bool apply_hue_sat_map = true
    ) const;
    */
    void setStep2ApplyState(const Glib::ustring& working_space, bool use_tone_curve, bool apply_look_table, bool apply_baseline_exposure, ApplyState& as_out);
    void step2ApplyTile(float* r, float* g, float* b, int width, int height, int tile_width, const ApplyState& as_in) const;

    Matrix makeXyzCam(double cam_wb[3], double camWbMatrix[3][3], int preferred_illuminant) const;
    std::vector<HsbModify> makeHueSatMap(/*const ColorTemp& white_balance*/const double cam_wb[3], int preferred_illuminant) const;
    std::vector<HsbModify> get_look_table() { return look_table; }

    //void MakeXYZCAM(double cam_wb[3], double camWbMatrix[3][3], int preferredIlluminant, double (*mXYZCAM)[3]) const;
    //void Apply(Imagefloat *pImg, int preferredIlluminant, Glib::ustring workingSpace, ColorTemp &wb, double pre_mul[3], double camMatrix[3][3], bool useToneCurve = false, bool applyHueSatMap = true, bool applyLookTable = false) const;
    //void setStep2ApplyState(Glib::ustring workingSpace, bool useToneCurve, bool applyLookTable, bool applyBaselineExposure);
    //void step2ApplyTile(float *r, float *g, float *b, int width, int height, int tileWidth) const;

private:
    Matrix findXyztoCamera(const std::array<double, 2>& white_xy, int preferred_illuminant) const;
    double camwb_to_temp(const double cam_wb[3], int preferred_illuminant) const;
    std::array<double, 2> neutralToXy(const Triple& neutral, int preferred_illuminant) const;
    //Matrix makeXyzCam(double cam_wb[3], double camWbMatrix[3][3], int preferred_illuminant) const;
    //std::vector<HsbModify> makeHueSatMap(/*const ColorTemp& white_balance*/const double cam_wb[3], int preferred_illuminant) const;

    Matrix color_matrix_1;
    Matrix color_matrix_2;
    bool has_color_matrix_1;
    bool has_color_matrix_2;
    bool has_forward_matrix_1;
    bool has_forward_matrix_2;
    bool has_tone_curve;
    bool has_baseline_exposure_offset;
    bool will_interpolate;
    Matrix forward_matrix_1;
    Matrix forward_matrix_2;
    double temperature_1;
    double temperature_2;
    double baseline_exposure_offset;
    std::vector<HsbModify> deltas_1;
    std::vector<HsbModify> deltas_2;
    std::vector<HsbModify> look_table;
    HsdTableInfo delta_info;
    HsdTableInfo look_info;
    short light_source_1;
    short light_source_2;

    AdobeToneCurve tone_curve;
};

void hsdApply(const DCPProfile::HsdTableInfo& table_info, const std::vector<DCPProfile::HsbModify>& table_base, float& h, float& s, float& v);

#ifdef ___TRUE___
class DCPStore
{
    //MyMutex mtx;

    // these contain standard profiles from RT. keys are all in uppercase, file path is value
    std::map<Glib::ustring, Glib::ustring> fileStdProfiles;

    // Maps file name to profile as cache
    std::map<Glib::ustring, DCPProfile*> profileCache;

public:
    void init(Glib::ustring rtProfileDir);

    bool isValidDCPFileName(Glib::ustring filename) const;

    DCPProfile* getProfile(Glib::ustring filename, bool isRTProfile = false);
    DCPProfile* getStdProfile(Glib::ustring camShortName);

    static DCPStore* getInstance();
};

#define dcpStore DCPStore::getInstance()
#endif
}
#endif
