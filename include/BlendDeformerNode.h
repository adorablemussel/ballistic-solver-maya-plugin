#pragma once

#include <maya/MPxDeformerNode.h>

// do testowania MEL:
// select sourceSphere; deformer -type blenddeformernode;
// connectAttr -force deformerTargetShape.outMesh blenddeformernode1.blendMesh;
class BlendDeformerNode : public MPxDeformerNode {
public:
	BlendDeformerNode();
	virtual ~BlendDeformerNode() override;

	MStatus deform(MDataBlock& block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex) override;


// static methods:
	static MStatus Initialize();
	static void* Creator();

	static MString GetTypeName();
	static MTypeId GetTypeId();

private:
	static MObject blendMeshObj;
	static MObject blendWeightObj;
};