#include "MyCallbacks.h"

#include <maya/MGlobal.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MEventMessage.h>
#include <maya/MSceneMessage.h>
#include <maya/MUiMessage.h>
#include <maya/MConditionMessage.h>
#include <maya/MTimerMessage.h>

//////////////////////
// STATIC VARIABLES //
//////////////////////
MCallbackIdArray MyCallbacks::sCallbackIds;

////////////////////
// STATIC METHODS //
////////////////////
void MyCallbacks::OnNewScene(void* clientData) {
    MGlobal::displayInfo("[Callback] New scene opened");
}

void MyCallbacks::OnTimeChanged(void* clientData) {
    MGlobal::displayInfo("[Callback] Time changed");
}

void MyCallbacks::OnSelectionChanged(void* clientData) {
    MGlobal::displayInfo("[Callback] Selection changed");
}

void MyCallbacks::BeforeImport(void* clientData) {
	MGlobal::displayInfo("Import pre-processing");
}

void MyCallbacks::AfterImport(void* clientData) {
	MGlobal::displayInfo("Import post-processing");
}

void MyCallbacks::OnViewportCameraChanged(const MString& panelName, MObject& node, void* clientData) {
    MFnDependencyNode nodeFn(node);

    MGlobal::displayInfo(MString("Camera changed in ") + panelName + " to \"" + nodeFn.name() + "\"");
}

void MyCallbacks::OnPlayingBackStateChanged(bool isPlaying, void* clientData) {
    MGlobal::displayInfo(MString("Playing state changed: ") + static_cast<int>(isPlaying));
}

void MyCallbacks::OnTimerFired(float elapsedTime, float previousExecutionTime, void* clientData) {
    MGlobal::displayInfo(MString("Timer fired - Elapsed Time: ") + elapsedTime);
}

void MyCallbacks::CallbacksAppend() {
	sCallbackIds.append(MEventMessage::addEventCallback("NewSceneOpened", OnNewScene));
	sCallbackIds.append(MEventMessage::addEventCallback("timeChanged", OnTimeChanged));
	sCallbackIds.append(MEventMessage::addEventCallback("SelectionChanged", OnSelectionChanged));
	sCallbackIds.append(MSceneMessage::addCallback(MSceneMessage::kBeforeImport, BeforeImport));
	sCallbackIds.append(MSceneMessage::addCallback(MSceneMessage::kAfterImport, AfterImport));
	sCallbackIds.append(MUiMessage::addCameraChangedCallback("modelPanel4", OnViewportCameraChanged));
	sCallbackIds.append(MConditionMessage::addConditionCallback("playingBack", OnPlayingBackStateChanged));
	sCallbackIds.append(MTimerMessage::addTimerCallback(2.5, OnTimerFired));
}

void MyCallbacks::CallbacksRemove() {
	MMessage::removeCallbacks(sCallbackIds);
	sCallbackIds.clear();
}