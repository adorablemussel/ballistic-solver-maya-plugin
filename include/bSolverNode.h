#pragma once

#include <maya/MPxNode.h>

class bSolverNode : public MPxNode {
public:
	bSolverNode();
	virtual ~bSolverNode() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

// static methods:
	static void* Creator();
	static MStatus Initialize();

	static MTypeId GetTypeId();
	static MString GetTypeName();

private:
	static MObject inBodyObj;
	static MObject inTimeObj;

	static MObject outTetMeshObj;
	static MObject outMeshObj;
};