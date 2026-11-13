#pragma once

#include <vector>
#include <array>
#include <string>
#include "bMaterial.h"
#include "bBodyState.h"

enum class bComputeBackend : short {
    CPU_Single = 0,
    CPU_OpenMP = 1,
    GPU_CUDA = 2
};

class bSolverEngine {
public:
    bSolverEngine() = default;
    ~bSolverEngine() = default;

    void clearBodies();
    void addBody(const bMaterial& mat,
        const std::vector<float>& inVertices,
        const std::vector<int>& inTets,
        const std::array<float, 3>& initVel);

    void pinVertices(size_t bodyIndex, const std::vector<int>& vertexIndices);
    
    void step(float frameDt, int substeps, bComputeBackend backend = bComputeBackend::CPU_Single, float damping = 0.999f);

    void extractSurfaceTriangles(size_t bodyIndex, std::vector<int>& outTriangleIndices) const;
    
    const std::vector<bBodyState>& getBodies() const { return bodies; }

private:
    std::vector<bBodyState> bodies;

    void precomputeRestState(bBodyState& body);
    float computeSignedTetVolume(const float* p0, const float* p1, const float* p2, const float* p3) const;

    void clearAndApplyExternalForces(bBodyState& body);
    void computeInternalForces(bBodyState& body, float dt);
    void handleBodyCollisions(float dt);
    void evaluateErosion(bBodyState& body);
    void integrate(bBodyState& body, float dt, float damping);
    void handleGroundCollision(bBodyState& body, float groundY = 0.0f);
};