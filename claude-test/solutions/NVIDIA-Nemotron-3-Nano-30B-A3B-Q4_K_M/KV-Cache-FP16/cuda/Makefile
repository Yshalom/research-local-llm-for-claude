# Simple Makefile for building the CUDA FFT solution
CUDA_PATH ?= /usr/local/cuda
NVCC       = $(CUDA_PATH)/bin/nvcc
CXXFLAGS   = -std=c++17 -O3
LDFLAGS    = -lcudart -lcuda

# Source files
SRCS = gpu.cu tests.cu
OBJS = $(SRCS:.cu=.o)

# Target executable
TARGET = fft_test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(NVCC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cu
	$(NVCC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean