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

#ifndef PF_ROI_HH
#define PF_ROI_HH


namespace PF
{

typedef struct _rp_roi_rect_t
{
  int left, top, width, height;
} rp_roi_rect_t;


typedef struct _rp_roi_t
{
  rp_roi_rect_t rect;
  int nchannels;
  float* buf;
  float*** data;
} rp_roi_t;


/* The rp_roi_new() allocates a RoI structure that allows to address pixels
 * in the specified rectangular region. The memory layout is such that
 * pixels from each image channel occupy a contiguous memory area.
 * The returned RoI structure must be freed with rp_roi_free().
 */
rp_roi_t* rp_roi_new(rp_roi_rect_t* rect, int nchannels);

/* The rp_roi_new_from_data() allocates a RoI structure that allows to address pixels
 * in the specified rectangular region. The memory layout is such that
 * pixels from each image channel occupy a contiguous memory area.
 * The pixels from an existing buffer are either referenced if the buffer is non-interleaved,
 * or copied if the image channels are interleaved.
 * The returned RoI structure must be freed with rp_roi_free().
 */
rp_roi_t* rp_roi_new_from_data(rp_roi_rect_t* rect, rp_roi_rect_t* rect_in, int nchannels, int rowstride, int interleaved, float* data, ...);

/* The rp_roi_free() function allows to free a previously allocated RoI structure.
 */
void rp_roi_free(rp_roi_t* roi);

}

#endif