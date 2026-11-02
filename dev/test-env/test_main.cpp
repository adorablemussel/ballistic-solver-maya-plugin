#include <iostream>
#include <fstream>
#include <iomanip>
#include "bSolverEngine.h"

void generateTetBlock(int nx, int ny, int nz, 
                      float dx, float dy, float dz,
                      float offsetX, float offsetY, float offsetZ,
                      std::vector<float>& outVertices, 
                      std::vector<int>& outTets) 
{
    outVertices.clear();
    outTets.clear();

    auto getVertexIndex = [&](int x, int y, int z) {
        return x * (ny + 1) * (nz + 1) + y * (nz + 1) + z;
    };

    for (int x = 0; x <= nx; ++x) {
        for (int y = 0; y <= ny; ++y) {
            for (int z = 0; z <= nz; ++z) {
                outVertices.push_back(offsetX + x * dx);
                outVertices.push_back(offsetY + y * dy);
                outVertices.push_back(offsetZ + z * dz);
            }
        }
    }

    for (int x = 0; x < nx; ++x) {
        for (int y = 0; y < ny; ++y) {
            for (int z = 0; z < nz; ++z) {
                int v000 = getVertexIndex(x,     y,     z);
                int v100 = getVertexIndex(x + 1, y,     z);
                int v010 = getVertexIndex(x,     y + 1, z);
                int v110 = getVertexIndex(x + 1, y + 1, z);
                int v001 = getVertexIndex(x,     y,     z + 1);
                int v101 = getVertexIndex(x + 1, y,     z + 1);
                int v011 = getVertexIndex(x,     y + 1, z + 1);
                int v111 = getVertexIndex(x + 1, y + 1, z + 1);

                int cellTets[6][4] = {
                    { v000, v100, v110, v111 },
                    { v000, v100, v101, v111 },
                    { v000, v010, v110, v111 },
                    { v000, v010, v011, v111 },
                    { v000, v001, v101, v111 },
                    { v000, v001, v011, v111 }
                };

                for (int t = 0; t < 6; ++t) {
                    outTets.push_back(cellTets[t][0]);
                    outTets.push_back(cellTets[t][1]);
                    outTets.push_back(cellTets[t][2]);
                    outTets.push_back(cellTets[t][3]);
                }
            }
        }
    }
}

void exportSceneToOBJ(const std::vector<SimulatedBody>& bodies, int frameNumber) {
    std::string filename = "frame_" + std::to_string(frameNumber) + ".obj";
    std::ofstream out(filename);
    
    int vertexOffset = 1;

    for (size_t b = 0; b < bodies.size(); ++b) {
        const auto& body = bodies[b];
        out << "o Body_" << b << "\n";

        for (size_t i = 0; i < body.positions.size() / 3; ++i) {
            out << "v " << body.positions[i * 3 + 0] << " "
                        << body.positions[i * 3 + 1] << " "
                        << body.positions[i * 3 + 2] << "\n";
        }

        for (size_t t = 0; t < body.tetrahedrons.size() / 4; ++t) {
            if (body.isEroded[t]) continue;
            int i0 = body.tetrahedrons[t * 4 + 0] + vertexOffset;
            int i1 = body.tetrahedrons[t * 4 + 1] + vertexOffset;
            int i2 = body.tetrahedrons[t * 4 + 2] + vertexOffset;
            int i3 = body.tetrahedrons[t * 4 + 3] + vertexOffset;

            out << "f " << i0 << " " << i2 << " " << i1 << "\n";
            out << "f " << i0 << " " << i1 << " " << i3 << "\n";
            out << "f " << i0 << " " << i3 << " " << i2 << "\n";
            out << "f " << i1 << " " << i2 << " " << i3 << "\n";
        }

        vertexOffset += static_cast<int>(body.positions.size() / 3);
    }
}

int main() {
    bSolverEngine solver;

    bMaterial plateMat{};
    plateMat.density = 2500.0f;
    plateMat.youngModulus = 2000000.0f;
    plateMat.poissonRatio = 0.25f;

    plateMat.A = 60000.0f;
    plateMat.B = 100000.0f;
    plateMat.n = 0.35f;
    plateMat.C = 0.02f;
    plateMat.m = 1.0f;

    plateMat.D1 = 0.12f;
    plateMat.D2 = 0.05f;
    plateMat.D3 = -1.5f;
    plateMat.D4 = 0.01f;
    plateMat.D5 = 0.5f;

    plateMat.T_room = 293.15f;
    plateMat.T_melt = 900.0f;
    plateMat.specificHeat = 0.5f;
    plateMat.eps_dot_0 = 1.0f;
    plateMat.taylorQuinney = 0.9f;

    plateMat.eos_C0 = 25.0f;
    plateMat.eos_S = 1.4f;
    plateMat.eos_Gamma0 = 1.8f;

    std::vector<float> plateVerts;
    std::vector<int> plateTets;
    generateTetBlock(8, 8, 2, 0.1f, 0.1f, 0.1f, -0.4f, 0.6f, -0.1f, plateVerts, plateTets);
    solver.addBody(plateMat, plateVerts, plateTets, {0.0f, 0.0f, 0.0f});

    solver.pinVerticesInBox(0, -0.45f, -0.35f, 0.55f, 1.45f, -0.15f, 0.15f);
    solver.pinVerticesInBox(0,  0.35f,  0.45f, 0.55f, 1.45f, -0.15f, 0.15f);
    solver.pinVerticesInBox(0, -0.45f,  0.45f, 0.55f, 0.65f, -0.15f, 0.15f);
    solver.pinVerticesInBox(0, -0.45f,  0.45f, 1.35f, 1.45f, -0.15f, 0.15f);

    bMaterial bulletMat{};
    bulletMat.density = 15000.0f;
    bulletMat.youngModulus = 10000000.0f;
    bulletMat.poissonRatio = 0.3f;
    bulletMat.T_room = 293.15f;

    std::vector<float> bulletVerts;
    std::vector<int> bulletTets;
    generateTetBlock(2, 2, 2, 0.06f, 0.06f, 0.06f, -0.06f, 0.94f, -0.26f, bulletVerts, bulletTets);
    solver.addBody(bulletMat, bulletVerts, bulletTets, {0.0f, 0.0f, 20.0f});

    float frameDt = 1.0f / 200.0f;
    int substeps = 20;

    size_t totalPlateTets = plateTets.size() / 4;
    std::cout << "Start symulacji. Czworoscianow w plycie: " << totalPlateTets << "\n";

    exportSceneToOBJ(solver.getBodies(), 0);
    std::cout << "Klatka  0 | Pocisk Z: " << std::fixed << std::setprecision(4) << solver.getBodies()[1].positions[2]
              << " | Vz: " << std::setw(6) << std::setprecision(2) << solver.getBodies()[1].velocities[2]
              << " | Plast: " << std::setprecision(3) << solver.getMaxPlasticStrain(0)
              << " | Max T: " << std::setprecision(1) << solver.getMaxTemperature(0) << " K"
              << " | Erozja: 0 / " << totalPlateTets << "\n";

    for (int frame = 1; frame <= 20; ++frame) {
        solver.step(frameDt, substeps);
        const auto& bullet = solver.getBodies()[1];

        std::cout << "Klatka " << std::setw(2) << frame 
                  << " | Pocisk Z: " << std::fixed << std::setprecision(4) << bullet.positions[2]
                  << " | Vz: " << std::setw(6) << std::setprecision(2) << bullet.velocities[2]
                  << " | Plast: " << std::setprecision(3) << solver.getMaxPlasticStrain(0)
                  << " | Max T: " << std::setprecision(1) << solver.getMaxTemperature(0) << " K"
                  << " | Erozja: " << solver.getErodedCount(0) << " / " << totalPlateTets << "\n";
        
        exportSceneToOBJ(solver.getBodies(), frame);
    }

    return 0;
}