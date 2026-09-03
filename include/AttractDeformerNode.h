#pragma once

#include <maya/MPxDeformerNode.h>

// do testowania MEL:
// select sourceSphere; deformer -type attractdeformernode;
// connectAttr -force targetLocator.translate attractdeformernode1.targetPosition
class AttractDeformerNode : public MPxDeformerNode {
public:
	AttractDeformerNode();
	virtual ~AttractDeformerNode() override;

	MStatus deform(MDataBlock& block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex) override;

// static methods:
	static void* Creator();
	static MStatus Initialize();

	static MTypeId GetTypeId();
	static MString GetTypeName();

private:
	static MObject maxDistanceObj;
	static MObject targetPositionObj;
};