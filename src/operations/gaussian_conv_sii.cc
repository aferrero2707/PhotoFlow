/**
 * \file gaussian_conv_sii.c
 * \brief Gaussian convolution using stacked integral images
 * \author Pascal Getreuer <getreuer@cmla.ens-cachan.fr>
 * http://www.ipol.im/pub/art/2013/87/
 *
 * This code implements stacked integral images Gaussian convolution approximation 
 * introduced by Bhatia, Snyder, and Bilbro (http://dx.doi.org/10.1109/ROBOT.2010.5509400)
 * and refined by Elboher and Werman (http://arxiv.org/abs/1107.4958).
 *
 * Modified for integration into PhotoFlow
 *
 * Copyright (c) 2012-2013, Pascal Getreuer
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under, at your option, the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version, or the terms of the
 * simplified BSD license.
 *
 * You should have received a copy of these licenses along with this program.
 * If not, see <http://www.gnu.org/licenses/> and
 * <http://www.opensource.org/licenses/bsd-license.html>.
 */

#include "gaussian_conv_sii.hh"
#include <assert.h>
#include <stdio.h>
//#include "filter_util.h"

#ifndef M_PI
/** \brief The constant pi */
#define M_PI        3.14159265358979323846264338327950288
#endif

/**
 * \brief Precompute filter coefficients for SII Gaussian convolution
 * \param c         sii_coeffs pointer to hold precomputed coefficients
 * \param sigma     Gaussian standard deviation
 * \param K         number of boxes = 3, 4, or 5
 * \return 1 on success, 0 on failure
 *
 * This routine reads Elboher and Werman's optimal SII radii and weights for
 * reference standard deviation \f$ \sigma_0 = 100/\pi \f$ from a table and
 * scales them to the specified value of sigma.
 */
void sii_precomp(sii_coeffs *c, double sigma, int K)
{
    /* Elboher and Werman's optimal radii and weights. */
    const double sigma0 = 100.0 / M_PI;
    static const short radii0[SII_MAX_K - SII_MIN_K + 1][SII_MAX_K] =
        {{76, 46, 23, 0, 0},
         {82, 56, 37, 19, 0},
         {85, 61, 44, 30, 16}};
    static const float weights0[SII_MAX_K - SII_MIN_K + 1][SII_MAX_K] =
        {{0.1618f, 0.5502f, 0.9495f, 0, 0},
         {0.0976f, 0.3376f, 0.6700f, 0.9649f, 0},
         {0.0739f, 0.2534f, 0.5031f, 0.7596f, 0.9738f}};

    const int i = K - SII_MIN_K;
    double sum;
    int k;
    
    assert(c && sigma > 0 && SII_VALID_K(K));
    c->K = K;
    
    for (k = 0, sum = 0; k < K; ++k)
    {
        c->radii[k] = (long)(radii0[i][k] * (sigma / sigma0) + 0.5);
        sum += weights0[i][k] * (2 * c->radii[k] + 1);
    }
    
    for (k = 0; k < K; ++k)
        c->weights[k] = (double)(weights0[i][k] / sum);
    
    return;
}

/**
 * \brief Determines the buffer size needed for SII Gaussian convolution
 * \param c     sii_coeffs created by sii_precomp()
 * \param N     number of samples
 * \return required buffer size in units of num samples
 *
 * This routine determines the minimum size of the buffer needed for use in
 * sii_gaussian_conv() or sii_gaussian_conv_image(). This size is the length
 * of the signal (or in 2D, max(width, height)) plus the twice largest box
 * radius, for padding.
 */
long sii_buffer_size(sii_coeffs c, long N)
{
    long pad = c.radii[0] + 1;
    return N + 2 * pad;
}

