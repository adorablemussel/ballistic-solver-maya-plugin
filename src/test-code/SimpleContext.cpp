#include "SimpleContext.h"

#include <maya/MGlobal.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MString TITLE = "Simple Context";

static const MString HELP_TEXT = "<insert help text here>";

////////////////////
// PUBLIC METHODS //
////////////////////

SimpleContext::SimpleContext()
{
	setTitleString(TITLE);
	setImage("C:/Users/Szymon/Desktop/PRACA_INZYNIERSKA/BallisticSolver/images/icon.png", MPxContext::kImage1);
}

SimpleContext::~SimpleContext()
{

}

MStatus SimpleContext::helpStateHasChanged(MEvent& event)
{
	return (setHelpString(HELP_TEXT));
}

void SimpleContext::toolOnSetup(MEvent& event)
{
	MGlobal::displayInfo("toolOnSetup");
}

void SimpleContext::toolOffCleanup()
{
	MGlobal::displayInfo("toolOffCleanup");
}

MStatus SimpleContext::doPress(MEvent& event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	MEvent::MouseButtonType MouseButton = event.mouseButton();

	if (MouseButton == MEvent::kLeftMouse) {
		MGlobal::displayInfo("Left mouse button pressed");
	}
	else if (MouseButton == MEvent::kMiddleMouse) {
		MGlobal::displayInfo("Middle mouse button pressed");
	}

	return(MS::kSuccess);
}

MStatus SimpleContext::doRelease(MEvent& event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	MGlobal::displayInfo("Mouse button released");

	return(MS::kSuccess);
}

MStatus SimpleContext::doDrag(MEvent& event, MHWRender::MUIDrawManager& drawMgr, const MHWRender::MFrameContext& context)
{
	MGlobal::displayInfo("Mouse drag");

	return(MS::kSuccess);
}

void SimpleContext::completeAction()
{
	MGlobal::displayInfo("Complete action (enter/return key pressed)");
}

void SimpleContext::deleteAction()
{
	MGlobal::displayInfo("Delete action (backspace/delete key pressed)");
}

void SimpleContext::abortAction()
{
	MGlobal::displayInfo("Abort action (escape key pressed)");
}

