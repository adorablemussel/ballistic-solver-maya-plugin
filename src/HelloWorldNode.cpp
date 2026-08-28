#include "HelloWorldNode.h"

#include <maya/MGlobal.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7F7); // numer, który u¿ywa Maya do identyfikacji locator node'a
static const MString TYPE_NAME = "helloworld";

static const MString DRAW_DB_CLASSIFICATION = "drawdb/geometry/helloworld";
static const MString DRAW_REGISTRATION_ID = "HelloWorldNode";

////////////////////
// PUBLIC METHODS //
////////////////////
HelloWorldNode::HelloWorldNode() : MPxLocatorNode(){

}

HelloWorldNode::~HelloWorldNode(){

}

////////////////////
// STATIC METHODS //
////////////////////
void* HelloWorldNode::Creator() {
	return (new HelloWorldNode());
}

MStatus HelloWorldNode::Initialize() {
	return (MS::kSuccess);
}

MTypeId HelloWorldNode::GetTypeId() {
	return (TYPE_ID);
}

MString HelloWorldNode::GetTypeName() {
	return (TYPE_NAME);
}

MString HelloWorldNode::GetDrawDbClassification() {
	return (DRAW_DB_CLASSIFICATION);
}

MString HelloWorldNode::GetDrawingRegistrationId() {
	return (DRAW_REGISTRATION_ID);
}