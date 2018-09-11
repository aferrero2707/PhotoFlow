/*
 * This file is part of librtprocess.
 *
 * Copyright (c) 2018 Carlo Vaccari
 *
 * librtprocess is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the license, or
 * (at your option) any later version.
 *
 * librtprocess is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with librtprocess.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <array>

#include "librtprocess.h"

namespace librtprocess
{

    void bayerborder_demosaic(int winw, int winh, int lborders, const array2D<float> &rawData, array2D<float> &red, array2D<float> &green, array2D<float> &blue, const ColorFilterArray &cfarray);
    void xtransborder_demosaic(int winw, int winh, int border, const array2D<float> &rawData, array2D<float> &red, array2D<float> &green, array2D<float> &blue, int xtrans[6][6]);

}//namespace
