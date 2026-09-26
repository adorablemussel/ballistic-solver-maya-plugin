#pragma once

#include <maya/MPxLocatorNode.h>

class bMeshVisualizationNode : public MPxLocatorNode {
public:
	bMeshVisualizationNode();
	virtual ~bMeshVisualizationNode() override;

	virtual bool isBounded() const override; // pod spodem s³u¿y do wywo³ania boundingBox(), tylko jeœli zwraca true
	virtual MBoundingBox boundingBox() const override;

// static methods:
	static void* Creator();
	static MStatus Initialize();

	static MTypeId GetTypeId();
	static MString GetTypeName();

	static MString GetDrawRegistrationId();
	static MString GetDrawDbClassification();

// input objects:
	static MObject inTetMeshObj;
};