#pragma once

#include <maya/MPxDrawOverride.h>

class HelloWorldDrawOverride : public MHWRender::MPxDrawOverride {
public:
	virtual ~HelloWorldDrawOverride() override;

	virtual MHWRender::DrawAPI supportedDrawAPIs() const override;

	virtual bool hasUIDrawables() const override;
	virtual void addUIDrawables(
		const MDagPath& objPath, 
		MHWRender::MUIDrawManager& drawManager, 
		const MHWRender::MFrameContext& frameContext, 
		const MUserData* data) 
		override;

	virtual MUserData* prepareForDraw(
		const MDagPath& objPath, 
		const MDagPath& cameraPath, 
		const MHWRender::MFrameContext& frameContext, 
		MUserData* oldData) 
		override;

// static methods:
	static MHWRender::MPxDrawOverride* Creator(const MObject& obj);

private:
	HelloWorldDrawOverride(const MObject& obj);
};