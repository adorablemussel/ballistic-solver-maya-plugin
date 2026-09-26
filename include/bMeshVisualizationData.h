#pragma once

#include <maya/MUserData.h>
#include <maya/MPointArray.h>

class bMeshVisualizationData : public MUserData {
public:
	bMeshVisualizationData(); // nie ma w argumencie deleteAfterUse, bo hardcode false to optymalizacja
	virtual ~bMeshVisualizationData() override;

	MPointArray wireframeLines;
};