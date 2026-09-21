#pragma once

#include <maya/MPxNode.h>

// wêze³ do definiowania materia³u 
class bMaterialNode : public MPxNode {
public:
	bMaterialNode();
	virtual ~bMaterialNode() override;

	virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

// static methods:
	static void* Creator();
	static MStatus Initialize();


	static MTypeId GetTypeId();
	static MString GetTypeName();


private:
// input objects:
	// do grupowania atrybutów w Initialize()
	static MObject elasticityCompoundObj; 
	static MObject plasticityCompoundObj;
	static MObject damageCompoundObj;
	static MObject thermalCompoundObj;
	static MObject eosCompoundObj;

	// sprê¿ystoœæ
	static MObject densityObj;
	static MObject youngModulusObj;
	static MObject poissonRatioObj;

	// plastycznoœæ (Johnson-Cook)
	static MObject AObj;
	static MObject BObj;
	static MObject nObj;
	static MObject CObj;
	static MObject mObj;

	// zniszczenie (Johnson-Cook)
	static MObject D1Obj;
	static MObject D2Obj;
	static MObject D3Obj;
	static MObject D4Obj;
	static MObject D5Obj;

	// termomechanika
	static MObject T_roomObj;
	static MObject T_meltObj;
	static MObject specificHeatObj;
	static MObject eps_dot_0Obj;
	static MObject taylorQuinneyObj;

	// hydrodynamika (equation of state)
	static MObject eos_C0Obj;
	static MObject eos_SObj;
	static MObject eos_Gamma0Obj;

// output attribute:
	static MObject materialObj;
};