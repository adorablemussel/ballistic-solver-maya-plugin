#include "HelloWorldDrawOverride.h"

////////////////////
// PUBLIC METHODS //
////////////////////
HelloWorldDrawOverride::~HelloWorldDrawOverride() {

}

MHWRender::DrawAPI HelloWorldDrawOverride::supportedDrawAPIs() const
{
	return (MHWRender::kAllDevices);
}

bool HelloWorldDrawOverride::hasUIDrawables() const
{
	return (true);
}

void HelloWorldDrawOverride::addUIDrawables(const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data)
{
	drawManager.beginDrawable();

	drawManager.text2d(MPoint(100, 100), "Hello World");

	drawManager.endDrawable();
}

MUserData* HelloWorldDrawOverride::prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath, const MHWRender::MFrameContext& frameContext, MUserData* oldData)
{
	return (nullptr);
}

////////////////////
// STATIC METHODS //
////////////////////
MHWRender::MPxDrawOverride* HelloWorldDrawOverride::Creator(const MObject& obj) {
	return (new HelloWorldDrawOverride(obj));
}

/////////////////////
// PRIVATE METHODS //
/////////////////////
HelloWorldDrawOverride::HelloWorldDrawOverride(const MObject& obj) : 
	MHWRender::MPxDrawOverride(obj, nullptr) 
{

}