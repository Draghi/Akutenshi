/**
Copyright (c) 2017 HiFi-LoFi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **/

/**
 * Contents of this file are from the HiFi-LoFi project but have been modified for its use in this engine.
 * Mostly this just consisted of creating free-floating functions and reformatting the code. The included
 * code will be sub-licensed under Akutenshi's license. Removing references to Akutenshi code will, of
 * course, allow to be used wholely under the MIT license.
 **/

#ifndef AK_MATH_FOURIERTRANSFORM_HPP_
#define AK_MATH_FOURIERTRANSFORM_HPP_

#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/util/Bits.hpp>
#include <crtdefs.h>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace akm {
	namespace internal {
		inline void makewt(int32 nw, int32* ip, fpDouble* w);
		inline void makect(int32 nc, int32* ip, fpDouble* c);

		inline void rdft(int32 n, int32 isgn, fpDouble* a, int32* ip, fpDouble* w);

		template<typename typeDst_t, typename typeSrc_t, typename typeFac_t> void scaleBuffer(typeDst_t* dest, const typeSrc_t* src, const typeFac_t factor, size_t len) {
			for (size_t i = 0; i < len; ++i) dest[i] = static_cast<typeDst_t>(static_cast<typeFac_t>(src[i]) * factor);
		}
	}

	inline akSize calcFFTOutputSize(akSize inputSize)  { return aku::nearestPowerOfTwo(inputSize)/2; }

	class FTTBuffer;

	bool fft(const scalar_t* signal, scalar_t* rPart, scalar_t* iPart, akSize count, FTTBuffer& buffer);
	bool ifft(scalar_t* signal, const scalar_t* rPart, const scalar_t* iPart, akSize count, FTTBuffer& buffer);

	class FTTBuffer final {
		friend bool fft(const scalar_t*, scalar_t*, scalar_t*, akSize, FTTBuffer&);
		friend bool ifft(scalar_t*, const scalar_t*, const scalar_t*, akSize, FTTBuffer&);
		private:
			std::vector<int32> ip;
			std::vector<fpDouble> w;
			std::vector<fpDouble> buffer;
			akSize size;

		public:
			FTTBuffer(akSize inSize) : size(inSize) {
				if (!aku::isPowerOfTwo(size)) throw std::logic_error("FFT only works on POT buffer sizes.");
				ip.resize(2 + static_cast<int32>(std::sqrt(static_cast<fpDouble>(size))));
				w.resize(size / 2);
				buffer.resize(size);

				const int32 size4 = static_cast<int32>(buffer.size()) / 4;
				internal::makewt(size4, ip.data(), w.data());
				internal::makect(size4, ip.data(), w.data() + size4);
			}

			akSize bufSize() const { return size; }
	};

	inline bool fft(const scalar_t* signal, scalar_t* rPart, scalar_t* iPart, akSize count, FTTBuffer& buffer) {
		if (count != buffer.bufSize()) return false;

		// Convert internal representation
		for (size_t i = 0; i < buffer.size; ++i) buffer.buffer[i] = static_cast<fpDouble>(signal[i]);

		// FTT
		internal::rdft(static_cast<int32>(buffer.size), +1, buffer.buffer.data(), buffer.ip.data(), buffer.w.data());

		/* Unpack data into output */ {
			const fpDouble* b = buffer.buffer.data();
			const fpDouble* bEnd = b + buffer.size;
			scalar_t* r = rPart;
			scalar_t* i = iPart;
			while (b != bEnd) {
				*(r++) = static_cast<scalar_t>(*(b++));
				*(i++) = static_cast<scalar_t>(-(*(b++)));
			}

			const size_t size2 = buffer.size/2;
			rPart[size2] = -iPart[0];
			iPart[0] = 0.0;
			iPart[size2] = 0.0;
		}

		return true;
	}

	inline bool fft(const scalar_t* signal, scalar_t* rPart, scalar_t* iPart, akSize count) {
		if (!aku::isPowerOfTwo(count)) return false;
		FTTBuffer buffer(count);
		return fft(signal, rPart, iPart, count, buffer);
	}

	inline bool ifft(scalar_t* signal, const scalar_t* rPart, const scalar_t* iPart, akSize count, FTTBuffer& buffer) {
		if (count != buffer.bufSize()) return false;

		/* Convert internal representation */ {
			fpDouble* b = buffer.buffer.data();
			fpDouble* bEnd = b + buffer.size;
			const scalar_t* r = rPart;
			const scalar_t* i = iPart;
			while (b != bEnd) {
				*(b++) =  static_cast<fpDouble>(*(r++));
				*(b++) = -static_cast<fpDouble>(*(i++));
			}
			buffer.buffer[1] = rPart[buffer.size/2];
		}

		// IFTT
		internal::rdft(static_cast<int32>(buffer.size), -1, buffer.buffer.data(), buffer.ip.data(), buffer.w.data());

		// Convert to normalized signal
		internal::scaleBuffer(signal, buffer.buffer.data(), 2.0 / static_cast<fpDouble>(buffer.buffer.size()), buffer.buffer.size());

		return true;
	}

	inline bool ifft(scalar_t* signal, const scalar_t* rPart, const scalar_t* iPart, akSize count) {
		if (!aku::isPowerOfTwo(count)) return false;
		FTTBuffer buffer(count);
		return ifft(signal, rPart, iPart, count, buffer);
	}

}

namespace akm {
	namespace internal {
		inline void cft1st(int32 n, fpDouble* a, fpDouble* w);
		inline void cftmdl(int32 n, int32 l, fpDouble* a, fpDouble* w);
		inline void bitrv2(int32 n, int32* ip, fpDouble* a);
		inline void cftfsub(int32 n, fpDouble* a, fpDouble* w);
		inline void cftbsub(int32 n, fpDouble* a, fpDouble* w);
		inline void rftfsub(int32 n, fpDouble* a, int32 nc, fpDouble* c);
		inline void rftbsub(int32 n, fpDouble* a, int32 nc, fpDouble* c);

		/* -------- initializing routines -------- */
		inline void makewt(int32 nw, int32* ip, fpDouble* w) {
			int32 j, nwh;
			fpDouble delta, x, y;

			ip[0] = nw;
			ip[1] = 1;
			if (nw > 2) {
				nwh = nw >> 1;
				delta = std::atan(1.0) / nwh;
				w[0] = 1;
				w[1] = 0;
				w[nwh] = std::cos(delta * nwh);
				w[nwh + 1] = w[nwh];
				if (nwh > 2) {
					for (j = 2; j < nwh; j += 2) {
						x = std::cos(delta * j);
						y = std::sin(delta * j);
						w[j] = x;
						w[j + 1] = y;
						w[nw - j] = y;
						w[nw - j + 1] = x;
					}
					bitrv2(nw, ip + 2, w);
				}
			}
		}


		inline void makect(int32 nc, int32* ip, fpDouble* c) {
			int32 j, nch;
			fpDouble delta;

			ip[1] = nc;
			if (nc > 1) {
				nch = nc >> 1;
				delta = std::atan(1.0) / nch;
				c[0] = std::cos(delta * nch);
				c[nch] = 0.5 * c[0];
				for (j = 1; j < nch; j++) {
					c[j] = 0.5 * std::cos(delta * j);
					c[nc - j] = 0.5 * std::sin(delta * j);
				}
			}
		}

		/* -------- main routines -------- */
		inline void rdft(int32 n, int32 isgn, fpDouble* a, int32* ip, fpDouble* w) {
			int32 nw = ip[0];
			int32 nc = ip[1];

			if (isgn >= 0) {
				if (n > 4) {
					bitrv2(n, ip + 2, a);
					cftfsub(n, a, w);
					rftfsub(n, a, nc, w + nw);
				} else if (n == 4) {
					cftfsub(n, a, w);
				}
				fpDouble xi = a[0] - a[1];
				a[0] += a[1];
				a[1] = xi;
			} else {
				a[1] = 0.5 * (a[0] - a[1]);
				a[0] -= a[1];
				if (n > 4) {
					rftbsub(n, a, nc, w + nw);
					bitrv2(n, ip + 2, a);
					cftbsub(n, a, w);
				} else if (n == 4) {
					cftfsub(n, a, w);
				}
			}
		}

		/* -------- child routines -------- */
		inline void cft1st(int32 n, fpDouble* a, fpDouble* w) {
			int32 j, k1, k2;
			fpDouble wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
			fpDouble x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

			x0r = a[0] + a[2];
			x0i = a[1] + a[3];
			x1r = a[0] - a[2];
			x1i = a[1] - a[3];
			x2r = a[4] + a[6];
			x2i = a[5] + a[7];
			x3r = a[4] - a[6];
			x3i = a[5] - a[7];
			a[0] = x0r + x2r;
			a[1] = x0i + x2i;
			a[4] = x0r - x2r;
			a[5] = x0i - x2i;
			a[2] = x1r - x3i;
			a[3] = x1i + x3r;
			a[6] = x1r + x3i;
			a[7] = x1i - x3r;
			wk1r = w[2];
			x0r = a[8] + a[10];
			x0i = a[9] + a[11];
			x1r = a[8] - a[10];
			x1i = a[9] - a[11];
			x2r = a[12] + a[14];
			x2i = a[13] + a[15];
			x3r = a[12] - a[14];
			x3i = a[13] - a[15];
			a[8] = x0r + x2r;
			a[9] = x0i + x2i;
			a[12] = x2i - x0i;
			a[13] = x0r - x2r;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			a[10] = wk1r * (x0r - x0i);
			a[11] = wk1r * (x0r + x0i);
			x0r = x3i + x1r;
			x0i = x3r - x1i;
			a[14] = wk1r * (x0i - x0r);
			a[15] = wk1r * (x0i + x0r);
			k1 = 0;
			for (j = 16; j < n; j += 16) {
				k1 += 2;
				k2 = 2 * k1;
				wk2r = w[k1];
				wk2i = w[k1 + 1];
				wk1r = w[k2];
				wk1i = w[k2 + 1];
				wk3r = wk1r - 2 * wk2i * wk1i;
				wk3i = 2 * wk2i * wk1r - wk1i;
				x0r = a[j] + a[j + 2];
				x0i = a[j + 1] + a[j + 3];
				x1r = a[j] - a[j + 2];
				x1i = a[j + 1] - a[j + 3];
				x2r = a[j + 4] + a[j + 6];
				x2i = a[j + 5] + a[j + 7];
				x3r = a[j + 4] - a[j + 6];
				x3i = a[j + 5] - a[j + 7];
				a[j] = x0r + x2r;
				a[j + 1] = x0i + x2i;
				x0r -= x2r;
				x0i -= x2i;
				a[j + 4] = wk2r * x0r - wk2i * x0i;
				a[j + 5] = wk2r * x0i + wk2i * x0r;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j + 2] = wk1r * x0r - wk1i * x0i;
				a[j + 3] = wk1r * x0i + wk1i * x0r;
				x0r = x1r + x3i;
				x0i = x1i - x3r;
				a[j + 6] = wk3r * x0r - wk3i * x0i;
				a[j + 7] = wk3r * x0i + wk3i * x0r;
				wk1r = w[k2 + 2];
				wk1i = w[k2 + 3];
				wk3r = wk1r - 2 * wk2r * wk1i;
				wk3i = 2 * wk2r * wk1r - wk1i;
				x0r = a[j + 8] + a[j + 10];
				x0i = a[j + 9] + a[j + 11];
				x1r = a[j + 8] - a[j + 10];
				x1i = a[j + 9] - a[j + 11];
				x2r = a[j + 12] + a[j + 14];
				x2i = a[j + 13] + a[j + 15];
				x3r = a[j + 12] - a[j + 14];
				x3i = a[j + 13] - a[j + 15];
				a[j + 8] = x0r + x2r;
				a[j + 9] = x0i + x2i;
				x0r -= x2r;
				x0i -= x2i;
				a[j + 12] = -wk2i * x0r - wk2r * x0i;
				a[j + 13] = -wk2i * x0i + wk2r * x0r;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j + 10] = wk1r * x0r - wk1i * x0i;
				a[j + 11] = wk1r * x0i + wk1i * x0r;
				x0r = x1r + x3i;
				x0i = x1i - x3r;
				a[j + 14] = wk3r * x0r - wk3i * x0i;
				a[j + 15] = wk3r * x0i + wk3i * x0r;
			}
		}

		inline void cftmdl(int32 n, int32 l, fpDouble* a, fpDouble* w) {
			int32 j, j1, j2, j3, k, k1, k2, m, m2;
			fpDouble wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
			fpDouble x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

			m = l << 2;
			for (j = 0; j < l; j += 2) {
				j1 = j + l;
				j2 = j1 + l;
				j3 = j2 + l;
				x0r = a[j] + a[j1];
				x0i = a[j + 1] + a[j1 + 1];
				x1r = a[j] - a[j1];
				x1i = a[j + 1] - a[j1 + 1];
				x2r = a[j2] + a[j3];
				x2i = a[j2 + 1] + a[j3 + 1];
				x3r = a[j2] - a[j3];
				x3i = a[j2 + 1] - a[j3 + 1];
				a[j] = x0r + x2r;
				a[j + 1] = x0i + x2i;
				a[j2] = x0r - x2r;
				a[j2 + 1] = x0i - x2i;
				a[j1] = x1r - x3i;
				a[j1 + 1] = x1i + x3r;
				a[j3] = x1r + x3i;
				a[j3 + 1] = x1i - x3r;
			}
			wk1r = w[2];
			for (j = m; j < l + m; j += 2) {
				j1 = j + l;
				j2 = j1 + l;
				j3 = j2 + l;
				x0r = a[j] + a[j1];
				x0i = a[j + 1] + a[j1 + 1];
				x1r = a[j] - a[j1];
				x1i = a[j + 1] - a[j1 + 1];
				x2r = a[j2] + a[j3];
				x2i = a[j2 + 1] + a[j3 + 1];
				x3r = a[j2] - a[j3];
				x3i = a[j2 + 1] - a[j3 + 1];
				a[j] = x0r + x2r;
				a[j + 1] = x0i + x2i;
				a[j2] = x2i - x0i;
				a[j2 + 1] = x0r - x2r;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j1] = wk1r * (x0r - x0i);
				a[j1 + 1] = wk1r * (x0r + x0i);
				x0r = x3i + x1r;
				x0i = x3r - x1i;
				a[j3] = wk1r * (x0i - x0r);
				a[j3 + 1] = wk1r * (x0i + x0r);
			}
			k1 = 0;
			m2 = 2 * m;
			for (k = m2; k < n; k += m2) {
				k1 += 2;
				k2 = 2 * k1;
				wk2r = w[k1];
				wk2i = w[k1 + 1];
				wk1r = w[k2];
				wk1i = w[k2 + 1];
				wk3r = wk1r - 2 * wk2i * wk1i;
				wk3i = 2 * wk2i * wk1r - wk1i;
				for (j = k; j < l + k; j += 2) {
					j1 = j + l;
					j2 = j1 + l;
					j3 = j2 + l;
					x0r = a[j] + a[j1];
					x0i = a[j + 1] + a[j1 + 1];
					x1r = a[j] - a[j1];
					x1i = a[j + 1] - a[j1 + 1];
					x2r = a[j2] + a[j3];
					x2i = a[j2 + 1] + a[j3 + 1];
					x3r = a[j2] - a[j3];
					x3i = a[j2 + 1] - a[j3 + 1];
					a[j] = x0r + x2r;
					a[j + 1] = x0i + x2i;
					x0r -= x2r;
					x0i -= x2i;
					a[j2] = wk2r * x0r - wk2i * x0i;
					a[j2 + 1] = wk2r * x0i + wk2i * x0r;
					x0r = x1r - x3i;
					x0i = x1i + x3r;
					a[j1] = wk1r * x0r - wk1i * x0i;
					a[j1 + 1] = wk1r * x0i + wk1i * x0r;
					x0r = x1r + x3i;
					x0i = x1i - x3r;
					a[j3] = wk3r * x0r - wk3i * x0i;
					a[j3 + 1] = wk3r * x0i + wk3i * x0r;
				}
				wk1r = w[k2 + 2];
				wk1i = w[k2 + 3];
				wk3r = wk1r - 2 * wk2r * wk1i;
				wk3i = 2 * wk2r * wk1r - wk1i;
				for (j = k + m; j < l + (k + m); j += 2) {
					j1 = j + l;
					j2 = j1 + l;
					j3 = j2 + l;
					x0r = a[j] + a[j1];
					x0i = a[j + 1] + a[j1 + 1];
					x1r = a[j] - a[j1];
					x1i = a[j + 1] - a[j1 + 1];
					x2r = a[j2] + a[j3];
					x2i = a[j2 + 1] + a[j3 + 1];
					x3r = a[j2] - a[j3];
					x3i = a[j2 + 1] - a[j3 + 1];
					a[j] = x0r + x2r;
					a[j + 1] = x0i + x2i;
					x0r -= x2r;
					x0i -= x2i;
					a[j2] = -wk2i * x0r - wk2r * x0i;
					a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
					x0r = x1r - x3i;
					x0i = x1i + x3r;
					a[j1] = wk1r * x0r - wk1i * x0i;
					a[j1 + 1] = wk1r * x0i + wk1i * x0r;
					x0r = x1r + x3i;
					x0i = x1i - x3r;
					a[j3] = wk3r * x0r - wk3i * x0i;
					a[j3 + 1] = wk3r * x0i + wk3i * x0r;
				}
			}
		}

		inline void bitrv2(int32 n, int32* ip, fpDouble* a) {
			int32 j, j1, k, k1, l, m, m2;
			fpDouble xr, xi, yr, yi;

			ip[0] = 0;
			l = n;
			m = 1;
			while ((m << 3) < l) {
				l >>= 1;
				for (j = 0; j < m; j++) ip[m + j] = ip[j] + l;
				m <<= 1;
			}
			m2 = 2 * m;
			if ((m << 3) == l) {
				for (k = 0; k < m; k++) {
					for (j = 0; j < k; j++) {
						j1 = 2 * j + ip[k];
						k1 = 2 * k + ip[j];
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
						j1 += m2;
						k1 += 2 * m2;
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
						j1 += m2;
						k1 -= m2;
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
						j1 += m2;
						k1 += 2 * m2;
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
					}
					j1 = 2 * k + m2 + ip[k];
					k1 = j1 + m2;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
				}
			} else {
				for (k = 1; k < m; k++) {
					for (j = 0; j < k; j++) {
						j1 = 2 * j + ip[k];
						k1 = 2 * k + ip[j];
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
						j1 += m2;
						k1 += m2;
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
					}
				}
			}
		}


		inline void cftfsub(int32 n, fpDouble* a, fpDouble* w) {
			int32 j, j1, j2, j3, l;
			fpDouble x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

			l = 2;

			if (n > 8) {
				cft1st(n, a, w);
				l = 8;
				while ((l << 2) < n) {
					cftmdl(n, l, a, w);
					l <<= 2;
				}
			}

			if ((l << 2) == n) {
				for (j = 0; j < l; j += 2) {
					j1 = j + l;
					j2 = j1 + l;
					j3 = j2 + l;
					x0r = a[j] + a[j1];
					x0i = a[j + 1] + a[j1 + 1];
					x1r = a[j] - a[j1];
					x1i = a[j + 1] - a[j1 + 1];
					x2r = a[j2] + a[j3];
					x2i = a[j2 + 1] + a[j3 + 1];
					x3r = a[j2] - a[j3];
					x3i = a[j2 + 1] - a[j3 + 1];
					a[j] = x0r + x2r;
					a[j + 1] = x0i + x2i;
					a[j2] = x0r - x2r;
					a[j2 + 1] = x0i - x2i;
					a[j1] = x1r - x3i;
					a[j1 + 1] = x1i + x3r;
					a[j3] = x1r + x3i;
					a[j3 + 1] = x1i - x3r;
				}
			} else {
				for (j = 0; j < l; j += 2) {
					j1 = j + l;
					x0r = a[j] - a[j1];
					x0i = a[j + 1] - a[j1 + 1];
					a[j] += a[j1];
					a[j + 1] += a[j1 + 1];
					a[j1] = x0r;
					a[j1 + 1] = x0i;
				}
			}
		}


		inline void cftbsub(int32 n, fpDouble* a, fpDouble* w) {
			int32 j, j1, j2, j3, l;
			fpDouble x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

			l = 2;

			if (n > 8) {
				cft1st(n, a, w);
				l = 8;
				while ((l << 2) < n) {
					cftmdl(n, l, a, w);
					l <<= 2;
				}
			}

			if ((l << 2) == n) {
				for (j = 0; j < l; j += 2) {
					j1 = j + l;
					j2 = j1 + l;
					j3 = j2 + l;
					x0r = a[j] + a[j1];
					x0i = -a[j + 1] - a[j1 + 1];
					x1r = a[j] - a[j1];
					x1i = -a[j + 1] + a[j1 + 1];
					x2r = a[j2] + a[j3];
					x2i = a[j2 + 1] + a[j3 + 1];
					x3r = a[j2] - a[j3];
					x3i = a[j2 + 1] - a[j3 + 1];
					a[j] = x0r + x2r;
					a[j + 1] = x0i - x2i;
					a[j2] = x0r - x2r;
					a[j2 + 1] = x0i + x2i;
					a[j1] = x1r - x3i;
					a[j1 + 1] = x1i - x3r;
					a[j3] = x1r + x3i;
					a[j3 + 1] = x1i + x3r;
				}
			} else {
				for (j = 0; j < l; j += 2) {
					j1 = j + l;
					x0r = a[j] - a[j1];
					x0i = -a[j + 1] + a[j1 + 1];
					a[j] += a[j1];
					a[j + 1] = -a[j + 1] - a[j1 + 1];
					a[j1] = x0r;
					a[j1 + 1] = x0i;
				}
			}
		}

		inline void rftfsub(int32 n, fpDouble* a, int32 nc, fpDouble* c) {
			int32 j, k, kk, ks, m;
			fpDouble wkr, wki, xr, xi, yr, yi;

			m = n >> 1;
			ks = 2 * nc / m;
			kk = 0;
			for (j = 2; j < m; j += 2) {
				k = n - j;
				kk += ks;
				wkr = 0.5 - c[nc - kk];
				wki = c[kk];
				xr = a[j] - a[k];
				xi = a[j + 1] + a[k + 1];
				yr = wkr * xr - wki * xi;
				yi = wkr * xi + wki * xr;
				a[j] -= yr;
				a[j + 1] -= yi;
				a[k] += yr;
				a[k + 1] -= yi;
			}
		}


		inline void rftbsub(int32 n, fpDouble* a, int32 nc, fpDouble* c) {
			int32 j, k, kk, ks, m;
			fpDouble wkr, wki, xr, xi, yr, yi;

			a[1] = -a[1];
			m = n >> 1;
			ks = 2 * nc / m;
			kk = 0;
			for (j = 2; j < m; j += 2) {
				k = n - j;
				kk += ks;
				wkr = 0.5 - c[nc - kk];
				wki = c[kk];
				xr = a[j] - a[k];
				xi = a[j + 1] + a[k + 1];
				yr = wkr * xr + wki * xi;
				yi = wkr * xi - wki * xr;
				a[j] -= yr;
				a[j + 1] = yi - a[j + 1];
				a[k] += yr;
				a[k + 1] = yi - a[k + 1];
			}
			a[m + 1] = -a[m + 1];
		}
	}
}

#endif /* AK_MATH_FOURIERTRANSFORM_HPP_ */
