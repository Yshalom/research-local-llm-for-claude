#ifndef GPU_CUH
#define GPU_CUH

#include <cuComplex.h>

// Forward declarations
__global__ void fft_kernel(const cuFloatComplex* input, cuFloatComplex* output, unsigned int size, bool invert);
__global__ void ifft_kernel(const cuFloatComplex* input, cuFloatComplex* output, unsigned int size);

#endif // GPU_CUH