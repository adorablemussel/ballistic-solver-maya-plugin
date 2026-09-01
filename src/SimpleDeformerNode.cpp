#include "SimpleDeformerNode.h"

#include "maya/MItGeometry.h"
#include "maya/MPoint.h"

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7FA);
static const MString TYPE_NAME = MString("simpledeformernode");

//////////////////////
// STATIC VARIABLES //
//////////////////////

////////////////////
// PUBLIC METHODS //
////////////////////

SimpleDeformerNode::SimpleDeformerNode() : MPxDeformerNode()
{

}

SimpleDeformerNode::~SimpleDeformerNode()
{

}

MStatus SimpleDeformerNode::deform(MDataBlock& dataBlock, MItGeometry& geoIter, const MMatrix& matrix, unsigned int multiIndex)
{
	float envelopeValue = dataBlock.inputValue(envelope).asFloat();

	if (envelopeValue != 0) {
		geoIter.reset(); 

		while (!geoIter.isDone()) {
			if (geoIter.index() % 2 == 0) {
				MPoint pt = geoIter.position();
				pt.y += 2.0f * envelopeValue;
				geoIter.setPosition(pt);
			}
			geoIter.next();
		}
	} 

	return (MS::kSuccess);
}



////////////////////
// STATIC METHODS //
////////////////////
void* SimpleDeformerNode::Creator()
{
	return (new SimpleDeformerNode);
}

MStatus SimpleDeformerNode::Initialize()
{
	return (MS::kSuccess);
}

MTypeId SimpleDeformerNode::GetTypeID()
{
	return (TYPE_ID);
}

MString SimpleDeformerNode::GetTypeName()
{
	return (TYPE_NAME);
}
