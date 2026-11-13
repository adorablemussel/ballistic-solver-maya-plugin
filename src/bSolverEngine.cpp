#include "bSolverEngine.h"
#include <cmath>
#include <map>
#include <algorithm>
#include <omp.h>

void bSolverEngine::clearBodies() {
    bodies.clear();
}

void bSolverEngine::addBody(const bMaterial& mat,
    const std::vector<float>& inVertices,
    const std::vector<int>& inTets,
    const std::array<float, 3>& initVel)
{
    bBodyState body;
    body.material = mat;
    body.positions = inVertices;
    body.tetrahedrons = inTets;

    size_t numVertices = inVertices.size() / 3;
    size_t numTets = inTets.size() / 4;

    body.velocities.resize(numVertices * 3);
    body.forces.resize(numVertices * 3, 0.0f);
    body.masses.resize(numVertices, 0.0f);
    body.contactRadii.resize(numVertices, 0.0f);
    body.isVertexActive.resize(numVertices, 1);

    for (size_t i = 0; i < numVertices; ++i) {
        body.velocities[i * 3 + 0] = initVel[0];
        body.velocities[i * 3 + 1] = initVel[1];
        body.velocities[i * 3 + 2] = initVel[2];
    }

    body.restVolumes.resize(numTets, 0.0f);
    body.isEroded.resize(numTets, 0);

    float initTemp = (mat.T_room > 0.0f) ? mat.T_room : 293.15f;
    body.temperatures.resize(numTets, initTemp);
    body.plasticStrains.resize(numTets, 0.0f);
    body.internalEnergies.resize(numTets, 0.0f);
    body.damageAccum.resize(numTets, 0.0f);

    precomputeRestState(body);
    bodies.push_back(std::move(body));
}

void bSolverEngine::extractSurfaceTriangles(size_t bodyIndex, std::vector<int>& outTriangleIndices) const {
    outTriangleIndices.clear();
    if (bodyIndex >= bodies.size()) return;

    const auto& body = bodies[bodyIndex];
    size_t numTets = body.tetrahedrons.size() / 4;

    struct OrientedFace {
        int v0, v1, v2;
        int count;
    };

    std::map<std::array<int, 3>, OrientedFace> faceMap;

    const int tetFaces[4][3] = {
        {0, 2, 1},
        {0, 1, 3},
        {0, 3, 2},
        {1, 2, 3}
    };

    for (size_t t = 0; t < numTets; ++t) {
        if (body.isEroded[t]) continue;

        for (int f = 0; f < 4; ++f) {
            int a = body.tetrahedrons[t * 4 + tetFaces[f][0]];
            int b = body.tetrahedrons[t * 4 + tetFaces[f][1]];
            int c = body.tetrahedrons[t * 4 + tetFaces[f][2]];

            std::array<int, 3> key = { a, b, c };
            std::sort(key.begin(), key.end());

            auto it = faceMap.find(key);
            if (it == faceMap.end()) {
                faceMap[key] = { a, b, c, 1 };
            }
            else {
                it->second.count++;
            }
        }
    }

    outTriangleIndices.reserve(faceMap.size() * 3);
    for (const auto& pair : faceMap) {
        if (pair.second.count == 1) {
            outTriangleIndices.push_back(pair.second.v0);
            outTriangleIndices.push_back(pair.second.v1);
            outTriangleIndices.push_back(pair.second.v2);
        }
    }
}

void bSolverEngine::pinVertices(size_t bodyIndex, const std::vector<int>& vertexIndices) {
    if (bodyIndex >= bodies.size()) return;
    bBodyState& body = bodies[bodyIndex];
    body.pinnedVertices = vertexIndices;
    size_t numVertices = body.masses.size();

    for (int idx : vertexIndices) {
        if (idx >= 0 && static_cast<size_t>(idx) < numVertices) {
            body.masses[idx] = 0.0f;
            body.velocities[idx * 3 + 0] = 0.0f;
            body.velocities[idx * 3 + 1] = 0.0f;
            body.velocities[idx * 3 + 2] = 0.0f;
        }
    }
}

float bSolverEngine::computeSignedTetVolume(const float* p0, const float* p1, const float* p2, const float* p3) const {
    float e1[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
    float e2[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };
    float e3[3] = { p3[0] - p0[0], p3[1] - p0[1], p3[2] - p0[2] };

    float cross[3] = {
        e2[1] * e3[2] - e2[2] * e3[1],
        e2[2] * e3[0] - e2[0] * e3[2],
        e2[0] * e3[1] - e2[1] * e3[0]
    };

    float det = e1[0] * cross[0] + e1[1] * cross[1] + e1[2] * cross[2];
    return det / 6.0f;
}

void bSolverEngine::precomputeRestState(bBodyState& body) {
    size_t numTets = body.tetrahedrons.size() / 4;
    size_t numVertices = body.positions.size() / 3;
    float density = (body.material.density > 0.0f) ? body.material.density : 1000.0f;

    body.restEdgeLengths.clear();
    body.restEdgeLengths.reserve(numTets * 6);

    std::vector<float> nodalVolumes(numVertices, 0.0f);
    int edges[6][2] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };

    for (size_t t = 0; t < numTets; ++t) {
        int& i0 = body.tetrahedrons[t * 4 + 0];
        int& i1 = body.tetrahedrons[t * 4 + 1];
        int& i2 = body.tetrahedrons[t * 4 + 2];
        int& i3 = body.tetrahedrons[t * 4 + 3];

        float signedVol = computeSignedTetVolume(&body.positions[i0 * 3],
            &body.positions[i1 * 3],
            &body.positions[i2 * 3],
            &body.positions[i3 * 3]);

        if (signedVol < 0.0f) {
            std::swap(i0, i1);
            signedVol = -signedVol;
        }

        body.restVolumes[t] = signedVol;

        float quarterVol = signedVol * 0.25f;
        nodalVolumes[i0] += quarterVol;
        nodalVolumes[i1] += quarterVol;
        nodalVolumes[i2] += quarterVol;
        nodalVolumes[i3] += quarterVol;

        float tetMass = signedVol * density;
        body.masses[i0] += tetMass * 0.25f;
        body.masses[i1] += tetMass * 0.25f;
        body.masses[i2] += tetMass * 0.25f;
        body.masses[i3] += tetMass * 0.25f;

        for (int e = 0; e < 6; ++e) {
            int idxA = body.tetrahedrons[t * 4 + edges[e][0]];
            int idxB = body.tetrahedrons[t * 4 + edges[e][1]];
            const float* pA = &body.positions[idxA * 3];
            const float* pB = &body.positions[idxB * 3];

            float dx = pB[0] - pA[0];
            float dy = pB[1] - pA[1];
            float dz = pB[2] - pA[2];
            body.restEdgeLengths.push_back(std::sqrt(dx * dx + dy * dy + dz * dz));
        }
    }

    for (size_t i = 0; i < numVertices; ++i) {
        body.contactRadii[i] = 0.45f * std::cbrt(nodalVolumes[i]);
    }
}

void bSolverEngine::step(float frameDt, int substeps, bComputeBackend backend, float damping) {
    if (substeps <= 0) substeps = 1;
    float dt = frameDt / static_cast<float>(substeps);

    // W kolejnym kroku podepniemy tutaj wywo³anie stepCUDA() dla bComputeBackend::GPU_CUDA
    bool useOpenMP = (backend == bComputeBackend::CPU_OpenMP);

    for (int s = 0; s < substeps; ++s) {
        for (auto& body : bodies) {
            clearAndApplyExternalForces(body, useOpenMP);
            computeInternalForces(body, dt, useOpenMP);
        }

        handleBodyCollisions(dt, useOpenMP);

        for (auto& body : bodies) {
            evaluateErosion(body, useOpenMP);
            integrate(body, dt, damping, useOpenMP);
            handleGroundCollision(body, 0.0f, useOpenMP);
        }
    }
}

void bSolverEngine::clearAndApplyExternalForces(bBodyState& body, bool useOpenMP) {
    long long numVertices = static_cast<long long>(body.positions.size() / 3);
    const float gravityY = -9.81f;

    #pragma omp parallel for if(useOpenMP)
    for (long long i = 0; i < numVertices; ++i) {
        body.forces[i * 3 + 0] = 0.0f;
        body.forces[i * 3 + 1] = body.masses[i] * gravityY;
        body.forces[i * 3 + 2] = 0.0f;
    }
}

void bSolverEngine::computeInternalForces(bBodyState& body, float dt, bool useOpenMP) {
    long long numTets = static_cast<long long>(body.tetrahedrons.size() / 4);
    const bMaterial& mat = body.material;

    float E_base = (mat.youngModulus > 0.0f) ? mat.youngModulus : 500000.0f;
    float nu = std::clamp(mat.poissonRatio, 0.0f, 0.45f);
    float rho0 = (mat.density > 0.0f) ? mat.density : 1000.0f;
    float K_linear = E_base / (3.0f * (1.0f - 2.0f * nu));

    float T_room = (mat.T_room > 0.0f) ? mat.T_room : 293.15f;
    float T_melt = (mat.T_melt > T_room) ? mat.T_melt : 1800.0f;
    float Cp = (mat.specificHeat > 0.0f) ? mat.specificHeat : 470.0f;
    float beta_TQ = (mat.taylorQuinney > 0.0f) ? mat.taylorQuinney : 0.9f;
    float eps_dot_0 = (mat.eps_dot_0 > 0.0f) ? mat.eps_dot_0 : 1.0f;

    const int edges[6][2] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };

    #pragma omp parallel for if(useOpenMP)
    for (long long t = 0; t < numTets; ++t) {
        if (body.isEroded[t]) continue;

        float V0 = body.restVolumes[t];
        if (V0 <= 1e-12f) continue;

        float tetNodeMass = 0.25f * rho0 * V0;
        float maxStableK = (dt > 1e-7f) ? (0.12f * tetNodeMass / (dt * dt)) : 1e30f;

        float T = body.temperatures[t];
        float T_star = std::clamp((T - T_room) / (T_melt - T_room), 0.0f, 0.999f);
        float m_exp = (mat.m > 0.0f) ? mat.m : 1.0f;
        float thermalFactor = 1.0f - std::pow(T_star, m_exp);
        float E_eff = E_base * thermalFactor;

        int tetVerts[4] = {
            body.tetrahedrons[t * 4 + 0],
            body.tetrahedrons[t * 4 + 1],
            body.tetrahedrons[t * 4 + 2],
            body.tetrahedrons[t * 4 + 3]
        };

        const float* p0 = &body.positions[tetVerts[0] * 3];
        const float* p1 = &body.positions[tetVerts[1] * 3];
        const float* p2 = &body.positions[tetVerts[2] * 3];
        const float* p3 = &body.positions[tetVerts[3] * 3];

        float e1[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
        float e2[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };
        float e3[3] = { p3[0] - p0[0], p3[1] - p0[1], p3[2] - p0[2] };

        float gradV1[3] = { (e2[1] * e3[2] - e2[2] * e3[1]) / 6.0f, (e2[2] * e3[0] - e2[0] * e3[2]) / 6.0f, (e2[0] * e3[1] - e2[1] * e3[0]) / 6.0f };
        float gradV2[3] = { (e3[1] * e1[2] - e3[2] * e1[1]) / 6.0f, (e3[2] * e1[0] - e3[0] * e1[2]) / 6.0f, (e3[0] * e1[1] - e3[1] * e1[0]) / 6.0f };
        float gradV3[3] = { (e1[1] * e2[2] - e1[2] * e2[1]) / 6.0f, (e1[2] * e2[0] - e1[0] * e2[2]) / 6.0f, (e1[0] * e2[1] - e1[1] * e2[0]) / 6.0f };
        float gradV0[3] = { -(gradV1[0] + gradV2[0] + gradV3[0]),
                            -(gradV1[1] + gradV2[1] + gradV3[1]),
                            -(gradV1[2] + gradV2[2] + gradV3[2]) };

        float maxGradSq = 1e-16f;
        const float* grads[4] = { gradV0, gradV1, gradV2, gradV3 };
        for (int g = 0; g < 4; ++g) {
            float gSq = grads[g][0] * grads[g][0] + grads[g][1] * grads[g][1] + grads[g][2] * grads[g][2];
            maxGradSq = std::max(maxGradSq, gSq);
        }

        float maxStableBulkModulus = maxStableK * V0 / maxGradSq;

        float currentV = e1[0] * gradV1[0] + e1[1] * gradV1[1] + e1[2] * gradV1[2];
        float safeV = std::max(currentV, 0.35f * V0);
        float mu = std::clamp((V0 / safeV) - 1.0f, -0.5f, 1.5f);

        float pressure = 0.0f;
        if (mat.eos_C0 > 0.0f && mu > 0.0f) {
            float K_eos = std::min(rho0 * mat.eos_C0 * mat.eos_C0, maxStableBulkModulus);
            float s = mat.eos_S;
            float gamma0 = mat.eos_Gamma0;
            float denom = std::max(1.0f - (s - 1.0f) * mu, 0.35f);
            pressure = (K_eos * mu * (1.0f + (1.0f - 0.5f * gamma0) * mu)) / (denom * denom);
        }
        else {
            float K_eff = std::min(K_linear * thermalFactor, maxStableBulkModulus);
            float volStrain = std::clamp((currentV - V0) / V0, -0.5f, 0.5f);
            pressure = -K_eff * volStrain;
        }

        // Lokalna akumulacja si³ dla 4 wierzcho³ków czworoœcianu (ogranicza liczbê operacji atomic 4-krotnie)
        float localF[4][3] = {
            { pressure * gradV0[0], pressure * gradV0[1], pressure * gradV0[2] },
            { pressure * gradV1[0], pressure * gradV1[1], pressure * gradV1[2] },
            { pressure * gradV2[0], pressure * gradV2[1], pressure * gradV2[2] },
            { pressure * gradV3[0], pressure * gradV3[1], pressure * gradV3[2] }
        };

        float maxTetPlasticInc = 0.0f;
        float maxEdgeStrainRate = 0.0f;
        float maxTrialStress = 0.0f;

        for (int e = 0; e < 6; ++e) {
            int localA = edges[e][0];
            int localB = edges[e][1];
            int iA = tetVerts[localA];
            int iB = tetVerts[localB];

            const float* pA = &body.positions[iA * 3];
            const float* pB = &body.positions[iB * 3];
            const float* vA = &body.velocities[iA * 3];
            const float* vB = &body.velocities[iB * 3];

            float dx = pB[0] - pA[0];
            float dy = pB[1] - pA[1];
            float dz = pB[2] - pA[2];
            float currentLength = std::sqrt(dx * dx + dy * dy + dz * dz);

            if (currentLength < 1e-6f) continue;

            float nx = dx / currentLength;
            float ny = dy / currentLength;
            float nz = dz / currentLength;

            float& restLength = body.restEdgeLengths[t * 6 + e];
            float deltaL = currentLength - restLength;
            float elasticStrain = std::abs(deltaL) / restLength;

            float relVel = (vB[0] - vA[0]) * nx + (vB[1] - vA[1]) * ny + (vB[2] - vA[2]) * nz;
            float strainRate = std::abs(relVel) / restLength;
            maxEdgeStrainRate = std::max(maxEdgeStrainRate, strainRate);

            float trialStress = E_eff * elasticStrain;

            if (mat.A > 0.0f) {
                float eps_p = body.plasticStrains[t];
                float n_exp = (mat.n > 0.0f) ? mat.n : 1.0f;
                float hardTerm = mat.A + mat.B * std::pow(eps_p, n_exp);

                float rateRatio = std::max(strainRate / eps_dot_0, 1.0f);
                float rateTerm = 1.0f + mat.C * std::log(rateRatio);

                float yieldStress = hardTerm * rateTerm * thermalFactor;

                if (trialStress > yieldStress && E_eff > 1e-6f) {
                    float plasticStrainInc = (trialStress - yieldStress) / E_eff;
                    trialStress = yieldStress;

                    float sign = (deltaL >= 0.0f) ? 1.0f : -1.0f;
                    restLength += sign * plasticStrainInc * restLength;
                    restLength = std::max(restLength, 1e-5f);
                    deltaL = currentLength - restLength;

                    if (plasticStrainInc > maxTetPlasticInc) {
                        maxTetPlasticInc = plasticStrainInc;
                    }

                    float plasticWorkPerVol = yieldStress * (plasticStrainInc / 6.0f);
                    body.internalEnergies[t] += plasticWorkPerVol / rho0;
                    body.temperatures[t] += (beta_TQ * plasticWorkPerVol) / (rho0 * Cp);
                }
            }

            maxTrialStress = std::max(maxTrialStress, trialStress);

            float k_edge = std::min(E_eff * V0 / (restLength * restLength), maxStableK);
            float c_edge = 0.1f * std::sqrt(k_edge * tetNodeMass);

            float springForce = k_edge * deltaL;
            float totalForce = springForce + relVel * c_edge;

            localF[localA][0] += totalForce * nx;
            localF[localA][1] += totalForce * ny;
            localF[localA][2] += totalForce * nz;

            localF[localB][0] -= totalForce * nx;
            localF[localB][1] -= totalForce * ny;
            localF[localB][2] -= totalForce * nz;
        }

        // Bezpieczny wielow¹tkowo zapis zagregowanych si³ do pamiêci globalnej
        for (int v = 0; v < 4; ++v) {
            int globalIdx = tetVerts[v] * 3;
            #pragma omp atomic
            body.forces[globalIdx + 0] += localF[v][0];
            #pragma omp atomic
            body.forces[globalIdx + 1] += localF[v][1];
            #pragma omp atomic
            body.forces[globalIdx + 2] += localF[v][2];
        }

        if (maxTetPlasticInc > 0.0f) {
            body.plasticStrains[t] += maxTetPlasticInc;

            if (mat.D1 > 0.0f) {
                float sigma_eq = std::max(maxTrialStress, 1.0f);
                float triaxiality = std::clamp(-pressure / sigma_eq, -2.0f, 2.0f);

                float d123 = mat.D1 + mat.D2 * std::exp(mat.D3 * triaxiality);
                float rateRatio = std::max(maxEdgeStrainRate / eps_dot_0, 1.0f);
                float d4_term = 1.0f + mat.D4 * std::log(rateRatio);
                float d5_term = 1.0f + mat.D5 * T_star;

                float fractureStrain = std::max(d123 * d4_term * d5_term, 1e-4f);
                body.damageAccum[t] += maxTetPlasticInc / fractureStrain;
            }
        }
    }
}

void bSolverEngine::handleBodyCollisions(float dt, bool useOpenMP) {
    if (bodies.size() < 2) return;

    for (size_t a = 0; a < bodies.size(); ++a) {
        for (size_t b = a + 1; b < bodies.size(); ++b) {
            bBodyState& bodyA = bodies[a];
            bBodyState& bodyB = bodies[b];

            long long numVA = static_cast<long long>(bodyA.positions.size() / 3);
            long long numVB = static_cast<long long>(bodyB.positions.size() / 3);

            float EA = (bodyA.material.youngModulus > 0.0f) ? bodyA.material.youngModulus : 500000.0f;
            float EB = (bodyB.material.youngModulus > 0.0f) ? bodyB.material.youngModulus : 500000.0f;
            float E_eff = 0.5f * (EA + EB);

            #pragma omp parallel for if(useOpenMP)
            for (long long i = 0; i < numVA; ++i) {
                if (!bodyA.isVertexActive[i]) continue;

                const float* pA = &bodyA.positions[i * 3];
                const float* vA = &bodyA.velocities[i * 3];
                float rA = bodyA.contactRadii[i];

                float sumForceAx = 0.0f;
                float sumForceAy = 0.0f;
                float sumForceAz = 0.0f;

                for (long long j = 0; j < numVB; ++j) {
                    if (!bodyB.isVertexActive[j]) continue;

                    const float* pB = &bodyB.positions[j * 3];
                    const float* vB = &bodyB.velocities[j * 3];
                    float rB = bodyB.contactRadii[j];

                    float minDist = 1.3f * (rA + rB);
                    float dx = pB[0] - pA[0];
                    if (std::abs(dx) >= minDist) continue;
                    float dy = pB[1] - pA[1];
                    if (std::abs(dy) >= minDist) continue;
                    float dz = pB[2] - pA[2];
                    if (std::abs(dz) >= minDist) continue;

                    float distSq = dx * dx + dy * dy + dz * dz;
                    if (distSq < minDist * minDist && distSq > 1e-12f) {
                        float dist = std::sqrt(distSq);
                        float overlap = minDist - dist;

                        float nx = dx / dist;
                        float ny = dy / dist;
                        float nz = dz / dist;

                        float mA = (bodyA.masses[i] > 1e-8f) ? bodyA.masses[i] : 1e30f;
                        float mB = (bodyB.masses[j] > 1e-8f) ? bodyB.masses[j] : 1e30f;
                        float minMass = std::min(mA, mB);

                        float k_contact = 0.5f * E_eff * minDist;
                        if (minMass < 1e20f && dt > 1e-7f) {
                            k_contact = std::min(k_contact, 0.05f * minMass / (dt * dt));
                        }

                        float forceMag = k_contact * overlap;

                        float relVel = (vB[0] - vA[0]) * nx + (vB[1] - vA[1]) * ny + (vB[2] - vA[2]) * nz;
                        if (relVel < 0.0f && minMass < 1e20f) {
                            forceMag -= 0.25f * relVel * std::sqrt(k_contact * minMass);
                        }

                        float fx = forceMag * nx;
                        float fy = forceMag * ny;
                        float fz = forceMag * nz;

                        sumForceAx -= fx;
                        sumForceAy -= fy;
                        sumForceAz -= fz;

                        #pragma omp atomic
                        bodyB.forces[j * 3 + 0] += fx;
                        #pragma omp atomic
                        bodyB.forces[j * 3 + 1] += fy;
                        #pragma omp atomic
                        bodyB.forces[j * 3 + 2] += fz;
                    }
                }

                bodyA.forces[i * 3 + 0] += sumForceAx;
                bodyA.forces[i * 3 + 1] += sumForceAy;
                bodyA.forces[i * 3 + 2] += sumForceAz;
            }
        }
    }
}

void bSolverEngine::evaluateErosion(bBodyState& body, bool useOpenMP) {
    if (body.material.D1 <= 0.0f) return;

    long long numTets = static_cast<long long>(body.tetrahedrons.size() / 4);
    const int edges[6][2] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };
    int anyNewErosion = 0;

#pragma omp parallel for if(useOpenMP) reduction(|:anyNewErosion)
    for (long long t = 0; t < numTets; ++t) {
        if (body.isEroded[t]) continue;

        if (body.damageAccum[t] >= 1.0f ||
            (body.material.T_melt > 0.0f && body.temperatures[t] >= body.material.T_melt)) {
            body.isEroded[t] = 1;
            anyNewErosion = 1;
            continue;
        }

        int i0 = body.tetrahedrons[t * 4 + 0];
        int i1 = body.tetrahedrons[t * 4 + 1];
        int i2 = body.tetrahedrons[t * 4 + 2];
        int i3 = body.tetrahedrons[t * 4 + 3];
        float currentV = computeSignedTetVolume(&body.positions[i0 * 3],
            &body.positions[i1 * 3],
            &body.positions[i2 * 3],
            &body.positions[i3 * 3]);
        if (currentV < 0.2f * body.restVolumes[t]) {
            body.isEroded[t] = 1;
            anyNewErosion = 1;
            continue;
        }

        for (int e = 0; e < 6; ++e) {
            int iA = body.tetrahedrons[t * 4 + edges[e][0]];
            int iB = body.tetrahedrons[t * 4 + edges[e][1]];

            const float* pA = &body.positions[iA * 3];
            const float* pB = &body.positions[iB * 3];

            float dx = pB[0] - pA[0];
            float dy = pB[1] - pA[1];
            float dz = pB[2] - pA[2];
            float currentLength = std::sqrt(dx * dx + dy * dy + dz * dz);
            float restLength = body.restEdgeLengths[t * 6 + e];

            float strain = std::abs(currentLength - restLength) / restLength;

            if (strain > body.material.D1) {
                body.isEroded[t] = 1;
                anyNewErosion = 1;
                break;
            }
        }
    }

    if (anyNewErosion != 0) {
        std::fill(body.isVertexActive.begin(), body.isVertexActive.end(), 0);
        #pragma omp parallel for if(useOpenMP)
        for (long long t = 0; t < numTets; ++t) {
            if (!body.isEroded[t]) {
                body.isVertexActive[body.tetrahedrons[t * 4 + 0]] = 1;
                body.isVertexActive[body.tetrahedrons[t * 4 + 1]] = 1;
                body.isVertexActive[body.tetrahedrons[t * 4 + 2]] = 1;
                body.isVertexActive[body.tetrahedrons[t * 4 + 3]] = 1;
            }
        }
    }
}

void bSolverEngine::integrate(bBodyState& body, float dt, float damping, bool useOpenMP) {
    long long numVertices = static_cast<long long>(body.positions.size() / 3);
    const float maxVel = 200.0f;

    #pragma omp parallel for if(useOpenMP)
    for (long long i = 0; i < numVertices; ++i) {
        if (!body.isVertexActive[i]) continue;
        float mass = body.masses[i];
        if (mass <= 1e-8f) continue;

        float invMass = 1.0f / mass;

        float vx = (body.velocities[i * 3 + 0] + body.forces[i * 3 + 0] * invMass * dt) * damping;
        float vy = (body.velocities[i * 3 + 1] + body.forces[i * 3 + 1] * invMass * dt) * damping;
        float vz = (body.velocities[i * 3 + 2] + body.forces[i * 3 + 2] * invMass * dt) * damping;

        body.velocities[i * 3 + 0] = std::clamp(vx, -maxVel, maxVel);
        body.velocities[i * 3 + 1] = std::clamp(vy, -maxVel, maxVel);
        body.velocities[i * 3 + 2] = std::clamp(vz, -maxVel, maxVel);

        body.positions[i * 3 + 0] += body.velocities[i * 3 + 0] * dt;
        body.positions[i * 3 + 1] += body.velocities[i * 3 + 1] * dt;
        body.positions[i * 3 + 2] += body.velocities[i * 3 + 2] * dt;
    }
}

void bSolverEngine::handleGroundCollision(bBodyState& body, float groundY, bool useOpenMP) {
    long long numVertices = static_cast<long long>(body.positions.size() / 3);

    #pragma omp parallel for if(useOpenMP)
    for (long long i = 0; i < numVertices; ++i) {
        if (body.positions[i * 3 + 1] < groundY) {
            body.positions[i * 3 + 1] = groundY;
            if (body.velocities[i * 3 + 1] < 0.0f) {
                body.velocities[i * 3 + 1] *= -0.3f;
                body.velocities[i * 3 + 0] *= 0.8f;
                body.velocities[i * 3 + 2] *= 0.8f;
            }
        }
    }
}