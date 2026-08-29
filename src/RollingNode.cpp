#include "RollingNode.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7F9);
static const MString TYPE_NAME = "rollingnode"; 

//////////////////////
// STATIC VARIABLES //
//////////////////////
MObject RollingNode::distanceObj;
MObject RollingNode::radiusObj;
MObject RollingNode::rotationObj;


////////////////////
// PUBLIC METHODS // 
////////////////////
RollingNode::RollingNode() : MPxNode()
{

}

RollingNode::~RollingNode()
{
	
}

MStatus RollingNode::compute(const MPlug& plug, MDataBlock& data)
{
	if (plug == rotationObj) {
		double distance = data.inputValue(distanceObj).asDouble();
		double radius = data.inputValue(radiusObj).asDouble();

		double rotation = 0.0;
		
		if (radius != 0.0) {
			rotation = distance / radius;
		}

		MDataHandle rotationDataHandle = data.outputValue(rotationObj);
		rotationDataHandle.setDouble(rotation);

		data.setClean(plug);
	}
	else {
		return (MS::kFailure);
	}

	return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////

void* RollingNode::Creator()
{
	return (new RollingNode());
}

MStatus RollingNode::Initialize()
{
	MFnNumericAttribute numericAttr;
	distanceObj = numericAttr.create("distance", "dist", MFnNumericData::kDouble, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	radiusObj = numericAttr.create("radius", "rad", MFnNumericData::kDouble, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	MFnUnitAttribute unitAttr;
	rotationObj = unitAttr.create("rotation", "rot", MFnUnitAttribute::kAngle, 0.0);

	addAttribute(distanceObj);
	addAttribute(radiusObj);
	addAttribute(rotationObj);

	attributeAffects(distanceObj, rotationObj);
	attributeAffects(radiusObj, rotationObj);

	return (MS::kSuccess);
}

MTypeId RollingNode::GetTypeId()
{
	return (TYPE_ID);
}

MString RollingNode::GetTypeName()
{
	return (TYPE_NAME);
}

