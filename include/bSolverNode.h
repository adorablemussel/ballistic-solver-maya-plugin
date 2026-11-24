#pragma once

#include <maya/MPxNode.h>
#include <map>
#include "bSolverEngine.h"

class bSolverNode : public MPxNode {
public:
	bSolverNode();
	virtual ~bSolverNode() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;
	virtual SchedulingType schedulingType() const override;
	virtual MStatus setDependentsDirty(const MPlug& plug, MPlugArray& plugArray) override;

// static methods:
	static void* Creator();
	static MStatus Initialize();

	static MTypeId GetTypeId();
	static MString GetTypeName();

private:
	static MObject inBodyObj;
	static MObject inTimeObj;
	static MObject substepsObj;
	static MObject dampingObj;
	static MObject timeScaleObj;
	static MObject computeBackendObj;

	static MObject outTetMeshObj;
	static MObject outMeshObj;

	// instancja silnika
	bSolverEngine solver;
	int lastSimulatedFrame = -1;
	bool isInitialized = false;

	// cache klatek
	std::map<int, std::vector<bBodyState>> frameCache;
};