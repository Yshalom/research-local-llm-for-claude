#include "gpu.cuh"
#include <cuda_runtime.h>
#include <cmath>
#include <cstdio>
#include <vector>

// ---------------------------------------------------------------------------
// Complex number wrappers for clarity
// ---------------------------------------------------------------------------
inline __device__ cuFloatComplex make_complex(float real, float imag) {
    cuFloatComplex c;
    c.x = real;
    c.y = imag;
    return c;
}

inline __device__ float complex_real(cuFloatComplex c) { return c.x; }
inline __device__ float complex_imag(cuFloatComplex c) { return c.y; }

inline __device__ cuFloatComplex complex_add(cuFloatComplex a, cuFloatComplex b) {
    return make_complex(a.x + b.x, a.y + b.y);
}

inline __device__ cuFloatComplex complex_sub(cuFloatComplex a, cuFloatComplex b) {
    return make_complex(a.x - b.x, a.y - b.y);
}

inline __device__ cuFloatComplex complex_mul(cuFloatComplex a, cuFloatComplex b) {
    // (a.x + i a.y) * (b.x + i b.y) = (a.x*b.x - a.y*b.y) + i (a.x*b.y + a.y*b.x)
    float real = a.x * b.x - a.y * b.y;
    float imag = a.x * b.y + a.y * b.x;
    return make_complex(real, imag);
}

// ---------------------------------------------------------------------------
// Bit‑reversal permutation (in‑place)
// ---------------------------------------------------------------------------
static __device__ unsigned int bit_reverse(unsigned int x, unsigned int log2n) {
    unsigned int r = 0;
    for (unsigned int i = 0; i < log2n; ++i) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }
    return r;
}

// ---------------------------------------------------------------------------
// FFT kernel (iterative Cooley‑Tukey, in‑place)
// ---------------------------------------------------------------------------
static __global__ void fft_forward(const cuFloatComplex* __restrict__ input,
                                 cuFloatComplex* __restrict__ output,
                                 unsigned int n,
                                 unsigned int log2n,
                                 const cuFloatComplex* __restrict__ twiddle,
                                 bool invert) {
    // Each thread handles one output element
    unsigned int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= n) return;

    // Bit‑reverse the index
    unsigned int rev = bit_reverse(idx, log2n);
    cuFloatComplex x = input[rev];

    // Butterfly stages
    for (unsigned int s = 0; s < log2n; ++s) {
        unsigned int stride = 1u << s;
        unsigned int t = idx & stride;
        unsigned int m = idx ^ stride;

        // Determine twiddle factor index
        unsigned int twiddle_idx = invert ? (n - (idx % (1u << (s + 1)))) / (1u << (s + 1))
                                        : (idx % (1u << (s + 1))) / stride;

        cuFloatComplex w = twiddle[twiddle_idx];
        cuFloatComplex y = __ldg(&w); // cached load

        if (t) {
            // subtract
            cuFloatComplex u = x;
            x = complex_sub(u, y);
            y = u;
        } else {
            // add
            cuFloatComplex u = x;
            x = complex_add(u, y);
            y = u;
        }

        // Prepare for next stage
        x = y;
    }

    output[idx] = x;
}

// ---------------------------------------------------------------------------
// Launch helpers
// ---------------------------------------------------------------------------
static void launch_fft(const cuFloatComplex* h_input,
                       cuFloatComplex* h_output,
                       unsigned int size,
                       bool invert) {
    // Determine log2(size)
    unsigned int n = size;
    unsigned int log2n = 0;
    while ((1u << log2n) < n) ++log2n;

    // Allocate device memory
    cuFloatComplex *d_input, *d_output;
    size_t bytes = n * sizeof(cuFloatComplex);
    cudaMalloc(&d_input, bytes);
    cudaMalloc(&d_output, bytes);
    cudaMemcpy(d_input, h_input, bytes, cudaMemcpyHostToDevice);

    // Allocate twiddle factors
    std::vector<cuFloatComplex> twiddle_powers(n);
    for (unsigned int k = 0; k < n; ++k) {
        float angle = 2.0f * M_PI * static_cast<float>(k) / static_cast<float>(n);
        if (invert) angle = -angle;
        twiddle_powers[k] = make_complex(cosf(angle), sinf(angle));
    }

    cuFloatComplex *d_twiddle;
    cudaMalloc(&d_twiddle, n * sizeof(cuFloatComplex));
    cudaMemcpy(d_twiddle, twiddle_powers.data(),
               n * sizeof(cuFloatComplex), cudaMemcpyHostToDevice);

    // Kernel launch parameters
    const unsigned int blockSize = 256;
    unsigned int gridSize = (n + blockSize - 1) / blockSize;

    // Launch the appropriate kernel
    if (invert) {
        fft_forward<<<gridSize, blockSize>>>(d_input, d_output, n, log2n,
                                             d_twiddle, true);
    } else {
        fft_forward<<<gridSize, blockSize>>>(d_input, d_output, n, log2n,
                                             d_twiddle, false);
    }

    // Copy back
    cudaMemcpy(h_output, d_output, bytes, cudaMemcpyDeviceToHost);

    // Cleanup
    cudaFree(d_input);
    cudaFree(d_output);
    cudaFree(d_twiddle);
}

// ---------------------------------------------------------------------------
// Host API functions
// ---------------------------------------------------------------------------
extern "C" void fft(const cuFloatComplex* input, cuFloatComplex* output,
                    unsigned int size) {
    launch_fft(input, output, size, false);
}

extern "C" void ifft(const cuFloatComplex* input, cuFloatComplex* output,
                     unsigned int size) {
    launch_fft(input, output, size, true);
}

// ---------------------------------------------------------------------------
// Test utilities (read/write)
// ---------------------------------------------------------------------------
static void read_test_data(const char* filename,
                           std::vector<cuFloatComplex>& expected,
                           unsigned int size) {
    // Expected file format: each line contains two floats (real imag)
    // We'll read exactly `size` pairs.
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open test data file %s\n", filename);
        return;
    }
    for (unsigned int i = 0; i < size; ++i) {
        float real, imag;
        if (fscanf(fp, "%f %f", &real, &imag) != 2) {
            fprintf(stderr, "Unexpected format in %s at line %u\n", filename, i);
            break;
        }
        expected.emplace_back(make_complex(real, imag));
    }
    fclose(fp);
}

static void write_result(const char* filename,
                         const cuFloatComplex* data,
                         unsigned int size) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Cannot write result file %s\n", filename);
        return;
    }
    for (unsigned int i = 0; i < size; ++i) {
        fprintf(fp, "%f %f\n", data[i].x, data[i].y);
    }
    fclose(fp);
}

// ---------------------------------------------------------------------------
// Maximum error computation
// ---------------------------------------------------------------------------
static float max_error(const cuFloatComplex* got,
                       const cuFloatComplex* expected,
                       unsigned int size) {
    float max_err = 0.0f;
    for (unsigned int i = 0; i < size; ++i) {
        float err = fabsf(got[i].x - expected[i].x) +
                    fabsf(got[i].y - expected[i].y);
        if (err > max_err) max_err = err;
    }
    return max_err;
}

// ---------------------------------------------------------------------------
// Speed test helpers (Windows high‑resolution timer)
// ---------------------------------------------------------------------------
#ifdef _WIN32
#include <windows.h>
static uint64_t wintime_ns() {
    LARGE_INTEGER freq, cnt;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cnt);
    return (uint64_t)cnt.QuadPart * 1e9 / freq.QuadPart;
}
#else
static uint64_t wintime_ns() { return 0; }
#endif

static void speed_test(const char* label, unsigned int n) {
    // Allocate host arrays
    std::vector<cuFloatComplex> h_input(n);
    std::vector<cuFloatComplex> h_output(n);

    // Fill with random data in range (-1, 1)
    std::mt19937_64 rng(12345);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (unsigned int i = 0; i < n; ++i) {
        float r = dist(rng);
        h_input[i] = make_complex(r, r);
    }

    // Time FFT
    uint64_t t0 = wintime_ns();
    fft(h_input.data(), h_output.data(), n);
    uint64_t t1 = wintime_ns();

    // Time IFFT (on the same data, need fresh input)
    std::copy(h_input.begin(), h_input.end(), h_output.begin());
    t0 = wintime_ns();
    ifft(h_output.data(), h_output.data(), n);
    t1 = wintime_ns();

    printf("%s with %u elements: %lluns\n", label, n, (unsigned long long)(t1 - t0));
}