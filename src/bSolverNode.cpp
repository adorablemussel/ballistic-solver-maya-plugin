#include "bSolverNode.h"
#include "bBodyData.h"
#include "bMeshData.h"

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>

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

MStatus bSolverNode::compute(const MPlug& plug, MDataBlock& data)
{


	return (MS::kSuccess);
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

	outTetMeshObj = typedAttr.create("outTetMesh", "otm", bMeshData::GetTypeId());
	typedAttr.setStorable(false);
	typedAttr.setWritable(false);
	typedAttr.setArray(true);

	outMeshObj = typedAttr.create("outMesh", "om", MFnData::kMesh);
	typedAttr.setStorable(false);
	typedAttr.setWritable(false);
	typedAttr.setArray(true);

	addAttribute(inBodyObj);
	addAttribute(inTimeObj);
	addAttribute(outTetMeshObj);
	addAttribute(outMeshObj);

	attributeAffects(inBodyObj, outTetMeshObj);
	attributeAffects(inBodyObj, outMeshObj);
	attributeAffects(inTimeObj, outTetMeshObj);
	attributeAffects(inTimeObj, outMeshObj);


	return (MS::kSuccess);
}

MTypeId bSolverNode::GetTypeId() {
	return (TYPE_ID);
}

MString bSolverNode::GetTypeName() {
	return (TYPE_NAME);
}

