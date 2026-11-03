#pragma once

#include <vector>
#include <array>
#include <string>
#include "bMaterial.h"
#include "bBodyState.h"


class bSolverEngine {
public:
    bSolverEngine() = default;
    ~bSolverEngine() = default;

    void clearBodies();
    void addBody(const bMaterial& mat,
        const std::vector<float>& inVertices,
        const std::vector<int>& inTets,
        const std::array<float, 3>& initVel);

    void extractSurfaceTriangles(size_t bodyIndex, std::vector<int>& outTriangleIndices) const;

    void pinVerticesInBox(size_t bodyIndex, float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

    void step(float frameDt, int substeps, float damping = 0.999f);

    const std::vector<bBodyState>& getBodies() const { return bodies; }
    size_t getErodedCount(size_t bodyIndex) const;
    float getMaxTemperature(size_t bodyIndex) const;
    float getMaxPlasticStrain(size_t bodyIndex) const;

private:
    std::vector<bBodyState> bodies;

    void precomputeRestState(bBodyState& body);
    float computeSignedTetVolume(const float* p0, const float* p1, const float* p2, const float* p3) const;

    void clearAndApplyExternalForces(bBodyState& body);
    void computeInternalForces(bBodyState& body, float dt);
    void handleBodyCollisions();
    void evaluateErosion(bBodyState& body);
    void integrate(bBodyState& body, float dt, float damping);
    void handleGroundCollision(bBodyState& body, float groundY = 0.0f);
};