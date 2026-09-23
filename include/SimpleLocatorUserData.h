#pragma once

#include <maya/MColor.h>
#include <maya/MUserData.h>

class SimpleLocatorUserData : public MUserData {
public:
	int shapeIndex;
	MColor wireframeColor;

	SimpleLocatorUserData(bool deleteAfterUse);
	virtual ~SimpleLocatorUserData() override;


private:

};