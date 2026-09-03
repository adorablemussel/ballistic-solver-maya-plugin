#include "AttractDeformerNode.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFnMesh.h>
#include <maya/MItGeometry.h>
#include <maya/MGlobal.h>
#include <maya/MMatrix.h>


//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7FC);
static const MString TYPE_NAME = MString("attractdeformernode");

static const double MAX_ANGLE = 0.5 * 3.14159264; // 90 stopni

//////////////////////
// STATIC VARIABLES //
//////////////////////
MObject AttractDeformerNode::maxDistanceObj;
MObject AttractDeformerNode::targetPositionObj;

////////////////////
// PUBLIC METHODS //
////////////////////
AttractDeformerNode::AttractDeformerNode()
{

}

AttractDeformerNode::~AttractDeformerNode()
{

}

MStatus AttractDeformerNode::deform(MDataBlock& block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex)
{
	float envelopeValue = block.inputValue(envelope).asFloat();
	if (envelopeValue == 0) {
		return (MS::kSuccess);
	}

	float maxDistance = block.inputValue(maxDistanceObj).asFloat();
	if (maxDistance == 0) {
		return (MS::kSuccess);
	}

	MFloatVector targetPosition = block.inputValue(targetPositionObj).asFloatVector();
	targetPosition = MPoint(targetPosition) * mat.inverse(); // zmiana wspó³rzêdnych globalnych na lokalne dla locatora

	MArrayDataHandle inputHandle = block.outputArrayValue(input);
	inputHandle.jumpToElement(multiIndex);
	MDataHandle inputElementHandle = inputHandle.outputValue();

	MObject inputGeomObj = inputElementHandle.child(inputGeom).asMesh();
	MFnMesh meshFn(inputGeomObj);

	MFloatVectorArray normals;
	meshFn.getVertexNormals(false, normals);

	MPoint ptLocal;
	MFloatVector targetVector;
	MFloatVector normal;
	MFloatVector offset;
	float angle = 0.0f;
	float distance = 0.0f;

	iter.reset();
	while (!iter.isDone()) {
		ptLocal = iter.position();
		targetVector = targetPosition - MFloatVector(ptLocal);

		distance = targetVector.length();
		if (distance <= maxDistance) {
			normal = normals[iter.index()];

			angle = normal.angle(targetVector);
			if (angle <= MAX_ANGLE) {
				offset = targetVector * ((maxDistance - distance) / maxDistance);

				iter.setPosition(ptLocal + MVector(offset));
			}
		}

		iter.next();
	}



	return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////
void* AttractDeformerNode::Creator()
{
	return (new AttractDeformerNode());
}

MStatus AttractDeformerNode::Initialize()
{
	MFnNumericAttribute numericAttr;
	maxDistanceObj = numericAttr.create("maximumDistance", "maxDist", MFnNumericData::kFloat, 1.0f);
	numericAttr.setKeyable(true);
	numericAttr.setMin(0.0f);
	numericAttr.setMax(2.0f);

	targetPositionObj = numericAttr.createPoint("targetPosition", "targetPos");
	numericAttr.setKeyable(true);

	addAttribute(maxDistanceObj);
	addAttribute(targetPositionObj);

	attributeAffects(maxDistanceObj, outputGeom);
	attributeAffects(targetPositionObj, outputGeom);

	return (MS::kSuccess);
}

MTypeId AttractDeformerNode::GetTypeId()
{
	return (TYPE_ID);
}

MString AttractDeformerNode::GetTypeName()
{
	return (TYPE_NAME);
}


