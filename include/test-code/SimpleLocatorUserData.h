#pragma once

#include <maya/MColor.h>
#include <maya/MUserData.h>

class SimpleLocatorUserData : public MUserData {
public:
	SimpleLocatorUserData(bool deleteAfterUse);
	virtual ~SimpleLocatorUserData() override;

	int shapeIndex;
	MColor wireframeColor;
};