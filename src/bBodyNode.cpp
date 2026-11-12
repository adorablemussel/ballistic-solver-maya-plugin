#include "bBodyNode.h"
#include "bBodyData.h"
#include "bMaterialData.h"
#include "bMaterial.h"
#include "bMeshData.h"

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPluginData.h>
#include <maya/MFloatVector.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MIntArray.h>

#include <array>
#include <algorithm>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F804);
static const MString TYPE_NAME = MString("bBodyNode");

////////////////////
// STATIC OBJECTS //
////////////////////
MObject bBodyNode::inTetMeshObj;
MObject bBodyNode::inMaterialObj;
MObject bBodyNode::velocityObj;
MObject bBodyNode::pinnedVerticesObj;

MObject bBodyNode::outBodyObj;

////////////////////
// PUBLIC METHODS //
////////////////////
bBodyNode::bBodyNode() : MPxNode()
{
}

bBodyNode::~bBodyNode()
{
}

MStatus bBodyNode::compute(const MPlug& plug, MDataBlock& data)
{
    if (plug == outBodyObj) {
        // odczyt danych z customowych node'ów
        bMeshData* inTetMeshData = dynamic_cast<bMeshData*>(data.inputValue(inTetMeshObj).asPluginData());
        bMaterialData* inMaterialData = dynamic_cast<bMaterialData*>(data.inputValue(inMaterialObj).asPluginData());
        if (!inTetMeshData || !inMaterialData) {
            return (MS::kFailure);
        }

        MFnPluginData pluginDataFn;
        MObject newBodyObject = pluginDataFn.create(bBodyData::GetTypeId());

        MPxData* rawData = pluginDataFn.data();
        bBodyData* outData = dynamic_cast<bBodyData*>(rawData);
        if (!outData) {
            return (MS::kFailure);
        }

        // odczyt z velocity
        const float3& velocityData = data.inputValue(velocityObj).asFloat3();

        // zapis
        outData->vertices = inTetMeshData->vertices;
        outData->tetrahedrons = inTetMeshData->tetrahedrons;
        outData->material = inMaterialData->material;
        outData->velocity = std::array<float, 3>{ velocityData[0], velocityData[1], velocityData[2] };
        
        outData->pinnedVertices.clear();
        MObject pinArrayObj = data.inputValue(pinnedVerticesObj).data();
        if (!pinArrayObj.isNull()) {
            MFnIntArrayData intArrayFn(pinArrayObj);
            MIntArray mayaIndices = intArrayFn.array();
            outData->pinnedVertices.resize(mayaIndices.length());
            for (unsigned int i = 0; i < mayaIndices.length(); ++i) {
                outData->pinnedVertices[i] = mayaIndices[i];
            }
        }
    
        MDataHandle bodyDataHandle = data.outputValue(outBodyObj);
        bodyDataHandle.set(newBodyObject);

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
void* bBodyNode::Creator() {
    return (new bBodyNode());
}

MStatus bBodyNode::Initialize() {
    MFnTypedAttribute typedAttr;
    inTetMeshObj = typedAttr.create("inTetMesh", "intm", bMeshData::GetTypeId());
    typedAttr.setStorable(false);
    typedAttr.setKeyable(false);

    inMaterialObj = typedAttr.create("inMaterial", "inmat", bMaterialData::GetTypeId());
    typedAttr.setStorable(false);
    typedAttr.setKeyable(false);

    MFnNumericAttribute numericAttr;
    velocityObj = numericAttr.create("velocity", "vel", MFnNumericData::k3Float);
    numericAttr.setKeyable(true);
    numericAttr.setWritable(true);

    pinnedVerticesObj = typedAttr.create("pinnedVertices", "pinv", MFnData::kIntArray);
    typedAttr.setStorable(true);
    typedAttr.setKeyable(false);
    typedAttr.setWritable(true);

    outBodyObj = typedAttr.create("outBody", "ob", bBodyData::GetTypeId());
    typedAttr.setStorable(false);
    typedAttr.setWritable(false);

    addAttribute(inTetMeshObj);
    addAttribute(inMaterialObj);
    addAttribute(velocityObj);
    addAttribute(pinnedVerticesObj);
    addAttribute(outBodyObj);

    attributeAffects(inTetMeshObj, outBodyObj);
    attributeAffects(inMaterialObj, outBodyObj);
    attributeAffects(velocityObj, outBodyObj);
    attributeAffects(pinnedVerticesObj, outBodyObj);
    

    return (MS::kSuccess);
}

MTypeId bBodyNode::GetTypeId()
{
    return (TYPE_ID);
}

MString bBodyNode::GetTypeName()
{
    return (TYPE_NAME);
}


