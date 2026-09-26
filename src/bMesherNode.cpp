#include "bMesherNode.h"
#include "bMeshData.h"

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MGlobal.h>

#include <gmsh.h>

#include <vector>
#include <unordered_map>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F801);
static const MString TYPE_NAME = MString("bMesherNode");

//////////////////////
// STATIC VARIABLES //
//////////////////////
MObject bMesherNode::inMeshObj;
MObject bMesherNode::elementSizeObj;

MObject bMesherNode::outTetMeshObj;

////////////////////
// PUBLIC METHODS //
////////////////////
bMesherNode::bMesherNode() : MPxNode()
{
}

bMesherNode::~bMesherNode()
{
}

MStatus bMesherNode::compute(const MPlug& plug, MDataBlock& data)
{
	if (plug == outTetMeshObj) 
	{
		float elementSize = data.inputValue(elementSizeObj).asFloat();
		MObject inputMesh = data.inputValue(inMeshObj).asMesh();
		
		MFnPluginData pluginDataFn;
		MObject newOutTetMeshObj = pluginDataFn.create(bMeshData::GetTypeId());

		MPxData* rawData = pluginDataFn.data();
		bMeshData* outData = dynamic_cast<bMeshData*>(rawData);

		if (!outData) {
			return (MS::kFailure);
		}

		if (!inputMesh.isNull()) {
			MFnMesh meshFn(inputMesh); // odczyt geometrii

			MFloatPointArray mayaPoints;
			meshFn.getPoints(mayaPoints, MSpace::kObject); // pobranie wierzcho³ków w lokalnym uk³adzie wsp.

			MIntArray triangleCounts; // na ile trójk¹tów zosta³a podzielona dana œciana (tbh bezu¿yteczne!)
			MIntArray triangleVertices; // z których wierzcho³ków jest tworzony dany trójk¹t
			meshFn.getTriangles(triangleCounts, triangleVertices); // pobranie trójk¹tów (powierzchni obiektu)
		
			try {
				gmsh::initialize();
				gmsh::model::add("BallisticMesher");

				gmsh::option::setNumber("Mesh.MeshSizeMax", elementSize);
				gmsh::option::setNumber("Mesh.MeshSizeMin", elementSize);

				// rejestracja powierzchni obiektu
				int surfaceTag = 1;
				gmsh::model::addDiscreteEntity(2, surfaceTag); // 2 = wymiar 2D

				// t³umaczenie punktów z mayi do gmsh
				std::vector<std::size_t> nodeTags(mayaPoints.length());
				std::vector<double> nodeCoords(mayaPoints.length() * 3);

				for (std::size_t i = 0; i < mayaPoints.length(); i++) {
					nodeTags[i] = i + 1; // GMSH TU INDEKSUJE OD 1!!
					nodeCoords[i * 3 + 0] = mayaPoints[i].x;
					nodeCoords[i * 3 + 1] = mayaPoints[i].y;
					nodeCoords[i * 3 + 2] = mayaPoints[i].z;
				}

				gmsh::model::mesh::addNodes(2, surfaceTag, nodeTags, nodeCoords);
				
				// t³uamczenie trójk¹tów z mayi do gmsh
				std::vector<int> elementTypes = { 2 }; // 2 = trójk¹t z 3 wierzcho³kami
				std::vector<std::vector<std::size_t>> elementTags(1);
				std::vector<std::vector<std::size_t>> elementNodeTags(1);

				std::size_t numTriangles = triangleVertices.length() / 3;
				
				elementTags[0].resize(numTriangles);
				elementNodeTags[0].resize(triangleVertices.length());

				for (std::size_t i = 0; i < numTriangles; i++) {
					elementTags[0][i] = i + 1; // TUTAJ TE¯ GMSH INDEKSUJE OD 1!!
					
					elementNodeTags[0][i * 3 + 0] = triangleVertices[i * 3 + 0] + 1;
					elementNodeTags[0][i * 3 + 1] = triangleVertices[i * 3 + 1] + 1;
					elementNodeTags[0][i * 3 + 2] = triangleVertices[i * 3 + 2] + 1;
				}

				gmsh::model::mesh::addElements(2, surfaceTag, elementTypes, elementTags, elementNodeTags); // 2 = 3-wêz³owy trójk¹t

				// wype³nienie wnêtrza
				int volumeTag = 1;
				gmsh::model::geo::addSurfaceLoop(std::vector<int>{surfaceTag}, volumeTag);
				gmsh::model::geo::addVolume(std::vector<int>{volumeTag}, volumeTag);
				gmsh::model::geo::synchronize();

				gmsh::model::mesh::generate(3); // 3 = 3D

				// wyci¹gniêcie danych z powrotem do mayi
				std::vector<std::size_t> outNodeTags;
				std::vector<double> outCoords;
				std::vector<double> parametricCoords;

				gmsh::model::mesh::getNodes(outNodeTags, outCoords, parametricCoords, -1, -1); // -1, -1 = wszystkie wierzcho³ki z ca³ej sceny

				outData->vertices.assign(outCoords.begin(), outCoords.end()); // konwersja double na float wewn¹trz funkcji! (TO DO usuniêcie otrze¿enia)

				// hashmapa: klucz: tag gmsha, wartoœæ: indeks w tablicy
				std::unordered_map<std::size_t, int> tagToIndex;
				for (size_t i = 0; i < outNodeTags.size(); i++) {
					tagToIndex[outNodeTags[i]] = static_cast<int>(i);
				}

				std::vector<std::size_t> tetTags;
				std::vector<std::size_t> tetNodeTags;

				gmsh::model::mesh::getElementsByType(4, tetTags, tetNodeTags, volumeTag); // 4 = czworoœcian

				outData->tetrahedrons.resize(tetNodeTags.size());
				for (size_t i = 0; i < tetNodeTags.size(); i++) { 
					outData->tetrahedrons[i] = tagToIndex[tetNodeTags[i]];
				}


				gmsh::clear();
				gmsh::finalize();

				MGlobal::displayInfo("Gmsh: meshing completed.");
			}
			catch (std::exception& e) {
				MGlobal::displayError(MString("Gmsh Exception: ") + e.what());
				if (gmsh::isInitialized()) {
					gmsh::clear();
					gmsh::finalize();
				}
			}
		
		}
		MDataHandle outHandle = data.outputValue(outTetMeshObj);
		outHandle.set(newOutTetMeshObj);
		data.setClean(plug);
	}
	else {
		return (MS::kUnknownParameter);
	}

	return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////
void* bMesherNode::Creator() {
	return (new bMesherNode());
}

MStatus bMesherNode::Initialize() {
	MFnTypedAttribute typedAttr;
	inMeshObj = typedAttr.create("inMesh", "inm", MFnData::kMesh);
	typedAttr.setStorable(false);
	typedAttr.setKeyable(false);

	MFnNumericAttribute numericAttr;
	elementSizeObj = numericAttr.create("elementSize", "esize", MFnNumericData::kFloat, 1.0f);
	numericAttr.setKeyable(true);
	numericAttr.setMin(0.001f);

	outTetMeshObj = typedAttr.create("outTetMesh", "otm", bMeshData::GetTypeId());
	typedAttr.setStorable(false);
	typedAttr.setWritable(false);

	addAttribute(inMeshObj);
	addAttribute(elementSizeObj);
	addAttribute(outTetMeshObj);

	attributeAffects(inMeshObj, outTetMeshObj);
	attributeAffects(elementSizeObj, outTetMeshObj);

	return (MS::kSuccess);
}

MTypeId bMesherNode::GetTypeId() {
	return (TYPE_ID);
}

MString bMesherNode::GetTypeName() {
	return (TYPE_NAME);
}