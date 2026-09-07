#include "BlendDeformerNode.h"

#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MItGeometry.h>
#include <maya/MPointArray.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7FB);
static const MString TYPE_NAME = MString("blenddeformernode");

//////////////////////
// STATIC VARIABLES //
//////////////////////
MObject BlendDeformerNode::blendMeshObj;
MObject BlendDeformerNode::blendWeightObj;

////////////////////
// PUBLIC METHODS //
////////////////////
BlendDeformerNode::BlendDeformerNode() : MPxDeformerNode() {

}

BlendDeformerNode::~BlendDeformerNode()
{

}

MStatus BlendDeformerNode::deform(MDataBlock& block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex)
{
	float envelopeValue = block.inputValue(envelope).asFloat();
	float blendWeightValue = block.inputValue(blendWeightObj).asFloat();
	MObject targetMesh = block.inputValue(blendMeshObj).asMesh();

	if (envelopeValue == 0 || blendWeightValue == 0 || targetMesh.isNull()) {
		return (MS::kSuccess);
	}

	MPointArray targetPoints;

	MFnMesh targetMeshFn(targetMesh);
	targetMeshFn.getPoints(targetPoints);

	float globalWeight = blendWeightValue * envelopeValue;

	iter.reset();
	while (!iter.isDone()) {
		float sourceWeight = weightValue(block, multiIndex, iter.index());

		MPoint sourcePt = iter.position();
		MPoint targetPt = targetPoints[iter.index()];
		MPoint finalPt = sourcePt + ((targetPt - sourcePt) * globalWeight * sourceWeight);

		iter.setPosition(finalPt);

		iter.next();
	}

	return (MS::kSuccess);
}




////////////////////
// STATIC METHODS //
////////////////////
MStatus BlendDeformerNode::Initialize()
{
	MFnTypedAttribute typeAttr;
	blendMeshObj = typeAttr.create("blendMesh", "bMesh", MFnData::kMesh);
	typeAttr.setReadable(true);
	typeAttr.setWritable(true);
	typeAttr.setConnectable(true);
	typeAttr.setHidden(false);

	MFnNumericAttribute numericAttr;
	blendWeightObj = numericAttr.create("blendWeight", "bWeight", MFnNumericData::kFloat, 0.0f);
	numericAttr.setKeyable(true);
	numericAttr.setMin(0.0f);
	numericAttr.setMax(1.0f);

	addAttribute(blendMeshObj);
	addAttribute(blendWeightObj);

	attributeAffects(blendMeshObj, outputGeom);
	attributeAffects(blendWeightObj, outputGeom);

	return (MS::kSuccess);
}

void* BlendDeformerNode::Creator()
{
	return (new BlendDeformerNode());
}

MString BlendDeformerNode::GetTypeName()
{
	return (TYPE_NAME);
}

MTypeId BlendDeformerNode::GetTypeId()
{
	return (TYPE_ID);
}


/////////////////////
// PRIVATE METHODS //
/////////////////////
