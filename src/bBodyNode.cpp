#include "bBodyNode.h"
#include "bBodyData.h"
#include "bMaterialData.h"
#include "bMaterial.h"
#include "bMeshData.h"

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>

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

MStatus bBodyNode::compute(const MPlug&, MDataBlock&)
{
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

    outBodyObj = typedAttr.create("outBody", "ob", bBodyData::GetTypeId());
    typedAttr.setStorable(false);
    typedAttr.setWritable(false);

    addAttribute(inTetMeshObj);
    addAttribute(inMaterialObj);
    addAttribute(velocityObj);
    addAttribute(outBodyObj);

    attributeAffects(inTetMeshObj, outBodyObj);
    attributeAffects(inMaterialObj, outBodyObj);
    attributeAffects(velocityObj, outBodyObj);
    

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


