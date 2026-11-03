#include "bSolverNode.h"
#include "bBodyData.h"
#include "bMeshData.h"

#include <maya/MGlobal.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MTime.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <cmath>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F805);
static const MString TYPE_NAME = MString("bSolverNode");

////////////////////
// STATIC OBJECTS //
////////////////////
MObject bSolverNode::inBodyObj;
MObject bSolverNode::inTimeObj;
MObject bSolverNode::substepsObj; 

MObject bSolverNode::outTetMeshObj;
MObject bSolverNode::outMeshObj;

////////////////////
// PUBLIC METHODS //
////////////////////
bSolverNode::bSolverNode() : MPxNode()
{
}

bSolverNode::~bSolverNode()
{
}

MStatus bSolverNode::compute(const MPlug& plug, MDataBlock& data) {
    if (plug != outTetMeshObj && plug != outMeshObj) {
        return MS::kUnknownParameter;
    }

    MStatus status;
    MTime currentTime = data.inputValue(inTimeObj, &status).asTime();
    int substeps = data.inputValue(substepsObj, &status).asInt();

    double currentFrame = currentTime.as(MTime::uiUnit());
    double currentSeconds = currentTime.as(MTime::kSeconds);

    // inicjalizacja (klatka <= 1 lub cofniêcie suwaka czasu na osi timeline)
    if (!isInitialized || currentFrame <= 1.0 || currentSeconds < lastTimeSeconds) {
        solver.clearBodies();

        MArrayDataHandle inBodyArray = data.inputArrayValue(inBodyObj, &status);
        unsigned int bodyCount = inBodyArray.elementCount();

        for (unsigned int i = 0; i < bodyCount; ++i) {
            MDataHandle elemHandle = inBodyArray.inputValue(&status);
            bBodyData* bodyData = static_cast<bBodyData*>(elemHandle.asPluginData());

            if (bodyData) {
                solver.addBody(bodyData->material,
                    bodyData->vertices,
                    bodyData->tetrahedrons,
                    bodyData->velocity);
            }
            inBodyArray.next();
        }

        lastTimeSeconds = currentSeconds;
        isInitialized = true;
        MGlobal::displayInfo(MString("bSolverNode: Initialized bodies: ") + static_cast<int>(solver.getBodies().size()));
    }
    // krok symulacji
    else if (currentSeconds > lastTimeSeconds) {
        float frameDt = static_cast<float>(currentSeconds - lastTimeSeconds);
        // zabezpieczenie przeskokiem typu na klatkê 100 z klatki 1
        if (frameDt > 0.1f) frameDt = 1.0f / 24.0f;

        solver.step(frameDt, substeps);
        lastTimeSeconds = currentSeconds;
    }

    // zapis wyjœæ
    const auto& bodies = solver.getBodies();

    MArrayDataHandle outTetArray = data.outputArrayValue(outTetMeshObj, &status);
    MArrayDataBuilder tetBuilder = outTetArray.builder(&status);

    MArrayDataHandle outMeshArray = data.outputArrayValue(outMeshObj, &status);
    MArrayDataBuilder meshBuilder = outMeshArray.builder(&status);

    MFnPluginData pluginDataFn;

    for (size_t b = 0; b < bodies.size(); ++b) {
        const bBodyState& simBody = bodies[b];
        unsigned int elemIdx = static_cast<unsigned int>(b);

        // budowanie outTetMesh[b] (bMeshData) bez zniszczonych czworoœcianów
        MDataHandle outTetHandle = tetBuilder.addElement(elemIdx, &status);
        MObject newTetDataObj = pluginDataFn.create(bMeshData::GetTypeId(), &status);
        bMeshData* outTetData = static_cast<bMeshData*>(pluginDataFn.data(&status));

        if (outTetData) {
            outTetData->vertices = simBody.positions;
            outTetData->tetrahedrons.clear();

            size_t numTets = simBody.tetrahedrons.size() / 4;
            outTetData->tetrahedrons.reserve(simBody.tetrahedrons.size());

            for (size_t t = 0; t < numTets; ++t) {
                if (!simBody.isEroded[t]) {
                    outTetData->tetrahedrons.push_back(simBody.tetrahedrons[t * 4 + 0]);
                    outTetData->tetrahedrons.push_back(simBody.tetrahedrons[t * 4 + 1]);
                    outTetData->tetrahedrons.push_back(simBody.tetrahedrons[t * 4 + 2]);
                    outTetData->tetrahedrons.push_back(simBody.tetrahedrons[t * 4 + 3]);
                }
            }
            outTetHandle.set(newTetDataObj);
        }

        // budowanie outMesh[b] (Standardowy Maya Mesh z powierzchni zewnêtrznej)
        MDataHandle outSurfaceHandle = meshBuilder.addElement(elemIdx, &status);
        MFnMeshData meshDataFn;
        MObject newMeshObj = meshDataFn.create(&status);

        size_t numVerts = simBody.positions.size() / 3;
        MFloatPointArray mayaPoints(static_cast<unsigned int>(numVerts));
        for (unsigned int v = 0; v < numVerts; ++v) {
            mayaPoints.set(v, simBody.positions[v * 3 + 0],
                simBody.positions[v * 3 + 1],
                simBody.positions[v * 3 + 2]);
        }

        std::vector<int> surfaceTris;
        solver.extractSurfaceTriangles(b, surfaceTris);
        unsigned int numTris = static_cast<unsigned int>(surfaceTris.size() / 3);

        MIntArray polygonCounts(numTris, 3);
        MIntArray polygonConnects(static_cast<unsigned int>(surfaceTris.size()));
        for (unsigned int k = 0; k < surfaceTris.size(); ++k) {
            polygonConnects[k] = surfaceTris[k];
        }

        MFnMesh meshFn;
        meshFn.create(static_cast<int>(numVerts),
            static_cast<int>(numTris),
            mayaPoints,
            polygonCounts,
            polygonConnects,
            newMeshObj,
            &status);

        outSurfaceHandle.set(newMeshObj);
    }

    outTetArray.set(tetBuilder);
    outTetArray.setAllClean();

    outMeshArray.set(meshBuilder);
    outMeshArray.setAllClean();

    data.setClean(outTetMeshObj);
    data.setClean(outMeshObj);
    data.setClean(plug);
    return MS::kSuccess;
}

////////////////////
// STATIC METHODS //
////////////////////
void* bSolverNode::Creator() {
	return (new bSolverNode());
}

MStatus bSolverNode::Initialize() {
	MFnTypedAttribute typedAttr;
	inBodyObj = typedAttr.create("inBody", "inb", bBodyData::GetTypeId());
	typedAttr.setStorable(false);
	typedAttr.setKeyable(false);
	typedAttr.setArray(true);

	MFnUnitAttribute unitAttr;
	inTimeObj = unitAttr.create("inTime", "int", MFnUnitAttribute::kTime, 0.0);
	unitAttr.setStorable(false);
	unitAttr.setKeyable(false);

    MFnNumericAttribute numAttr;
    substepsObj = numAttr.create("substeps", "sub", MFnNumericData::kInt, 30);
    numAttr.setMin(1);
    numAttr.setStorable(true);
    numAttr.setKeyable(true);

	outTetMeshObj = typedAttr.create("outTetMesh", "otm", bMeshData::GetTypeId());
	typedAttr.setStorable(false);
	typedAttr.setWritable(false);
	typedAttr.setArray(true);
    typedAttr.setUsesArrayDataBuilder(true);

	outMeshObj = typedAttr.create("outMesh", "om", MFnData::kMesh);
	typedAttr.setStorable(false);
	typedAttr.setWritable(false);
	typedAttr.setArray(true);
    typedAttr.setUsesArrayDataBuilder(true);

	addAttribute(inBodyObj);
	addAttribute(inTimeObj);
    addAttribute(substepsObj);
	addAttribute(outTetMeshObj);
	addAttribute(outMeshObj);

	attributeAffects(inBodyObj, outTetMeshObj);
	attributeAffects(inBodyObj, outMeshObj);
	attributeAffects(inTimeObj, outTetMeshObj);
	attributeAffects(inTimeObj, outMeshObj);
    attributeAffects(substepsObj, outTetMeshObj);
    attributeAffects(substepsObj, outMeshObj);


	return (MS::kSuccess);
}

MTypeId bSolverNode::GetTypeId() {
	return (TYPE_ID);
}

MString bSolverNode::GetTypeName() {
	return (TYPE_NAME);
}

