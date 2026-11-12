#include "bMesherNode.h"
#include "bMeshData.h"

#include <maya/MPxNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MGlobal.h>

#include <gmsh.h>

#include <mutex>
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
		MObject inputMesh = data.inputValue(inMeshObj).asMeshTransformed(); // od razu na wspó³rzêdnych oryginalnej siatki
		if (inputMesh.isNull()) {
			inputMesh = data.inputValue(inMeshObj).asMesh();
		}

		MFnPluginData pluginDataFn;
		MObject newOutTetMeshObj = pluginDataFn.create(bMeshData::GetTypeId());

		bMeshData* outData = dynamic_cast<bMeshData*>(pluginDataFn.data());

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
		
			static std::mutex g_gmshMutex;
			std::lock_guard<std::mutex> lock(g_gmshMutex);
			try {
				gmsh::initialize();
				gmsh::option::setNumber("General.Terminal", 0);
				gmsh::model::add("BallisticMesher");

				gmsh::option::setNumber("Mesh.MeshSizeMax", elementSize);
				gmsh::option::setNumber("Mesh.MeshSizeMin", elementSize);

				// rejestracja powierzchni obiektu
				int surfaceTag = 1;
				gmsh::model::addDiscreteEntity(2, surfaceTag);

				std::size_t numSurfaceVerts = mayaPoints.length();
				std::vector<std::size_t> nodeTags(numSurfaceVerts);
				std::vector<double> nodeCoords(numSurfaceVerts * 3);

				for (std::size_t i = 0; i < numSurfaceVerts; i++) {
					nodeTags[i] = i + 1;
					nodeCoords[i * 3 + 0] = mayaPoints[i].x;
					nodeCoords[i * 3 + 1] = mayaPoints[i].y;
					nodeCoords[i * 3 + 2] = mayaPoints[i].z;
				}

				gmsh::model::mesh::addNodes(2, surfaceTag, nodeTags, nodeCoords);

				std::vector<int> elementTypes = { 2 };
				std::vector<std::vector<std::size_t>> elementTags(1);
				std::vector<std::vector<std::size_t>> elementNodeTags(1);

				std::size_t numTriangles = triangleVertices.length() / 3;
				elementTags[0].resize(numTriangles);
				elementNodeTags[0].resize(triangleVertices.length());

				for (std::size_t i = 0; i < numTriangles; i++) {
					elementTags[0][i] = i + 1;
					elementNodeTags[0][i * 3 + 0] = triangleVertices[i * 3 + 0] + 1;
					elementNodeTags[0][i * 3 + 1] = triangleVertices[i * 3 + 1] + 1;
					elementNodeTags[0][i * 3 + 2] = triangleVertices[i * 3 + 2] + 1;
				}

				gmsh::model::mesh::addElements(2, surfaceTag, elementTypes, elementTags, elementNodeTags);

				int volumeTag = 1;
				gmsh::model::geo::addSurfaceLoop(std::vector<int>{surfaceTag}, volumeTag);
				gmsh::model::geo::addVolume(std::vector<int>{volumeTag}, volumeTag);
				gmsh::model::geo::synchronize();

				gmsh::model::mesh::generate(3);

				std::vector<std::size_t> outNodeTags;
				std::vector<double> outCoords;
				std::vector<double> parametricCoords;

				gmsh::model::mesh::getNodes(outNodeTags, outCoords, parametricCoords, -1, -1);

				std::size_t internalCount = 0;
				for (std::size_t tag : outNodeTags) {
					if (tag < 1 || tag > numSurfaceVerts) {
						internalCount++;
					}
				}

				outData->vertices.resize((numSurfaceVerts + internalCount) * 3, 0.0f);
				std::unordered_map<std::size_t, int> tagToIndex;
				tagToIndex.reserve(outNodeTags.size());

				int nextInternalIndex = static_cast<int>(numSurfaceVerts);
				for (size_t i = 0; i < outNodeTags.size(); i++) {
					std::size_t tag = outNodeTags[i];
					int targetIdx = (tag >= 1 && tag <= numSurfaceVerts) ? static_cast<int>(tag - 1) : nextInternalIndex++;

					tagToIndex[tag] = targetIdx;
					outData->vertices[targetIdx * 3 + 0] = static_cast<float>(outCoords[i * 3 + 0]);
					outData->vertices[targetIdx * 3 + 1] = static_cast<float>(outCoords[i * 3 + 1]);
					outData->vertices[targetIdx * 3 + 2] = static_cast<float>(outCoords[i * 3 + 2]);
				}

				std::vector<std::size_t> tetTags;
				std::vector<std::size_t> tetNodeTags;
				gmsh::model::mesh::getElementsByType(4, tetTags, tetNodeTags, volumeTag);

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

MPxNode::SchedulingType bMesherNode::schedulingType() const
{
	return MPxNode::kGloballySerialize;
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