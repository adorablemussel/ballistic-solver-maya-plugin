#pragma once

#include <maya/MPxNode.h>

class bBodyNode : public MPxNode {
public:
	bBodyNode();
	virtual ~bBodyNode() override;

	virtual MStatus compute(const MPlug&, MDataBlock&) override;
	
// static methods:
	static void* Creator();
	static MStatus Initialize();

	static MTypeId GetTypeId();
	static MString GetTypeName();

private:
// input objects:
	static MObject inTetMeshObj;
	static MObject inMaterialObj;
	
	static MObject velocityObj;

// output objects:
	static MObject outBodyObj;
};