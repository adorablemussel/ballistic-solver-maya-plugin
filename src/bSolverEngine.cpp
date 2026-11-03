#include "bSolverEngine.h"
#include <cmath>
#include <map>
#include <algorithm>
#include <iostream>

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
    body.isVertexActive.resize(numVertices, true);

    for (size_t i = 0; i < numVertices; ++i) {
        body.velocities[i * 3 + 0] = initVel[0];
        body.velocities[i * 3 + 1] = initVel[1];
        body.velocities[i * 3 + 2] = initVel[2];
    }

    body.restVolumes.resize(numTets, 0.0f);
    body.isEroded.resize(numTets, false);

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

    // klucz: posortowane indeksy 3 wierzcho³ków œciany
    // wartoœæ: oryginalna kolejnoœæ wierzcho³ków (zgodna z normaln¹ na zewn¹trz) oraz licznik wyst¹pieñ
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

void bSolverEngine::pinVerticesInBox(size_t bodyIndex, float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
    if (bodyIndex >= bodies.size()) return;
    bBodyState& body = bodies[bodyIndex];
    size_t numVertices = body.positions.size() / 3;

    for (size_t i = 0; i < numVertices; ++i) {
        float x = body.positions[i * 3 + 0];
        float y = body.positions[i * 3 + 1];
        float z = body.positions[i * 3 + 2];

        if (x >= minX && x <= maxX && y >= minY && y <= maxY && z >= minZ && z <= maxZ) {
            body.masses[i] = 0.0f;
            body.velocities[i * 3 + 0] = 0.0f;
            body.velocities[i * 3 + 1] = 0.0f;
            body.velocities[i * 3 + 2] = 0.0f;
        }
    }
}

size_t bSolverEngine::getErodedCount(size_t bodyIndex) const {
    if (bodyIndex >= bodies.size()) return 0;
    size_t count = 0;
    for (bool eroded : bodies[bodyIndex].isEroded) {
        if (eroded) ++count;
    }
    return count;
}

float bSolverEngine::getMaxTemperature(size_t bodyIndex) const {
    if (bodyIndex >= bodies.size() || bodies[bodyIndex].temperatures.empty()) return 0.0f;
    return *std::max_element(bodies[bodyIndex].temperatures.begin(), bodies[bodyIndex].temperatures.end());
}

float bSolverEngine::getMaxPlasticStrain(size_t bodyIndex) const {
    if (bodyIndex >= bodies.size() || bodies[bodyIndex].plasticStrains.empty()) return 0.0f;
    return *std::max_element(bodies[bodyIndex].plasticStrains.begin(), bodies[bodyIndex].plasticStrains.end());
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

void bSolverEngine::step(float frameDt, int substeps, float damping) {
    if (substeps <= 0) substeps = 1;
    float dt = frameDt / static_cast<float>(substeps);

    for (int s = 0; s < substeps; ++s) {
        for (auto& body : bodies) {
            clearAndApplyExternalForces(body);
            computeInternalForces(body, dt);
        }

        handleBodyCollisions();

        for (auto& body : bodies) {
            evaluateErosion(body);
            integrate(body, dt, damping);
            handleGroundCollision(body, 0.0f);
        }
    }
}

void bSolverEngine::clearAndApplyExternalForces(bBodyState& body) {
    size_t numVertices = body.positions.size() / 3;
    const float gravityY = -9.81f;

    for (size_t i = 0; i < numVertices; ++i) {
        body.forces[i * 3 + 0] = 0.0f;
        body.forces[i * 3 + 1] = body.masses[i] * gravityY;
        body.forces[i * 3 + 2] = 0.0f;
    }
}

void bSolverEngine::computeInternalForces(bBodyState& body, float dt) {
    size_t numTets = body.tetrahedrons.size() / 4;
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

    int edges[6][2] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };

    for (size_t t = 0; t < numTets; ++t) {
        if (body.isEroded[t]) continue;

        float V0 = body.restVolumes[t];
        if (V0 <= 1e-12f) continue;

        float T = body.temperatures[t];
        float T_star = std::clamp((T - T_room) / (T_melt - T_room), 0.0f, 0.999f);
        float m_exp = (mat.m > 0.0f) ? mat.m : 1.0f;
        float thermalFactor = 1.0f - std::pow(T_star, m_exp);
        float E_eff = E_base * thermalFactor;

        int i0 = body.tetrahedrons[t * 4 + 0];
        int i1 = body.tetrahedrons[t * 4 + 1];
        int i2 = body.tetrahedrons[t * 4 + 2];
        int i3 = body.tetrahedrons[t * 4 + 3];

        const float* p0 = &body.positions[i0 * 3];
        const float* p1 = &body.positions[i1 * 3];
        const float* p2 = &body.positions[i2 * 3];
        const float* p3 = &body.positions[i3 * 3];

        float e1[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
        float e2[3] = { p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2] };
        float e3[3] = { p3[0] - p0[0], p3[1] - p0[1], p3[2] - p0[2] };

        float gradV1[3] = { (e2[1] * e3[2] - e2[2] * e3[1]) / 6.0f, (e2[2] * e3[0] - e2[0] * e3[2]) / 6.0f, (e2[0] * e3[1] - e2[1] * e3[0]) / 6.0f };
        float gradV2[3] = { (e3[1] * e1[2] - e3[2] * e1[1]) / 6.0f, (e3[2] * e1[0] - e3[0] * e1[2]) / 6.0f, (e3[0] * e1[1] - e3[1] * e1[0]) / 6.0f };
        float gradV3[3] = { (e1[1] * e2[2] - e1[2] * e2[1]) / 6.0f, (e1[2] * e2[0] - e1[0] * e2[2]) / 6.0f, (e1[0] * e2[1] - e1[1] * e2[0]) / 6.0f };
        float gradV0[3] = { -(gradV1[0] + gradV2[0] + gradV3[0]),
                            -(gradV1[1] + gradV2[1] + gradV3[1]),
                            -(gradV1[2] + gradV2[2] + gradV3[2]) };

        float currentV = e1[0] * gradV1[0] + e1[1] * gradV1[1] + e1[2] * gradV1[2];
        float safeV = std::max(currentV, 0.2f * V0);
        float mu = (V0 / safeV) - 1.0f;

        float pressure = 0.0f; 
        if (mat.eos_C0 > 0.0f) {
            float c0 = mat.eos_C0;
            float s = mat.eos_S;
            float gamma0 = mat.eos_Gamma0;
            float U = body.internalEnergies[t];

            if (mu > 0.0f) {
                float denom = std::max(1.0f - (s - 1.0f) * mu, 0.25f);
                float p_hugoniot = (rho0 * c0 * c0 * mu * (1.0f + (1.0f - 0.5f * gamma0) * mu)) / (denom * denom);
                pressure = p_hugoniot + gamma0 * rho0 * U;
            }
            else {
                pressure = rho0 * c0 * c0 * mu + gamma0 * rho0 * U;
            }
        }
        else {
            float volStrain = (currentV - V0) / V0;
            pressure = -K_linear * thermalFactor * volStrain;
        }

        for (int d = 0; d < 3; ++d) {
            body.forces[i0 * 3 + d] += pressure * gradV0[d];
            body.forces[i1 * 3 + d] += pressure * gradV1[d];
            body.forces[i2 * 3 + d] += pressure * gradV2[d];
            body.forces[i3 * 3 + d] += pressure * gradV3[d];
        }

        float maxTetPlasticInc = 0.0f;
        float maxEdgeStrainRate = 0.0f;
        float maxTrialStress = 0.0f;

        for (int e = 0; e < 6; ++e) {
            int iA = body.tetrahedrons[t * 4 + edges[e][0]];
            int iB = body.tetrahedrons[t * 4 + edges[e][1]];

            float* pA = &body.positions[iA * 3];
            float* pB = &body.positions[iB * 3];
            float* vA = &body.velocities[iA * 3];
            float* vB = &body.velocities[iB * 3];

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

                    float plasticWorkPerVol = yieldStress * plasticStrainInc;
                    body.internalEnergies[t] += plasticWorkPerVol / rho0;
                    body.temperatures[t] += (beta_TQ * plasticWorkPerVol) / (rho0 * Cp);
                }
            }

            maxTrialStress = std::max(maxTrialStress, trialStress);

            float k_edge = E_eff * V0 / (restLength * restLength);
            float c_edge = 0.1f * std::sqrt(k_edge * V0 * rho0);

            float springForce = k_edge * deltaL;
            float totalForce = springForce + relVel * c_edge;

            body.forces[iA * 3 + 0] += totalForce * nx;
            body.forces[iA * 3 + 1] += totalForce * ny;
            body.forces[iA * 3 + 2] += totalForce * nz;

            body.forces[iB * 3 + 0] -= totalForce * nx;
            body.forces[iB * 3 + 1] -= totalForce * ny;
            body.forces[iB * 3 + 2] -= totalForce * nz;
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
    (void)dt;
}

void bSolverEngine::handleBodyCollisions() {
    if (bodies.size() < 2) return;

    for (size_t a = 0; a < bodies.size(); ++a) {
        for (size_t b = a + 1; b < bodies.size(); ++b) {
            bBodyState& bodyA = bodies[a];
            bBodyState& bodyB = bodies[b];

            float EA = (bodyA.material.youngModulus > 0.0f) ? bodyA.material.youngModulus : 500000.0f;
            float EB = (bodyB.material.youngModulus > 0.0f) ? bodyB.material.youngModulus : 500000.0f;
            float E_eff = 0.5f * (EA + EB);

            auto collideTetsWithVertices = [&](bBodyState& tetBody, bBodyState& vertBody) {
                size_t numTets = tetBody.tetrahedrons.size() / 4;
                size_t numVerts = vertBody.positions.size() / 3;

                for (size_t t = 0; t < numTets; ++t) {
                    if (tetBody.isEroded[t]) continue;

                    int i0 = tetBody.tetrahedrons[t * 4 + 0];
                    int i1 = tetBody.tetrahedrons[t * 4 + 1];
                    int i2 = tetBody.tetrahedrons[t * 4 + 2];
                    int i3 = tetBody.tetrahedrons[t * 4 + 3];

                    float cX = 0.25f * (tetBody.positions[i0 * 3 + 0] + tetBody.positions[i1 * 3 + 0] + tetBody.positions[i2 * 3 + 0] + tetBody.positions[i3 * 3 + 0]);
                    float cY = 0.25f * (tetBody.positions[i0 * 3 + 1] + tetBody.positions[i1 * 3 + 1] + tetBody.positions[i2 * 3 + 1] + tetBody.positions[i3 * 3 + 1]);
                    float cZ = 0.25f * (tetBody.positions[i0 * 3 + 2] + tetBody.positions[i1 * 3 + 2] + tetBody.positions[i2 * 3 + 2] + tetBody.positions[i3 * 3 + 2]);

                    float rTet = 0.65f * std::cbrt(tetBody.restVolumes[t]);

                    for (size_t j = 0; j < numVerts; ++j) {
                        if (!vertBody.isVertexActive[j]) continue;

                        float* pV = &vertBody.positions[j * 3];
                        float* vV = &vertBody.velocities[j * 3];
                        float rV = vertBody.contactRadii[j];

                        float minDist = rTet + rV;
                        float dx = pV[0] - cX;
                        if (std::abs(dx) >= minDist) continue;
                        float dy = pV[1] - cY;
                        if (std::abs(dy) >= minDist) continue;
                        float dz = pV[2] - cZ;
                        if (std::abs(dz) >= minDist) continue;

                        float distSq = dx * dx + dy * dy + dz * dz;
                        if (distSq < minDist * minDist && distSq > 1e-12f) {
                            float dist = std::sqrt(distSq);
                            float overlap = minDist - dist;

                            float nx = dx / dist;
                            float ny = dy / dist;
                            float nz = dz / dist;

                            float k_contact = E_eff * minDist;
                            float forceMag = k_contact * overlap;

                            int tetIdxs[4] = { i0, i1, i2, i3 };
                            for (int k = 0; k < 4; ++k) {
                                int idx = tetIdxs[k];
                                tetBody.forces[idx * 3 + 0] -= 0.25f * forceMag * nx;
                                tetBody.forces[idx * 3 + 1] -= 0.25f * forceMag * ny;
                                tetBody.forces[idx * 3 + 2] -= 0.25f * forceMag * nz;
                            }

                            vertBody.forces[j * 3 + 0] += forceMag * nx;
                            vertBody.forces[j * 3 + 1] += forceMag * ny;
                            vertBody.forces[j * 3 + 2] += forceMag * nz;

                            float pushTet = 0.5f * overlap;
                            float pushVert = 0.5f * overlap;

                            for (int k = 0; k < 4; ++k) {
                                int idx = tetIdxs[k];
                                if (tetBody.masses[idx] > 1e-8f) {
                                    tetBody.positions[idx * 3 + 0] -= 0.25f * pushTet * nx;
                                    tetBody.positions[idx * 3 + 1] -= 0.25f * pushTet * ny;
                                    tetBody.positions[idx * 3 + 2] -= 0.25f * pushTet * nz;
                                }
                            }
                            if (vertBody.masses[j] > 1e-8f) {
                                pV[0] += pushVert * nx;
                                pV[1] += pushVert * ny;
                                pV[2] += pushVert * nz;
                                float vn = vV[0] * nx + vV[1] * ny + vV[2] * nz;
                                if (vn < 0.0f) {
                                    vV[0] -= 0.5f * vn * nx;
                                    vV[1] -= 0.5f * vn * ny;
                                    vV[2] -= 0.5f * vn * nz;
                                }
                            }
                        }
                    }
                }
                };

            collideTetsWithVertices(bodyA, bodyB);
            collideTetsWithVertices(bodyB, bodyA);
        }
    }
}

void bSolverEngine::evaluateErosion(bBodyState& body) {
    if (body.material.D1 <= 0.0f) return;

    size_t numTets = body.tetrahedrons.size() / 4;
    int edges[6][2] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };
    bool anyNewErosion = false;

    for (size_t t = 0; t < numTets; ++t) {
        if (body.isEroded[t]) continue;

        if (body.damageAccum[t] >= 1.0f ||
            (body.material.T_melt > 0.0f && body.temperatures[t] >= body.material.T_melt)) {
            body.isEroded[t] = true;
            anyNewErosion = true;
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
        if (currentV < 0.25f * body.restVolumes[t]) {
            body.isEroded[t] = true;
            anyNewErosion = true;
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
                body.isEroded[t] = true;
                anyNewErosion = true;
                break;
            }
        }
    }

    if (anyNewErosion) {
        std::fill(body.isVertexActive.begin(), body.isVertexActive.end(), false);
        for (size_t t = 0; t < numTets; ++t) {
            if (!body.isEroded[t]) {
                body.isVertexActive[body.tetrahedrons[t * 4 + 0]] = true;
                body.isVertexActive[body.tetrahedrons[t * 4 + 1]] = true;
                body.isVertexActive[body.tetrahedrons[t * 4 + 2]] = true;
                body.isVertexActive[body.tetrahedrons[t * 4 + 3]] = true;
            }
        }
    }
}
void bSolverEngine::integrate(bBodyState& body, float dt, float damping) {
    size_t numVertices = body.positions.size() / 3;

    for (size_t i = 0; i < numVertices; ++i) {
        if (!body.isVertexActive[i]) continue;
        float mass = body.masses[i];
        if (mass <= 1e-8f) continue;

        float invMass = 1.0f / mass;

        body.velocities[i * 3 + 0] = (body.velocities[i * 3 + 0] + body.forces[i * 3 + 0] * invMass * dt) * damping;
        body.velocities[i * 3 + 1] = (body.velocities[i * 3 + 1] + body.forces[i * 3 + 1] * invMass * dt) * damping;
        body.velocities[i * 3 + 2] = (body.velocities[i * 3 + 2] + body.forces[i * 3 + 2] * invMass * dt) * damping;

        body.positions[i * 3 + 0] += body.velocities[i * 3 + 0] * dt;
        body.positions[i * 3 + 1] += body.velocities[i * 3 + 1] * dt;
        body.positions[i * 3 + 2] += body.velocities[i * 3 + 2] * dt;
    }
}

void bSolverEngine::handleGroundCollision(bBodyState& body, float groundY) {
    size_t numVertices = body.positions.size() / 3;
    for (size_t i = 0; i < numVertices; ++i) {
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