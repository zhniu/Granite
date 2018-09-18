/* Copyright (c) 2017-2018 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cmath>

namespace Granite
{
namespace Audio
{
namespace DSP
{
static inline void accumulate_channel(float * __restrict output, const float * __restrict input, float gain, size_t count)
{
	for (size_t i = 0; i < count; i++)
		output[i] += input[i] * gain;
}

static int16_t f32_to_i16(float v)
{
	int32_t i = int32_t(std::round(v * 0x8000));
	if (i > 0x7fff)
		return 0x7fff;
	else if (i < -0x8000)
		return -0x8000;
	else
		return int16_t(i);
}

static inline void interleave_stereo_f32_i16(int16_t * __restrict target,
                                             const float * __restrict left,
                                             const float * __restrict right,
                                             size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		*target++ = f32_to_i16(*left++);
		*target++ = f32_to_i16(*right++);
	}
}
}
}
}