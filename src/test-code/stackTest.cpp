#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>

#include <gmsh.h>

#include <vector>
#include <string>
#include <exception>

#include "kernel.cuh"

// =====================================================================
// KLASA KOMENDY MAYA
// =====================================================================
class TestCmd : public MPxCommand {
public:
    TestCmd() {}
    virtual ~TestCmd() {}

    // Metoda wykonuj¹ca g³ówn¹ logikê
    MStatus doIt(const MArgList& args) override;

    // Standardowy kreator wymagany przez Mayê
    static void* creator() {
        return new TestCmd();
    }
};

// =====================================================================
// G£ÓWNA LOGIKA IN¯YNIERSKA
// =====================================================================
MStatus TestCmd::doIt(const MArgList& args) {
    MGlobal::displayInfo("========================================");
    MGlobal::displayInfo("[BallisticSolver] Inicjalizacja Gmsh...");

    try {
        // 1. Budzimy silnik Gmsh
        gmsh::initialize();

        // Wyciszamy domyœlne printy Gmsha, ¿eby nie œmieci³y w konsoli Windowsa
        gmsh::option::setNumber("General.Terminal", 0);

        // 2. Tworzymy nowy wirtualny model
        gmsh::model::add("PancerzTestowy");

        // 3. Budujemy geometriê (P³aski kwadrat 10x10) u¿ywaj¹c natywnego silnika GMSH (geo)
        // Tworzymy 4 punkty (x, y, z, docelowa_gestosc_siatki, tag_punktu)
        gmsh::model::geo::addPoint(0, 0, 0, 1.0, 1);
        gmsh::model::geo::addPoint(10, 0, 0, 1.0, 2);
        gmsh::model::geo::addPoint(10, 10, 0, 1.0, 3);
        gmsh::model::geo::addPoint(0, 10, 0, 1.0, 4);

        // £¹czymy punkty w 4 linie
        gmsh::model::geo::addLine(1, 2, 1);
        gmsh::model::geo::addLine(2, 3, 2);
        gmsh::model::geo::addLine(3, 4, 3);
        gmsh::model::geo::addLine(4, 1, 4);

        // Tworzymy obwód z linii i wype³niamy go powierzchni¹
        gmsh::model::geo::addCurveLoop({ 1, 2, 3, 4 }, 1);
        gmsh::model::geo::addPlaneSurface({ 1 }, 1);

        // Synchronizacja geometrii z silnikiem siatek
        gmsh::model::geo::synchronize();

        // 4. Odpalamy ciê¿k¹ matematykê - generowanie siatki 2D (trójk¹ty)
        MGlobal::displayInfo("[BallisticSolver] Ciêcie geometrii na trójk¹ty...");
        gmsh::model::mesh::generate(2);

        // 5. Pobieramy wyniki pracy silnika bezpoœrednio do wektorów C++
        std::vector<std::size_t> nodeTags;
        std::vector<double> coords;
        std::vector<double> parametricCoords;

        gmsh::model::mesh::getNodes(nodeTags, coords, parametricCoords);

        // Kod CUDA
        MGlobal::displayInfo("[BallisticSolver] Wysylanie danych na karte graficzna...");

        // Zapisujemy pozycjê Y pierwszego wierzcho³ka PRZED modyfikacj¹
        double y_before = coords[1];

        // Odpalamy CUDA!
        runCudaTest(coords);

        // Zapisujemy pozycjê Y pierwszego wierzcho³ka PO modyfikacji
        double y_after = coords[1];

        // Wyœwietlamy dowód w Mayi
        MString cudaMsg = MString("[BallisticSolver] CUDA dziala! Punkt Y zmienil pozycje z ")
            + y_before + " na " + y_after;
        MGlobal::displayInfo(cudaMsg);

        // 6. Raportujemy sukces w interfejsie Mayi
        MString infoMsg = MString("[BallisticSolver] SUKCES! Gmsh wygenerowal ")
            + nodeTags.size()
            + MString(" wierzcholkow siatki.");
        MGlobal::displayInfo(infoMsg);

        // 7. Usypiamy silnik (zwalnianie pamiêci)
        gmsh::finalize();

    }
    catch (const std::exception& e) {
        // Przechwytywanie b³êdów - chroni Mayê przed natychmiastowym crashem (wywaleniem do pulpitu)
        MString errMsg = MString("[BallisticSolver] BLAD GMSH: ") + e.what();
        MGlobal::displayError(errMsg);

        // Próba ratunkowego zamkniêcia silnika
        try { gmsh::finalize(); }
        catch (...) {}

        return MS::kFailure;
    }

    MGlobal::displayInfo("========================================");
    return MS::kSuccess;
}

// =====================================================================
// REJESTRACJA PLUGINU W MAYI
// =====================================================================
MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "BallisticSolver", "1.0", "Any");

    MStatus status = plugin.registerCommand("testCommand", TestCmd::creator);
    if (!status) {
        status.perror("[BallisticSolver] Nie udalo sie zarejestrowac komendy testCommand.");
        return status;
    }
    else {
        MGlobal::displayInfo("[BallisticSolver] Plugin zarejestrowany poprawnie.");
    }

    return status;
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);

    MStatus status = plugin.deregisterCommand("testCommand");
    if (!status) {
        status.perror("[BallisticSolver] Nie udalo sie wyrejestrowac komendy testCommand.");
        return status;
    }
    else {
        MGlobal::displayInfo("[BallisticSolver] Plugin wyrejestrowany poprawnie.");
    }

    return status;
}