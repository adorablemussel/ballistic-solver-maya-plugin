#include "bSolverCUDA.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cmath>
#include <algorithm>

namespace {

    __device__ __forceinline__ float d_clamp(float val, float minVal, float maxVal) {
        return fminf(fmaxf(val, minVal), maxVal);
    }

    __device__ __forceinline__ float d_computeSignedTetVolume(
        const float* p0, const float* p1, const float* p2, const float* p3)
    {
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

    // 1. czyszczenie si³ i aplikacja grawitacji (1 w¹tek = 1 wierzcho³ek)
    __global__ void kClearAndApplyExternalForces(
        int numVertices,
        float* forces,
        const float* masses)
    {
        int i = blockIdx.x * blockDim.x + threadIdx.x;
        if (i >= numVertices) return;

        forces[i * 3 + 0] = 0.0f;
        forces[i * 3 + 1] = masses[i] * (-9.81f);
        forces[i * 3 + 2] = 0.0f;
    }

    // 2. si³y wewnêtrzne, plastycznoœæ JC, termomechanika i EOS (1 w¹tek = 1 czworoœcian)
    __global__ void kComputeInternalForces(
        int numTets,
        bMaterial mat,
        float dt,
        float timeScale,
        const int* tetrahedrons,
        const float* restVolumes,
        float* restEdgeLengths,
        const uint8_t* isEroded,
        const float* positions,
        const float* velocities,
        float* forces,
        float* temperatures,
        float* plasticStrains,
        float* internalEnergies,
        float* damageAccum)
    {
        int t = blockIdx.x * blockDim.x + threadIdx.x;
        if (t >= numTets) return;
        if (isEroded[t]) return;

        float V0 = restVolumes[t];
        if (V0 <= 1e-12f) return;

        float E_base = (mat.youngModulus > 0.0f) ? mat.youngModulus : 500000.0f;
        float nu = d_clamp(mat.poissonRatio, 0.0f, 0.45f);
        float rho0 = (mat.density > 0.0f) ? mat.density : 1000.0f;
        float K_linear = E_base / (3.0f * (1.0f - 2.0f * nu));

        float T_room = (mat.T_room > 0.0f) ? mat.T_room : 293.15f;
        float T_melt = (mat.T_melt > T_room) ? mat.T_melt : 1800.0f;
        float Cp = (mat.specificHeat > 0.0f) ? mat.specificHeat : 470.0f;
        float beta_TQ = (mat.taylorQuinney > 0.0f) ? mat.taylorQuinney : 0.9f;
        float eps_dot_0 = (mat.eps_dot_0 > 0.0f) ? mat.eps_dot_0 : 1.0f;

        float tetNodeMass = 0.25f * rho0 * V0;
        float baseDt = (timeScale > 0.001f) ? (dt / timeScale) : dt;
        float maxStableK = (baseDt > 1e-7f) ? (0.12f * tetNodeMass / (baseDt * baseDt)) : 1e30f;

        float T = temperatures[t];
        float T_star = d_clamp((T - T_room) / (T_melt - T_room), 0.0f, 0.999f);
        float m_exp = (mat.m > 0.0f) ? mat.m : 1.0f;
        float thermalFactor = 1.0f - powf(T_star, m_exp);
        float E_eff = E_base * thermalFactor;

        int tetVerts[4] = {
            tetrahedrons[t * 4 + 0],
            tetrahedrons[t * 4 + 1],
            tetrahedrons[t * 4 + 2],
            tetrahedrons[t * 4 + 3]
        };

        const float* p0 = &positions[tetVerts[0] * 3];
        const float* p1 = &positions[tetVerts[1] * 3];
        const float* p2 = &positions[tetVerts[2] * 3];
        const float* p3 = &positions[tetVerts[3] * 3];

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
            maxGradSq = fmaxf(maxGradSq, gSq);
        }

        float maxStableBulkModulus = maxStableK * V0 / maxGradSq;

        float currentV = e1[0] * gradV1[0] + e1[1] * gradV1[1] + e1[2] * gradV1[2];
        float safeV = fmaxf(currentV, 0.35f * V0);
        float mu = d_clamp((V0 / safeV) - 1.0f, -0.5f, 1.5f);

        float pressure = 0.0f;
        if (mat.eos_C0 > 0.0f && mu > 0.0f) {
            float K_eos = fminf(rho0 * mat.eos_C0 * mat.eos_C0, maxStableBulkModulus);
            float s = mat.eos_S;
            float gamma0 = mat.eos_Gamma0;
            float denom = fmaxf(1.0f - (s - 1.0f) * mu, 0.35f);
            pressure = (K_eos * mu * (1.0f + (1.0f - 0.5f * gamma0) * mu)) / (denom * denom);
        }
        else {
            float K_eff = fminf(K_linear * thermalFactor, maxStableBulkModulus);
            float volStrain = d_clamp((currentV - V0) / V0, -0.5f, 0.5f);
            pressure = -K_eff * volStrain;
        }

        float localF[4][3] = {
            { pressure * gradV0[0], pressure * gradV0[1], pressure * gradV0[2] },
            { pressure * gradV1[0], pressure * gradV1[1], pressure * gradV1[2] },
            { pressure * gradV2[0], pressure * gradV2[1], pressure * gradV2[2] },
            { pressure * gradV3[0], pressure * gradV3[1], pressure * gradV3[2] }
        };

        const int edges[6][2] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };

        float maxTetPlasticInc = 0.0f;
        float maxEdgeStrainRate = 0.0f;
        float maxTrialStress = 0.0f;

        for (int e = 0; e < 6; ++e) {
            int localA = edges[e][0];
            int localB = edges[e][1];
            int iA = tetVerts[localA];
            int iB = tetVerts[localB];

            const float* pA = &positions[iA * 3];
            const float* pB = &positions[iB * 3];
            const float* vA = &velocities[iA * 3];
            const float* vB = &velocities[iB * 3];

            float dx = pB[0] - pA[0];
            float dy = pB[1] - pA[1];
            float dz = pB[2] - pA[2];
            float currentLength = sqrtf(dx * dx + dy * dy + dz * dz);

            if (currentLength < 1e-6f) continue;

            float nx = dx / currentLength;
            float ny = dy / currentLength;
            float nz = dz / currentLength;

            float restLength = restEdgeLengths[t * 6 + e];
            float deltaL = currentLength - restLength;
            float elasticStrain = fabsf(deltaL) / restLength;

            float relVel = (vB[0] - vA[0]) * nx + (vB[1] - vA[1]) * ny + (vB[2] - vA[2]) * nz;
            float strainRate = fabsf(relVel) / restLength;
            maxEdgeStrainRate = fmaxf(maxEdgeStrainRate, strainRate);

            float trialStress = E_eff * elasticStrain;

            if (mat.A > 0.0f) {
                float eps_p = plasticStrains[t];
                float n_exp = (mat.n > 0.0f) ? mat.n : 1.0f;
                float hardTerm = mat.A + mat.B * powf(eps_p, n_exp);

                float rateRatio = fmaxf(strainRate / eps_dot_0, 1.0f);
                float rateTerm = 1.0f + mat.C * logf(rateRatio);

                float yieldStress = hardTerm * rateTerm * thermalFactor;

                if (trialStress > yieldStress && E_eff > 1e-6f) {
                    float plasticStrainInc = (trialStress - yieldStress) / E_eff;
                    trialStress = yieldStress;

                    float sign = (deltaL >= 0.0f) ? 1.0f : -1.0f;
                    restLength = fmaxf(restLength + sign * plasticStrainInc * restLength, 1e-5f);
                    restEdgeLengths[t * 6 + e] = restLength;
                    deltaL = currentLength - restLength;

                    maxTetPlasticInc = fmaxf(maxTetPlasticInc, plasticStrainInc);

                    float plasticWorkPerVol = yieldStress * (plasticStrainInc / 6.0f);
                    internalEnergies[t] += plasticWorkPerVol / rho0;
                    temperatures[t] += (beta_TQ * plasticWorkPerVol) / (rho0 * Cp);
                }
            }

            maxTrialStress = fmaxf(maxTrialStress, trialStress);

            float k_edge = fminf(E_eff * V0 / (restLength * restLength), maxStableK);
            float c_edge = 0.1f * sqrtf(k_edge * tetNodeMass);

            float totalForce = k_edge * deltaL + relVel * c_edge;

            localF[localA][0] += totalForce * nx;
            localF[localA][1] += totalForce * ny;
            localF[localA][2] += totalForce * nz;

            localF[localB][0] -= totalForce * nx;
            localF[localB][1] -= totalForce * ny;
            localF[localB][2] -= totalForce * nz;
        }

        for (int v = 0; v < 4; ++v) {
            int globalIdx = tetVerts[v] * 3;
            atomicAdd(&forces[globalIdx + 0], localF[v][0]);
            atomicAdd(&forces[globalIdx + 1], localF[v][1]);
            atomicAdd(&forces[globalIdx + 2], localF[v][2]);
        }

        if (maxTetPlasticInc > 0.0f) {
            plasticStrains[t] += maxTetPlasticInc;

            if (mat.D1 > 0.0f) {
                float sigma_eq = fmaxf(maxTrialStress, 1.0f);
                float triaxiality = d_clamp(-pressure / sigma_eq, -2.0f, 2.0f);

                float d123 = mat.D1 + mat.D2 * expf(mat.D3 * triaxiality);
                float rateRatio = fmaxf(maxEdgeStrainRate / eps_dot_0, 1.0f);
                float d4_term = 1.0f + mat.D4 * logf(rateRatio);
                float d5_term = 1.0f + mat.D5 * T_star;

                float fractureStrain = fmaxf(d123 * d4_term * d5_term, 1e-4f);
                damageAccum[t] += maxTetPlasticInc / fractureStrain;
            }
        }
    }

    // 3. kolizje pomiêdzy cia³ami (1 w¹tek = 1 wierzcho³ek cia³a A)
    __global__ void kHandleBodyCollisions(
        int numVA,
        int numVB,
        float E_eff,
        float dt,
        const float* posA, const float* velA, const float* massA, const float* radA, const uint8_t* activeA, float* forcesA,
        const float* posB, const float* velB, const float* massB, const float* radB, const uint8_t* activeB, float* forcesB)
    {
        int i = blockIdx.x * blockDim.x + threadIdx.x;
        if (i >= numVA) return;
        if (!activeA[i]) return;

        const float* pA = &posA[i * 3];
        const float* vA = &velA[i * 3];
        float rA = radA[i];
        float mA = (massA[i] > 1e-8f) ? massA[i] : 1e30f;

        float sumForceAx = 0.0f;
        float sumForceAy = 0.0f;
        float sumForceAz = 0.0f;

        for (int j = 0; j < numVB; ++j) {
            if (!activeB[j]) continue;

            const float* pB = &posB[j * 3];
            float rB = radB[j];

            float minDist = 1.3f * (rA + rB);
            float dx = pB[0] - pA[0];
            if (fabsf(dx) >= minDist) continue;
            float dy = pB[1] - pA[1];
            if (fabsf(dy) >= minDist) continue;
            float dz = pB[2] - pA[2];
            if (fabsf(dz) >= minDist) continue;

            float distSq = dx * dx + dy * dy + dz * dz;
            if (distSq < minDist * minDist && distSq > 1e-12f) {
                float dist = sqrtf(distSq);
                float overlap = minDist - dist;

                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;

                float mB = (massB[j] > 1e-8f) ? massB[j] : 1e30f;
                float minMass = fminf(mA, mB);

                float k_contact = 0.5f * E_eff * minDist;
                if (minMass < 1e20f && dt > 1e-7f) {
                    k_contact = fminf(k_contact, 0.05f * minMass / (dt * dt));
                }

                float forceMag = k_contact * overlap;

                const float* vB = &velB[j * 3];
                float relVel = (vB[0] - vA[0]) * nx + (vB[1] - vA[1]) * ny + (vB[2] - vA[2]) * nz;
                if (relVel < 0.0f && minMass < 1e20f) {
                    forceMag -= 0.25f * relVel * sqrtf(k_contact * minMass);
                }

                float fx = forceMag * nx;
                float fy = forceMag * ny;
                float fz = forceMag * nz;

                sumForceAx -= fx;
                sumForceAy -= fy;
                sumForceAz -= fz;

                atomicAdd(&forcesB[j * 3 + 0], fx);
                atomicAdd(&forcesB[j * 3 + 1], fy);
                atomicAdd(&forcesB[j * 3 + 2], fz);
            }
        }

        forcesA[i * 3 + 0] += sumForceAx;
        forcesA[i * 3 + 1] += sumForceAy;
        forcesA[i * 3 + 2] += sumForceAz;
    }

    // 4A. ewaluacja kryteriów zniszczenia / erozji (1 w¹tek = 1 czworoœcian)
    __global__ void kEvaluateErosion(
        int numTets,
        bMaterial mat,
        const int* tetrahedrons,
        const float* positions,
        const float* restVolumes,
        const float* restEdgeLengths,
        const float* damageAccum,
        const float* temperatures,
        uint8_t* isEroded,
        int* anyNewErosion)
    {
        int t = blockIdx.x * blockDim.x + threadIdx.x;
        if (t >= numTets) return;
        if (isEroded[t]) return;

        if (damageAccum[t] >= 1.0f || (mat.T_melt > 0.0f && temperatures[t] >= mat.T_melt)) {
            isEroded[t] = 1;
            *anyNewErosion = 1;
            return;
        }

        int i0 = tetrahedrons[t * 4 + 0];
        int i1 = tetrahedrons[t * 4 + 1];
        int i2 = tetrahedrons[t * 4 + 2];
        int i3 = tetrahedrons[t * 4 + 3];

        float currentV = d_computeSignedTetVolume(
            &positions[i0 * 3], &positions[i1 * 3], &positions[i2 * 3], &positions[i3 * 3]);

        if (currentV < 0.2f * restVolumes[t]) {
            isEroded[t] = 1;
            *anyNewErosion = 1;
            return;
        }

        const int edges[6][2] = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };
        for (int e = 0; e < 6; ++e) {
            int iA = tetrahedrons[t * 4 + edges[e][0]];
            int iB = tetrahedrons[t * 4 + edges[e][1]];

            const float* pA = &positions[iA * 3];
            const float* pB = &positions[iB * 3];

            float dx = pB[0] - pA[0];
            float dy = pB[1] - pA[1];
            float dz = pB[2] - pA[2];
            float currentLength = sqrtf(dx * dx + dy * dy + dz * dz);
            float restLength = restEdgeLengths[t * 6 + e];

            float strain = fabsf(currentLength - restLength) / restLength;
            if (strain > mat.D1) {
                isEroded[t] = 1;
                *anyNewErosion = 1;
                break;
            }
        }
    }

    // 4B. odbudowa flag aktywnych wierzcho³ków po erozji
    __global__ void kRebuildActiveVertices(
        int numTets,
        const int* tetrahedrons,
        const uint8_t* isEroded,
        uint8_t* isVertexActive)
    {
        int t = blockIdx.x * blockDim.x + threadIdx.x;
        if (t >= numTets) return;

        if (!isEroded[t]) {
            isVertexActive[tetrahedrons[t * 4 + 0]] = 1;
            isVertexActive[tetrahedrons[t * 4 + 1]] = 1;
            isVertexActive[tetrahedrons[t * 4 + 2]] = 1;
            isVertexActive[tetrahedrons[t * 4 + 3]] = 1;
        }
    }

    // 5. ca³kowanie pó³-jawne Eulera po³¹czone z kolizj¹ z pod³o¿em (1 w¹tek = 1 wierzcho³ek)
    __global__ void kIntegrateAndGroundCollision(
        int numVertices,
        float dt,
        float damping,
        float groundY,
        float* positions,
        float* velocities,
        const float* forces,
        const float* masses,
        const uint8_t* isVertexActive)
    {
        int i = blockIdx.x * blockDim.x + threadIdx.x;
        if (i >= numVertices) return;

        if (isVertexActive[i]) {
            float mass = masses[i];
            if (mass > 1e-8f) {
                float invMass = 1.0f / mass;
                const float maxVel = 200.0f;

                float vx = (velocities[i * 3 + 0] + forces[i * 3 + 0] * invMass * dt) * damping;
                float vy = (velocities[i * 3 + 1] + forces[i * 3 + 1] * invMass * dt) * damping;
                float vz = (velocities[i * 3 + 2] + forces[i * 3 + 2] * invMass * dt) * damping;

                vx = d_clamp(vx, -maxVel, maxVel);
                vy = d_clamp(vy, -maxVel, maxVel);
                vz = d_clamp(vz, -maxVel, maxVel);

                velocities[i * 3 + 0] = vx;
                velocities[i * 3 + 1] = vy;
                velocities[i * 3 + 2] = vz;

                positions[i * 3 + 0] += vx * dt;
                positions[i * 3 + 1] += vy * dt;
                positions[i * 3 + 2] += vz * dt;
            }
        }

        if (positions[i * 3 + 1] < groundY) {
            positions[i * 3 + 1] = groundY;
            if (velocities[i * 3 + 1] < 0.0f) {
                velocities[i * 3 + 1] *= -0.3f;
                velocities[i * 3 + 0] *= 0.8f;
                velocities[i * 3 + 2] *= 0.8f;
            }
        }
    }

    void allocateAndSyncStaticBuffers(
        const std::vector<bBodyState>& bodies,
        std::vector<bCudaBodyBuffer>& gpuBodies)
    {
        bool needsRealloc = (gpuBodies.size() != bodies.size());
        if (!needsRealloc) {
            for (size_t i = 0; i < bodies.size(); ++i) {
                if (gpuBodies[i].numVertices != bodies[i].positions.size() / 3 ||
                    gpuBodies[i].numTets != bodies[i].tetrahedrons.size() / 4) {
                    needsRealloc = true;
                    break;
                }
            }
        }

        if (!needsRealloc) return;

        bFreeCudaBuffers(gpuBodies);
        gpuBodies.resize(bodies.size());

        for (size_t i = 0; i < bodies.size(); ++i) {
            const auto& b = bodies[i];
            auto& g = gpuBodies[i];

            g.numVertices = b.positions.size() / 3;
            g.numTets = b.tetrahedrons.size() / 4;

            cudaMalloc(&g.d_positions, b.positions.size() * sizeof(float));
            cudaMalloc(&g.d_velocities, b.velocities.size() * sizeof(float));
            cudaMalloc(&g.d_forces, b.forces.size() * sizeof(float));
            cudaMalloc(&g.d_masses, b.masses.size() * sizeof(float));
            cudaMalloc(&g.d_contactRadii, b.contactRadii.size() * sizeof(float));
            cudaMalloc(&g.d_isVertexActive, b.isVertexActive.size() * sizeof(uint8_t));

            cudaMalloc(&g.d_tetrahedrons, b.tetrahedrons.size() * sizeof(int));
            cudaMalloc(&g.d_restVolumes, b.restVolumes.size() * sizeof(float));
            cudaMalloc(&g.d_restEdgeLengths, b.restEdgeLengths.size() * sizeof(float));
            cudaMalloc(&g.d_isEroded, b.isEroded.size() * sizeof(uint8_t));
            cudaMalloc(&g.d_temperatures, b.temperatures.size() * sizeof(float));
            cudaMalloc(&g.d_plasticStrains, b.plasticStrains.size() * sizeof(float));
            cudaMalloc(&g.d_internalEnergies, b.internalEnergies.size() * sizeof(float));
            cudaMalloc(&g.d_damageAccum, b.damageAccum.size() * sizeof(float));
            cudaMalloc(&g.d_anyNewErosion, sizeof(int));

            // zmienne termodynamiczne tylko w VRAM, nie trzeba marnowaæ PCIe na ich pobieranie
            cudaMemcpy(g.d_masses, b.masses.data(), b.masses.size() * sizeof(float), cudaMemcpyHostToDevice);
            cudaMemcpy(g.d_contactRadii, b.contactRadii.data(), b.contactRadii.size() * sizeof(float), cudaMemcpyHostToDevice);
            cudaMemcpy(g.d_tetrahedrons, b.tetrahedrons.data(), b.tetrahedrons.size() * sizeof(int), cudaMemcpyHostToDevice);
            cudaMemcpy(g.d_restVolumes, b.restVolumes.data(), b.restVolumes.size() * sizeof(float), cudaMemcpyHostToDevice);

            cudaMemcpy(g.d_restEdgeLengths, b.restEdgeLengths.data(), b.restEdgeLengths.size() * sizeof(float), cudaMemcpyHostToDevice);
            cudaMemcpy(g.d_isEroded, b.isEroded.data(), b.isEroded.size() * sizeof(uint8_t), cudaMemcpyHostToDevice);
            cudaMemcpy(g.d_temperatures, b.temperatures.data(), b.temperatures.size() * sizeof(float), cudaMemcpyHostToDevice);
            cudaMemcpy(g.d_plasticStrains, b.plasticStrains.data(), b.plasticStrains.size() * sizeof(float), cudaMemcpyHostToDevice);
            cudaMemcpy(g.d_internalEnergies, b.internalEnergies.data(), b.internalEnergies.size() * sizeof(float), cudaMemcpyHostToDevice);
            cudaMemcpy(g.d_damageAccum, b.damageAccum.data(), b.damageAccum.size() * sizeof(float), cudaMemcpyHostToDevice);
        }
    }

} // namespace

void bFreeCudaBuffers(std::vector<bCudaBodyBuffer>& gpuBodies) {
    for (auto& g : gpuBodies) {
        if (g.d_positions) cudaFree(g.d_positions);
        if (g.d_velocities) cudaFree(g.d_velocities);
        if (g.d_forces) cudaFree(g.d_forces);
        if (g.d_masses) cudaFree(g.d_masses);
        if (g.d_contactRadii) cudaFree(g.d_contactRadii);
        if (g.d_isVertexActive) cudaFree(g.d_isVertexActive);

        if (g.d_tetrahedrons) cudaFree(g.d_tetrahedrons);
        if (g.d_restVolumes) cudaFree(g.d_restVolumes);
        if (g.d_restEdgeLengths) cudaFree(g.d_restEdgeLengths);
        if (g.d_isEroded) cudaFree(g.d_isEroded);
        if (g.d_temperatures) cudaFree(g.d_temperatures);
        if (g.d_plasticStrains) cudaFree(g.d_plasticStrains);
        if (g.d_internalEnergies) cudaFree(g.d_internalEnergies);
        if (g.d_damageAccum) cudaFree(g.d_damageAccum);
        if (g.d_anyNewErosion) cudaFree(g.d_anyNewErosion);
    }
    gpuBodies.clear();
}

void bStepCUDA(
    std::vector<bBodyState>& bodies,
    std::vector<bCudaBodyBuffer>& gpuBodies,
    float frameDt,
    int substeps,
    float damping,
    float timeScale)
{
    if (bodies.empty()) return;
    if (substeps <= 0) substeps = 1;
    float dt = frameDt / static_cast<float>(substeps);

    float substepDamping = powf(damping, (40.0f * timeScale) / static_cast<float>(substeps));

    allocateAndSyncStaticBuffers(bodies, gpuBodies);

    for (size_t i = 0; i < bodies.size(); ++i) {
        const auto& b = bodies[i];
        auto& g = gpuBodies[i];

        cudaMemcpy(g.d_positions, b.positions.data(), b.positions.size() * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(g.d_velocities, b.velocities.data(), b.velocities.size() * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(g.d_isVertexActive, b.isVertexActive.data(), b.isVertexActive.size() * sizeof(uint8_t), cudaMemcpyHostToDevice);
    }

    const int blockSize = 256;

    for (int s = 0; s < substeps; ++s) {

        for (size_t i = 0; i < bodies.size(); ++i) {
            auto& g = gpuBodies[i];
            int numV = static_cast<int>(g.numVertices);
            int numT = static_cast<int>(g.numTets);

            if (numV > 0) {
                int gridV = (numV + blockSize - 1) / blockSize;
                kClearAndApplyExternalForces << <gridV, blockSize >> > (numV, g.d_forces, g.d_masses);
            }

            if (numT > 0) {
                int gridT = (numT + blockSize - 1) / blockSize;
                kComputeInternalForces << <gridT, blockSize >> > (
                    numT, bodies[i].material, dt, timeScale,
                    g.d_tetrahedrons, g.d_restVolumes, g.d_restEdgeLengths, g.d_isEroded,
                    g.d_positions, g.d_velocities, g.d_forces,
                    g.d_temperatures, g.d_plasticStrains, g.d_internalEnergies, g.d_damageAccum);
            }
        }

        for (size_t a = 0; a < bodies.size(); ++a) {
            for (size_t b = a + 1; b < bodies.size(); ++b) {
                auto& gA = gpuBodies[a];
                auto& gB = gpuBodies[b];
                int numVA = static_cast<int>(gA.numVertices);
                int numVB = static_cast<int>(gB.numVertices);
                if (numVA == 0 || numVB == 0) continue;

                float EA = (bodies[a].material.youngModulus > 0.0f) ? bodies[a].material.youngModulus : 500000.0f;
                float EB = (bodies[b].material.youngModulus > 0.0f) ? bodies[b].material.youngModulus : 500000.0f;
                float E_eff = 0.5f * (EA + EB);

                int gridVA = (numVA + blockSize - 1) / blockSize;
                kHandleBodyCollisions << <gridVA, blockSize >> > (
                    numVA, numVB, E_eff, dt,
                    gA.d_positions, gA.d_velocities, gA.d_masses, gA.d_contactRadii, gA.d_isVertexActive, gA.d_forces,
                    gB.d_positions, gB.d_velocities, gB.d_masses, gB.d_contactRadii, gB.d_isVertexActive, gB.d_forces);
            }
        }

        for (size_t i = 0; i < bodies.size(); ++i) {
            auto& g = gpuBodies[i];
            int numV = static_cast<int>(g.numVertices);
            int numT = static_cast<int>(g.numTets);

            if (bodies[i].material.D1 > 0.0f && numT > 0) {
                int gridT = (numT + blockSize - 1) / blockSize;
                kEvaluateErosion << <gridT, blockSize >> > (
                    numT, bodies[i].material,
                    g.d_tetrahedrons, g.d_positions, g.d_restVolumes, g.d_restEdgeLengths,
                    g.d_damageAccum, g.d_temperatures, g.d_isEroded, g.d_anyNewErosion);

                cudaMemsetAsync(g.d_isVertexActive, 0, numV * sizeof(uint8_t));
                kRebuildActiveVertices << <gridT, blockSize >> > (
                    numT, g.d_tetrahedrons, g.d_isEroded, g.d_isVertexActive);
            }

            if (numV > 0) {
                int gridV = (numV + blockSize - 1) / blockSize;

                kIntegrateAndGroundCollision << <gridV, blockSize >> > (
                    numV, dt, substepDamping, 0.0f,
                    g.d_positions, g.d_velocities, g.d_forces, g.d_masses, g.d_isVertexActive);
            }
        }
    }

    cudaDeviceSynchronize();

    for (size_t i = 0; i < bodies.size(); ++i) {
        auto& b = bodies[i];
        const auto& g = gpuBodies[i];

        cudaMemcpy(b.positions.data(), g.d_positions, b.positions.size() * sizeof(float), cudaMemcpyDeviceToHost);
        cudaMemcpy(b.velocities.data(), g.d_velocities, b.velocities.size() * sizeof(float), cudaMemcpyDeviceToHost);
        cudaMemcpy(b.isVertexActive.data(), g.d_isVertexActive, b.isVertexActive.size() * sizeof(uint8_t), cudaMemcpyDeviceToHost);
        cudaMemcpy(b.isEroded.data(), g.d_isEroded, b.isEroded.size() * sizeof(uint8_t), cudaMemcpyDeviceToHost);
    }
}