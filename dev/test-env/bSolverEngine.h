#pragma once

#include <vector>
#include <array>
#include <string>
#include "../../include/bMaterial.h"

struct SimulatedBody {
    bMaterial material;
    
    std::vector<float> positions;
    std::vector<float> velocities;
    std::vector<float> forces;
    std::vector<float> masses;
    std::vector<float> contactRadii;
    std::vector<bool> isVertexActive;

    std::vector<int> tetrahedrons;
    std::vector<float> restVolumes;
    std::vector<bool> isEroded;
    std::vector<float> restEdgeLengths;

    std::vector<float> temperatures;
    std::vector<float> plasticStrains;
    std::vector<float> internalEnergies;
    std::vector<float> damageAccum;
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

    void pinVerticesInBox(size_t bodyIndex, float minX, float maxX, float minY, float maxY, float minZ, float maxZ);

    void step(float frameDt, int substeps, float damping = 0.999f);

    const std::vector<SimulatedBody>& getBodies() const { return bodies; }
    size_t getErodedCount(size_t bodyIndex) const;
    float getMaxTemperature(size_t bodyIndex) const;
    float getMaxPlasticStrain(size_t bodyIndex) const;

private:
    std::vector<SimulatedBody> bodies;

    void precomputeRestState(SimulatedBody& body);
    float computeSignedTetVolume(const float* p0, const float* p1, const float* p2, const float* p3) const;

    void clearAndApplyExternalForces(SimulatedBody& body);
    void computeInternalForces(SimulatedBody& body, float dt);
    void handleBodyCollisions();
    void evaluateErosion(SimulatedBody& body);
    void integrate(SimulatedBody& body, float dt, float damping);
    void handleGroundCollision(SimulatedBody& body, float groundY = 0.0f);
};