#pragma once

#include <maya/MPxNode.h>

class bMesherNode : public MPxNode {
public:
	bMesherNode();
	virtual ~bMesherNode() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

// static methods:
	static MStatus Initialize();
	static void* Creator();

	static MTypeId GetTypeId();
	static MString GetTypeName();

private:
// input objects:
	static MObject inMeshObj;
	static MObject elementSizeObj;

// output objects:
	static MObject outTetMeshObj;
};