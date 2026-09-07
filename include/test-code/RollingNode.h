#pragma once

#include <maya/MPxNode.h>

class RollingNode : public MPxNode {
public:
	RollingNode();
	virtual ~RollingNode() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

// static methods:
	static void* Creator();
	static MStatus Initialize();

	static MTypeId GetTypeId();
	static MString GetTypeName();

private:
	static MObject distanceObj;
	static MObject radiusObj;

	static MObject rotationObj;
};