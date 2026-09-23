#include "SimpleLocatorNode.h"

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MFnNumericAttribute.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F802);
static const MString TYPE_NAME = MString("simplelocator");

static const MString DRAW_DB_CLASSIFICATION = MString("drawdb/geometry/simplelocator");
static const MString DRAW_REGISTRATION_ID = MString("SimpleLocatorNode");

//////////////////////
// STATIC VARIABLES //
//////////////////////
MObject SimpleLocatorNode::shapeIndexObj;

////////////////////
// PUBLIC METHODS //
////////////////////
SimpleLocatorNode::SimpleLocatorNode() : MPxLocatorNode() {

}

SimpleLocatorNode::~SimpleLocatorNode() {

}

////////////////////
// STATIC METHODS //
////////////////////
void* SimpleLocatorNode::Creator() {
	return (new SimpleLocatorNode());
}

MStatus SimpleLocatorNode::Initialize() {

	MFnNumericAttribute numericAttr;
	shapeIndexObj = numericAttr.create("shapeIndex", "sid", MFnNumericData::kInt, 0);
	numericAttr.setMin(0);
	numericAttr.setMax(2);

	addAttribute(shapeIndexObj);

	return (MS::kSuccess);
}

MTypeId SimpleLocatorNode::GetTypeId() {
	return (TYPE_ID);
}

MString SimpleLocatorNode::GetTypeName() {
	return (TYPE_NAME);
}

MString SimpleLocatorNode::GetDrawDbClassification() {
	return (DRAW_DB_CLASSIFICATION);
}

MString SimpleLocatorNode::GetDrawRegistrationId() {
	return (DRAW_REGISTRATION_ID);
}