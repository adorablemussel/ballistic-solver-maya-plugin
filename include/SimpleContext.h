#pragma once

#include <maya/MPxContext.h>

class SimpleContext : public MPxContext {
public:
	SimpleContext();
	virtual ~SimpleContext() override;

	virtual MStatus helpStateHasChanged(MEvent& event) override;

	virtual void toolOnSetup(MEvent& event) override;
	virtual void toolOffCleanup() override;

	virtual MStatus doPress(MEvent& event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context) override;
	virtual MStatus doRelease(MEvent& event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context) override;
	virtual MStatus doDrag(MEvent& event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context) override;

	virtual void completeAction() override;
	virtual void deleteAction() override;
	virtual void abortAction() override;
};