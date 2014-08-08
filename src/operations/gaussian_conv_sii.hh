/**
 * \file gaussian_conv_sii.h
 * \brief Gaussian convolution using stacked integral images
 * \author Pascal Getreuer <getreuer@cmla.ens-cachan.fr>
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

/**
 * \defgroup sii_gaussian Stacked integral images Gaussian convolution
 * \brief A sum of box filter responses, rather than a cascade.
 *
 * This code implements stacked integral images Gaussian convolution
 * approximation introduced by Bhatia, Snyder, and Bilbro and refined by
 * Elboher and Werman. The Gaussian is approximated as
 * \f[ u_n = \sum_{k=1}^K w_k (s_{n+r_k} - s_{n-r_k-1}), \quad
       s_n = \sum_{m\le n} f_n, \f]
 * where the kth term of the sum is effectively a box filter of radius
 * \f$ r_k \f$.
 *
 * The process to use these functions is the following:
 *    -# sii_precomp() to precompute coefficients for the convolution
 *    -# sii_gaussian_conv() or sii_gaussian_conv_image() to perform
 *       the convolution itself (may be called multiple times if desired)
 *
 * The function sii_buffer_size() should be used to determine the minimum
 * required buffer size.
 *
 * \par Example
\code
    sii_coeffs c;
    num *buffer;
    
    sii_precomp(&c, sigma, K);
    buffer = (num *)malloc(sizeof(num) * sii_buffer_size(c, N));
    sii_gaussian_conv(c, dest, buffer, src, N, stride);
    free(buffer);
\endcode
 *
 * \par References
 *  - A. Bhatia, W.E. Snyder, G. Bilbro, "Stacked Integral Image," IEEE
 *    International Conference on Robotics and Automation (ICRA),
 *    pp. 1530-1535, 2010. http://dx.doi.org/10.1109/ROBOT.2010.5509400
 *  - E. Elboher, M. Werman, "Efficient and Accurate Gaussian Image Filtering
 *    Using Running Sums," Computing Research Repository,
 *    vol. abs/1107.4958, 2011. http://arxiv.org/abs/1107.4958
 *
 * \{
 */

#ifndef GAUSSIAN_CONV_SII_H
#define GAUSSIAN_CONV_SII_H

//#include "num.h"
//#include "gaussblur.hh"

/** \brief Minimum SII filter order */
#define SII_MIN_K       3
/** \brief Maximum SII filter order */
#define SII_MAX_K       5
/** \brief Test whether a given K value is a valid SII filter order */
#define SII_VALID_K(K)  (SII_MIN_K <= (K) && (K) <= SII_MAX_K)

/** \brief Parameters for stacked integral images Gaussian approximation */
typedef struct sii_coeffs_
{
    double weights[SII_MAX_K];     /**< Box weights     */
    long radii[SII_MAX_K];      /**< Box radii       */
    int K;                      /**< Number of boxes */
} sii_coeffs;

void sii_precomp(sii_coeffs *c, double sigma, int K);
long sii_buffer_size(sii_coeffs c, long N);



/**
 * \brief 2D Gaussian convolution SII approximation
 * \param c             sii_coeffs created by sii_precomp()
 * \param dest          output convolved data
 * \param buffer        array with space for sii_buffer_size() samples
 * \param src           image to be convolved, overwritten if src = dest
 * \param width         image width
 * \param height        image height
 * \param num_channels  number of image channels
 *
 * Similar to sii_gaussian_conv(), this routine approximates 2D Gaussian
 * convolution with stacked integral images. The buffer array must have space
 * for at least sii_buffer_size(c,max(width,height)) samples.
 *
 * The convolution can be performed in-place by setting `src` = `dest` (the
 * source array is overwritten with the result). However, the buffer array
 * must be distinct from `src` and `dest`.
 */
template<class num>
void sii_gaussian_conv_image(sii_coeffs c, num *dest, num *buffer,
														 const num *src, int width, int height, int num_channels)
{
	long num_pixels = ((long)width) * ((long)height);
	int x, y, channel;
    
	assert(dest && buffer && src && num_pixels > 0);
    
	/* Loop over the image channels. */
	for (channel = 0; channel < num_channels; ++channel)
    {
			num *dest_y = dest;
			const num *src_y = src;
        
			/* Filter each row of the channel. */
			for (y = 0; y < height; ++y)
        {
					//sii_gaussian_conv(c,
					//									dest_y, buffer, src_y, width, 1);
					dest_y += width;
					src_y += width;
        }
        
			/* Filter each column of the channel. */
			for (x = 0; x < width; ++x)
				//sii_gaussian_conv(c,
				//									dest + x, buffer, dest + x, height, width);
        
			dest += num_pixels;
			src += num_pixels;
    }
    
	return;
}

/** \} */
#endif /* GAUSSIAN_CONV_SII_H */
