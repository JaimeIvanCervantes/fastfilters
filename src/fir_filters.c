// fastfilters
// Copyright (c) 2016 Sven Peter
// sven.peter@iwr.uni-heidelberg.de or mail@svenpeter.me
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "fastfilters.h"
#include "common.h"

fastfilters_kernel_fir_t DLL_PUBLIC fastfilters_kernel_fir_gaussian(unsigned int order, double sigma);
unsigned int DLL_PUBLIC fastfilters_kernel_fir_get_length(fastfilters_kernel_fir_t kernel);
void DLL_PUBLIC fastfilters_kernel_fir_free(fastfilters_kernel_fir_t kernel);

bool DLL_PUBLIC fastfilters_fir_gaussian2d(const fastfilters_array2d_t *inarray, unsigned order, double sigma,
                                           fastfilters_array2d_t *outarray)
{
    bool result = false;
    fastfilters_kernel_fir_t kx = NULL;
    fastfilters_kernel_fir_t ky = NULL;

    kx = fastfilters_kernel_fir_gaussian(order, sigma);
    if (!kx)
        goto out;

    ky = fastfilters_kernel_fir_gaussian(order, sigma);
    if (!ky)
        goto out;

    result = fastfilters_fir_convolve2d(inarray, kx, ky, outarray, 0, 0, 0, 0);

out:
    if (kx)
        fastfilters_kernel_fir_free(kx);
    if (ky)
        fastfilters_kernel_fir_free(ky);
    return result;
}

bool DLL_PUBLIC fastfilters_fir_hog2d(const fastfilters_array2d_t *inarray, double sigma, fastfilters_array2d_t *out_xx,
                                      fastfilters_array2d_t *out_xy, fastfilters_array2d_t *out_yy)
{
    bool result = false;
    fastfilters_kernel_fir_t k_smooth = NULL;
    fastfilters_kernel_fir_t k_first = NULL;
    fastfilters_kernel_fir_t k_second = NULL;

    k_smooth = fastfilters_kernel_fir_gaussian(0, sigma);
    if (!k_smooth)
        goto out;

    k_first = fastfilters_kernel_fir_gaussian(1, sigma);
    if (!k_first)
        goto out;

    k_second = fastfilters_kernel_fir_gaussian(2, sigma);
    if (!k_second)
        goto out;

    result = fastfilters_fir_convolve2d(inarray, k_second, k_smooth, out_xx, 0, 0, 0, 0);
    if (!result)
        goto out;

    result = fastfilters_fir_convolve2d(inarray, k_smooth, k_second, out_yy, 0, 0, 0, 0);
    if (!result)
        goto out;

    result = fastfilters_fir_convolve2d(inarray, k_first, k_first, out_xy, 0, 0, 0, 0);
    if (!result)
        goto out;

out:
    if (k_smooth)
        fastfilters_kernel_fir_free(k_smooth);
    if (k_first)
        fastfilters_kernel_fir_free(k_first);
    if (k_second)
        fastfilters_kernel_fir_free(k_second);
    return result;
}

DLL_PUBLIC fastfilters_array2d_t *fastfilters_array2d_alloc(size_t n_x, size_t n_y, size_t channels)
{
    return NULL;
}

DLL_PUBLIC void fastfilters_array2d_free(fastfilters_array2d_t *v)
{
    (void)v;
}
static bool fastfilters_fir_deriv2d(const fastfilters_array2d_t *inarray, double sigma, unsigned order,
                                    fastfilters_array2d_t *outarray, bool sqrt)
{
    bool result = false;
    fastfilters_kernel_fir_t k_smooth = NULL;
    fastfilters_kernel_fir_t k_deriv = NULL;
    fastfilters_array2d_t *tmparray = NULL;

    k_smooth = fastfilters_kernel_fir_gaussian(0, sigma);
    if (!k_smooth)
        goto out;

    k_deriv = fastfilters_kernel_fir_gaussian(order, sigma);
    if (!k_deriv)
        goto out;

    tmparray = fastfilters_array2d_alloc(inarray->n_x, inarray->n_y, inarray->n_channels);
    if (!tmparray)
        goto out;

    result = fastfilters_fir_convolve2d(inarray, k_deriv, k_smooth, outarray, 0, 0, 0, 0);
    if (!result)
        goto out;

    result = fastfilters_fir_convolve2d(inarray, k_smooth, k_deriv, tmparray, 0, 0, 0, 0);
    if (!result)
        goto out;

    if (sqrt)
        fastfilters_combine_addsqrt2d(outarray, tmparray, outarray);
    else
        fastfilters_combine_add2d(outarray, tmparray, outarray);

out:
    if (k_smooth)
        fastfilters_kernel_fir_free(k_smooth);
    if (k_deriv)
        fastfilters_kernel_fir_free(k_deriv);
    if (tmparray)
        fastfilters_array2d_free(tmparray);
    return result;
}

bool DLL_PUBLIC fastfilters_fir_gradmag2d(const fastfilters_array2d_t *inarray, double sigma,
                                          fastfilters_array2d_t *outarray)
{
    return fastfilters_fir_deriv2d(inarray, sigma, 1, outarray, true);
}

bool DLL_PUBLIC fastfilters_fir_laplacian2d(const fastfilters_array2d_t *inarray, double sigma,
                                            fastfilters_array2d_t *outarray)
{
    return fastfilters_fir_deriv2d(inarray, sigma, 2, outarray, false);
}