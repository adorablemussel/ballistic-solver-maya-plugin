#pragma once

#include <maya/MCallbackIdArray.h>

class MyCallbacks {
public:
	static MCallbackIdArray sCallbackIds;

	static void OnNewScene(void* clientData);
	static void OnTimeChanged(void* clientData);
	static void OnSelectionChanged(void* clientData);
	static void BeforeImport(void* clientData);
	static void AfterImport(void* clientData);
	static void OnViewportCameraChanged(const MString& panelName, MObject& node, void* clientData);
	static void OnPlayingBackStateChanged(bool isPlaying, void* clientData);
	static void OnTimerFired(float elapsedTime, float previousExecutionTime, void* clientData);

	static void CallbacksAppend();
	static void CallbacksRemove();

};