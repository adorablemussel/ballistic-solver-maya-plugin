#pragma once

#include <vector>
#include "bMaterial.h"

//bie¿¹cy stan dynamiczny i termomechaniczny cia³a wewn¹trz solvera (nadpisuje siê co klatkê)
struct bBodyState {
    bMaterial material;

    //zmienne stanu wierzcho³ków (aktualizowane w ka¿dym podkroku dt)
                                        // rozmiary:
    std::vector<float> positions;       // vertices * 3
    std::vector<float> velocities;      // vertices * 3
    std::vector<float> forces;          // vertices * 3
    std::vector<float> masses;          // vertices
    std::vector<float> contactRadii;    // vertices
    std::vector<bool> isVertexActive;   // vertices

    //topologia i stan spoczynkowy czworoœcianów (z klatki startowej)
    std::vector<int> tetrahedrons;      // tetrahedrons * 4
    std::vector<float> restVolumes;     // tetrahedrons 
    std::vector<float> restEdgeLengths; // tetrahedrons * 6 

    //zmienne stanu czworoœcianów (Plastycznoœæ, Termika, EOS, Erozja)
    std::vector<bool> isEroded;         // tetrahedrons
    std::vector<float> temperatures;    // tetrahedrons 
    std::vector<float> plasticStrains;  // tetrahedrons
    std::vector<float> internalEnergies;// tetrahedrons
    std::vector<float> damageAccum;     // tetrahedrons

    //przypiête wierzcho³ki
    std::vector<int> pinnedVertices;
};