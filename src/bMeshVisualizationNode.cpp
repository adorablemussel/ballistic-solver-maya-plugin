#include "bMeshVisualizationNode.h"
#include "bMeshData.h"
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnPluginData.h>


//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F803);
static const MString TYPE_NAME = MString("bMeshVisualizationNode");

static const MString DRAW_DB_CLASSIFICATION = MString("drawdb/geometry/bMeshVisualizationNode");
static const MString DRAW_REGISTRATION_ID = MString("bMeshVisualizationNode");

//////////////////////
// STATIC VARIABLES //
//////////////////////
MObject bMeshVisualizationNode::inTetMeshObj;

////////////////////
// PUBLIC METHODS //
////////////////////
bMeshVisualizationNode::bMeshVisualizationNode() : MPxLocatorNode() {

}

bMeshVisualizationNode::~bMeshVisualizationNode() {

}


bool bMeshVisualizationNode::isBounded() const {
	return (true);
}

MBoundingBox bMeshVisualizationNode::boundingBox() const
{
	MBoundingBox boundingBox;

	MObject thisNode = thisMObject();
	MPlug meshPlug(thisNode, inTetMeshObj);

	MObject meshDataObj;
	meshPlug.getValue(meshDataObj);

	if (!meshDataObj.isNull()) {
		MFnPluginData pluginDataFn(meshDataObj);
		bMeshData* data = dynamic_cast<bMeshData*>(pluginDataFn.data());
	
		if (data && data->vertices.size() >=3 )
		{
			MPoint expandingPoint;
			for (size_t i = 0; i < data->vertices.size(); i += 3) 
			{
				expandingPoint = MPoint(data->vertices[i], data->vertices[i + 1], data->vertices[i + 2]);
				boundingBox.expand(expandingPoint);
			}
		}
	}

	return (boundingBox);
}

////////////////////
// STATIC METHODS //
////////////////////
MStatus bMeshVisualizationNode::Initialize() {
	MFnTypedAttribute typedAttr;

	inTetMeshObj = typedAttr.create("inTetMesh", "intm", bMeshData::GetTypeId());
	typedAttr.setStorable(false);
	typedAttr.setKeyable(false);

	addAttribute(inTetMeshObj);

	return (MS::kSuccess);
}

void* bMeshVisualizationNode::Creator() {
	return (new bMeshVisualizationNode());
}


MTypeId bMeshVisualizationNode::GetTypeId() {
	return (TYPE_ID);
}

MString bMeshVisualizationNode::GetTypeName() {
	return (TYPE_NAME);
}

MString bMeshVisualizationNode::GetDrawDbClassification() {
	return (DRAW_DB_CLASSIFICATION);
}

MString bMeshVisualizationNode::GetDrawRegistrationId() {
	return (DRAW_REGISTRATION_ID);
}