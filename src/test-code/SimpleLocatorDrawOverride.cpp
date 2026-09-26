#include "SimpleLocatorDrawOverride.h"

#include <maya/MFnDependencyNode.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MPlug.h>
#include <maya/MPointArray.h>

#include "SimpleLocatorUserData.h"

////////////////////
// PUBLIC METHODS //
////////////////////
SimpleLocatorDrawOverride::~SimpleLocatorDrawOverride() {

}

MHWRender::DrawAPI SimpleLocatorDrawOverride::supportedDrawAPIs() const {
	return (MHWRender::kAllDevices);
}

bool SimpleLocatorDrawOverride::hasUIDrawables() const {
	return(true);
}

MUserData* SimpleLocatorDrawOverride::prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath, const MHWRender::MFrameContext& frameContext, MUserData* oldData)
{
	SimpleLocatorUserData* data = dynamic_cast<SimpleLocatorUserData*>(oldData);
	if (!data) {
		data = new SimpleLocatorUserData(false);
	}

	MObject locatorObj = objPath.node();
	MFnDependencyNode nodeFn(locatorObj);

	data->shapeIndex = nodeFn.findPlug("shapeIndex", false).asInt();

	MHWRender::DisplayStatus displayStatus = MGeometryUtilities::displayStatus(objPath);
	if (displayStatus == MHWRender::kDormant) {
		data->wireframeColor = MColor(0.0f, 0.1f, 0.0f);
	}
	else {
		data->wireframeColor = MGeometryUtilities::wireframeColor(objPath);
	}

	return (data);
}

void SimpleLocatorDrawOverride::addUIDrawables(const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data)
{
	const SimpleLocatorUserData* userData = dynamic_cast<const SimpleLocatorUserData*>(data);
	if (!userData) {
		return;
	}

	drawManager.beginDrawable();
	drawManager.setColor(userData->wireframeColor);

	MPointArray pointArray;
	switch (userData->shapeIndex) {
	case 0:
		drawManager.circle(MPoint(0.0, 0.0, 0.0), MVector(0.0, 1.0, 0.0), 1.0, false);
		break;
	case 1:
		drawManager.rect(MPoint(0.0, 0.0, 0.0), MVector(0.0, 0.0, 1.0), MVector(0.0, 1.0, 0.0), 1.0, 1.0, false);
		break;
	case 2:
		pointArray.append(MPoint(-1.0, 0.0, -1.0));
		pointArray.append(MPoint(0.0, 0.0, 1.0));
		pointArray.append(MPoint(1.0, 0.0, -1.0));
		pointArray.append(MPoint(-1.0, 0.0, -1.0));

		drawManager.lineStrip(pointArray, false);
		break;
	default:
		break;
	}

	drawManager.endDrawable();
}

////////////////////
// STATIC METHODS //
////////////////////
MHWRender::MPxDrawOverride* SimpleLocatorDrawOverride::Creator(const MObject& obj) {
	return (new SimpleLocatorDrawOverride(obj));
}

/////////////////////
// PRIVATE METHODS //
/////////////////////
SimpleLocatorDrawOverride::SimpleLocatorDrawOverride(const MObject& obj) : MHWRender::MPxDrawOverride(obj, nullptr, true) {

}