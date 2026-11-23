#pragma once

#include <vector>
#include <cstdint>
#include "bBodyState.h"

struct bCudaBodyBuffer {
    size_t numVertices = 0;
    size_t numTets = 0;

    // bufory wierzcho³kowe w VRAM
    float* d_positions = nullptr;
    float* d_velocities = nullptr;
    float* d_forces = nullptr;
    float* d_masses = nullptr;
    float* d_contactRadii = nullptr;
    uint8_t* d_isVertexActive = nullptr;

    // bufory czworoœcianów w VRAM
    int* d_tetrahedrons = nullptr;
    float* d_restVolumes = nullptr;
    float* d_restEdgeLengths = nullptr;
    uint8_t* d_isEroded = nullptr;
    float* d_temperatures = nullptr;
    float* d_plasticStrains = nullptr;
    float* d_internalEnergies = nullptr;
    float* d_damageAccum = nullptr;

    // flaga sygnalizuj¹ca now¹ erozjê w podkroku
    int* d_anyNewErosion = nullptr;
};

void bFreeCudaBuffers(std::vector<bCudaBodyBuffer>& gpuBodies);

void bStepCUDA(
    std::vector<bBodyState>& bodies,
    std::vector<bCudaBodyBuffer>& gpuBodies,
    float frameDt,
    int substeps,
    float damping);