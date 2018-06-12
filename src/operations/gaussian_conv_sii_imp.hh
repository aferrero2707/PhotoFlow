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


/**
 * \file gaussian_conv_sii_imp.hh
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

/**
 * \brief Gaussian convolution SII approximation
 * \param c         sii_coeffs created by sii_precomp()
 * \param dest      output convolved data
 * \param buffer    array with space for sii_buffer_size() samples
 * \param src       input, modified in-place if src = dest
 * \param N         number of samples
 * \param stride    stride between successive samples
 *
 * This routine performs stacked integral images approximation of Gaussian
 * convolution with half-sample symmetric boundary handling. The buffer array
 * is used to store the cumulative sum of the input, and must have space for
 * at least sii_buffer_size(c,N) samples.
 *
 * The convolution can be performed in-place by setting `src` = `dest` (the
 * source array is overwritten with the result). However, the buffer array
 * must be distinct from `src` and `dest`.
 */
/*
	Separated into horizontal and vertical steps and adapted to PhotoFlow 08/2014
 */
	template < OP_TEMPLATE_DEF_PREVIEW_SPEC > 
	void GaussBlurSiiProc<OP_TEMPLATE_IMP_PREVIEW_SPEC(true)>::sii_gaussian_conv_h(sii_coeffs& c, T *dest, double *buffer, const T *src,
																																							long start, long N, long width, int NCH)
	{
		//const int NCH = PF::ColorspaceInfo<CS>::NCH;

		//return;
    
		double accum[10/*NCH*/];
		long pad, pad2, N2, n, n2, n3, ch, stride2, x_start = 0, x_end = width-1, x_end2 = x_end*NCH, x0, x1, x2, x3;
		int k;

		assert(dest && buffer && src && N > 0);
    
		pad = c.radii[0] + 1; //pad=0;
		pad2 = pad * NCH;
		buffer += pad2;

		N2 = N * NCH;
		x0 = (start - pad) * NCH;
		x1 = start * NCH;
		x2 = (start + N - 1) * NCH;
		x3 = (start + N + pad - 1) * NCH;
		stride2 = NCH;

		for (ch = CHMIN; ch <= CHMAX; ++ch) accum[ch] = 0; 

		/* Compute cumulative sum of src over n = -pad,..., N + pad - 1. */
		//std::cout<<"1) left edge: x0="<<x0<<"  x3="<<x3<<"  x_start="<<x_start<<std::endl;
		for (n = x0, n2 = -pad2, n3 = -pad*stride2;
				 n < (x3<x_start ? x3 : x_start); 
				 n += NCH, n2 += NCH, n3 += stride2) {
			for (ch = CHMIN; ch <= CHMAX; ++ch) {
				accum[ch] += src[ch];
				//std::cout<<"1) buffer["<<n2<<"+"<<ch<<"]="<<accum[ch]<<std::endl;
				buffer[n2+ch] = accum[ch];
			}
		}

		//std::cout<<"2) image: n="<<n<<"  x3="<<x3<<"  x_end2="<<x_end2<<std::endl;
		for (; n <= (x3<x_end2 ? x3 : x_end2); n += NCH, n2 += NCH, n3 += stride2) {
			for (ch = CHMIN; ch <= CHMAX; ++ch) {
				accum[ch] += src[n3+ch];
				//std::cout<<"2) buffer["<<n2<<"+"<<ch<<"]="<<accum[ch]<<std::endl;
				buffer[n2+ch] = accum[ch];
			}
		}

		//std::cout<<"3) right edge: n="<<x_end2+1<<"  x3="<<x3<<"  x_end2="<<x_end2<<std::endl;
		for (n = x_end2+1; n <= x3; n += NCH, n2 += NCH, n3 += stride2) {
			for (ch = CHMIN; ch <= CHMAX; ++ch) {
				accum[ch] += src[N2+ch];
				//std::cout<<"3) buffer["<<n2<<"+"<<ch<<"]="<<accum[ch]<<std::endl;
				buffer[n2+ch] = accum[ch];
			}
		}
    
		long n1k[SII_MAX_K];
		long n2k[SII_MAX_K];
		/* Compute stacked box filters. */
		for (n = 0; n < N; ++n, dest += stride2) {
			for (k = 0; k < c.K; ++k) {
				n1k[k] = (n + c.radii[k]) * NCH;
				n2k[k] = (n - c.radii[k] - 1) * NCH;
			}
			for (ch = CHMIN; ch <= CHMAX; ++ch) {
				accum[ch] = c.weights[0] * (buffer[n1k[0] + ch]
																		- buffer[n2k[0] + ch]);
        
				for (k = 1; k < c.K; ++k)
					accum[ch] += c.weights[k] * (buffer[n1k[k] + ch]
																			 - buffer[n2k[k] + ch]);
        
				dest[ch] = (T)accum[ch];
			}
		}
			
		return;
	}



	template < OP_TEMPLATE_DEF_PREVIEW_SPEC > 
	void GaussBlurSiiProc<OP_TEMPLATE_IMP_PREVIEW_SPEC(true)>::sii_gaussian_conv_v(sii_coeffs& c, T *dest, double *buffer, const T *src,
																																							long start, long N, long heigth, long src_stride, long dest_stride, int NCH)
	{
		//const int NCH = PF::ColorspaceInfo<CS>::NCH;

		//return;
    
		double accum[10/*NCH*/];
		long pad, pad2, N2, n, n2, n3, ch, src_stride2, dest_stride2, y_start = 0, y_end = heigth-1, y_end2 = y_end*NCH, y0, y1, y2, y3;
		int k;

		assert(dest && buffer && src && N > 0);
    
		pad = c.radii[0] + 1; //pad=0;
		pad2 = pad * NCH;
		buffer += pad2;

		N2 = N * NCH;
		y0 = (start - pad) * NCH;
		y1 = start * NCH;
		y2 = (start + N - 1) * NCH;
		y3 = (start + N + pad - 1) * NCH;
		src_stride2 = src_stride*NCH;
		dest_stride2 = dest_stride*NCH;

		for (ch = CHMIN; ch <= CHMAX; ++ch) accum[ch] = 0; 

		/* Compute cumulative sum of src over n = -pad,..., N + pad - 1. */
		//std::cout<<"1) left edge: y0="<<y0<<"  y3="<<y3<<"  y_start="<<y_start<<std::endl;
		for (n = y0, n2 = -pad2, n3 = -pad*src_stride2;
				 n < (y3<y_start ? y3 : y_start); 
				 n += NCH, n2 += NCH, n3 += src_stride2) {
			for (ch = CHMIN; ch <= CHMAX; ++ch) {
				accum[ch] += src[ch];
				//std::cout<<"1) buffer["<<n2<<"+"<<ch<<"]="<<accum[ch]<<std::endl;
				buffer[n2+ch] = accum[ch];
			}
		}

		//std::cout<<"2) image: n="<<n<<"  y3="<<y3<<"  y_end2="<<y_end2<<std::endl;
		for (; n <= (y3<y_end2 ? y3 : y_end2); n += NCH, n2 += NCH, n3 += src_stride2) {
			for (ch = CHMIN; ch <= CHMAX; ++ch) {
				accum[ch] += src[n3+ch];
				//std::cout<<"2) buffer["<<n2<<"+"<<ch<<"]="<<accum[ch]<<std::endl;
				buffer[n2+ch] = accum[ch];
			}
		}

		//std::cout<<"3) right edge: n="<<y_end2+1<<"  y3="<<y3<<"  y_end2="<<y_end2<<std::endl;
		for (n = y_end2+1; n <= y3; n += NCH, n2 += NCH, n3 += src_stride2) {
			for (ch = CHMIN; ch <= CHMAX; ++ch) {
				accum[ch] += src[N2+ch];
				//std::cout<<"3) buffer["<<n2<<"+"<<ch<<"]="<<accum[ch]<<std::endl;
				buffer[n2+ch] = accum[ch];
			}
		}
    
		long n1k[SII_MAX_K];
		long n2k[SII_MAX_K];
		/* Compute stacked box filters. */
		for (n = 0; n < N; ++n, dest += dest_stride2) {
			for (k = 0; k < c.K; ++k) {
				n1k[k] = (n + c.radii[k]) * NCH;
				n2k[k] = (n - c.radii[k] - 1) * NCH;
			}
			for (ch = CHMIN; ch <= CHMAX; ++ch) {
				accum[ch] = c.weights[0] * (buffer[n1k[0] + ch]
																		- buffer[n2k[0] + ch]);
        
				for (k = 1; k < c.K; ++k)
					accum[ch] += c.weights[k] * (buffer[n1k[k] + ch]
																			 - buffer[n2k[k] + ch]);
        
				dest[ch] = (T)accum[ch];
			}
		}
			
		return;
	}
