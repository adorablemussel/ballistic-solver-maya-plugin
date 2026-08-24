#include "kernel.cuh"
#include <cuda_runtime.h>

// To jest KERNEL - ta funkcja wywoła się na karcie graficznej!
__global__ void moveVerticesUp(double* d_coords, int total_elements) {
    // Każdy wątek na karcie pyta: "Jaki jest mój numer?"
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Zabezpieczenie przed wyjściem poza tablicę
    if (idx < total_elements) {
        // Tablica coords z Gmsha ma strukturę płaską: [x1, y1, z1, x2, y2, z2...]
        // Interesuje nas oś Y, czyli indeksy: 1, 4, 7, 10...
        if (idx % 3 == 1) {
            d_coords[idx] += 5.0; // Przesuwamy punkt w górę!
        }
    }
}

// To jest funkcja-menedżer działająca na procesorze
void runCudaTest(std::vector<double>& coords) {
    int total_elements = coords.size();
    size_t size_in_bytes = total_elements * sizeof(double);

    // 1. Zarezerwuj miejsce w pamięci VRAM karty graficznej
    double* d_coords;
    cudaMalloc(&d_coords, size_in_bytes);

    // 2. Skopiuj dane z pamięci RAM (CPU) do VRAM (GPU)
    cudaMemcpy(d_coords, coords.data(), size_in_bytes, cudaMemcpyHostToDevice);

    // 3. Konfiguracja "armii" wątków
    int threadsPerBlock = 256;
    int blocksPerGrid = (total_elements + threadsPerBlock - 1) / threadsPerBlock;

    // 4. UWOLNIJ KRAKENA (odpal Kernel na karcie!)
    moveVerticesUp<<<blocksPerGrid, threadsPerBlock>>>(d_coords, total_elements);

    // Czekamy, aż karta skończy pracę
    cudaDeviceSynchronize();

    // 5. Pobierz zmodyfikowane wyniki z powrotem do procesora (do std::vectora)
    cudaMemcpy(coords.data(), d_coords, size_in_bytes, cudaMemcpyDeviceToHost);

    // 6. Posprzątaj po sobie na karcie
    cudaFree(d_coords);
}