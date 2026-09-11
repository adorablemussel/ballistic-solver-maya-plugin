#include "BallisticMaterialNode.h"
#include "MaterialData.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7FE);
static const MString TYPE_NAME = MString("ballisticmaterialnode");


//////////////////////
// STATIC VARIABLES //
//////////////////////
MObject BallisticMaterialNode::elasticityCompoundObj;
MObject BallisticMaterialNode::plasticityCompoundObj;
MObject BallisticMaterialNode::damageCompoundObj;
MObject BallisticMaterialNode::thermalCompoundObj;
MObject BallisticMaterialNode::eosCompoundObj;

MObject BallisticMaterialNode::densityObj;
MObject BallisticMaterialNode::youngModulusObj;
MObject BallisticMaterialNode::poissonRatioObj;
MObject BallisticMaterialNode::AObj;
MObject BallisticMaterialNode::BObj;
MObject BallisticMaterialNode::nObj;
MObject BallisticMaterialNode::CObj;
MObject BallisticMaterialNode::mObj;
MObject BallisticMaterialNode::D1Obj;
MObject BallisticMaterialNode::D2Obj;
MObject BallisticMaterialNode::D3Obj;
MObject BallisticMaterialNode::D4Obj;
MObject BallisticMaterialNode::D5Obj;
MObject BallisticMaterialNode::T_roomObj;
MObject BallisticMaterialNode::T_meltObj;
MObject BallisticMaterialNode::specificHeatObj;
MObject BallisticMaterialNode::eps_dot_0Obj;
MObject BallisticMaterialNode::taylorQuinneyObj;
MObject BallisticMaterialNode::eos_C0Obj;
MObject BallisticMaterialNode::eos_SObj;
MObject BallisticMaterialNode::eos_Gamma0Obj;

MObject BallisticMaterialNode::materialObj;

////////////////////
// PUBLIC METHODS //
////////////////////
BallisticMaterialNode::BallisticMaterialNode() : MPxNode()
{

}

BallisticMaterialNode::~BallisticMaterialNode()
{

}

MStatus BallisticMaterialNode::compute(const MPlug& plug, MDataBlock& data)
{
	return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////

void* BallisticMaterialNode::Creator()
{
	return (new BallisticMaterialNode());
}

MStatus BallisticMaterialNode::Initialize()
{
	// tworzenie atrybutów
	MFnNumericAttribute numericAttr;
	densityObj = numericAttr.create("density", "dens", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	youngModulusObj = numericAttr.create("YoungModulus", "Young", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	poissonRatioObj = numericAttr.create("PoissonRatio", "Poisson", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	AObj = numericAttr.create("A", "A", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	BObj = numericAttr.create("B", "B", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	nObj = numericAttr.create("n", "n", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	CObj = numericAttr.create("C", "C", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	mObj = numericAttr.create("m", "m", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	D1Obj = numericAttr.create("D1", "D1", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	D2Obj = numericAttr.create("D2", "D2", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	D3Obj = numericAttr.create("D3", "D3", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	D4Obj = numericAttr.create("D4", "D4", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	D5Obj = numericAttr.create("D5", "D5", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	T_roomObj = numericAttr.create("roomTemperature", "T_room", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	T_meltObj = numericAttr.create("meltTemperature", "T_melt", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	specificHeatObj = numericAttr.create("specificHeat", "specHeat", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	eps_dot_0Obj = numericAttr.create("referenceStrainRate", "refStrain", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	taylorQuinneyObj = numericAttr.create("TaylorQuinney", "Quinney", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	eos_C0Obj = numericAttr.create("referenceSoundSpeed", "refSound", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	eos_SObj = numericAttr.create("curveSlope", "slope", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	eos_Gamma0Obj = numericAttr.create("initialGruneisenGamma", "gamma", MFnNumericData::kFloat, 0.0);
	numericAttr.setKeyable(true);
	numericAttr.setReadable(false);

	MFnTypedAttribute typedAttr;
	materialObj = typedAttr.create("material", "mat", BallisticMaterialNode::GetTypeId());
	typedAttr.setStorable(false);
	typedAttr.setWritable(false);
	typedAttr.setReadable(true);

	// grupowanie
	MFnCompoundAttribute compoundAttr;
	elasticityCompoundObj = compoundAttr.create("elasticity", "elas");
	compoundAttr.addChild(densityObj);
	compoundAttr.addChild(youngModulusObj);
	compoundAttr.addChild(poissonRatioObj);

	plasticityCompoundObj = compoundAttr.create("plasticity", "plas");
	compoundAttr.addChild(AObj);
	compoundAttr.addChild(BObj);
	compoundAttr.addChild(nObj);
	compoundAttr.addChild(CObj);
	compoundAttr.addChild(mObj);

	damageCompoundObj = compoundAttr.create("damage", "dmg");
	compoundAttr.addChild(D1Obj);
	compoundAttr.addChild(D2Obj);
	compoundAttr.addChild(D3Obj);
	compoundAttr.addChild(D4Obj);
	compoundAttr.addChild(D5Obj);

	thermalCompoundObj = compoundAttr.create("thermal", "therm");
	compoundAttr.addChild(T_roomObj);
	compoundAttr.addChild(T_meltObj);
	compoundAttr.addChild(specificHeatObj);
	compoundAttr.addChild(eps_dot_0Obj);
	compoundAttr.addChild(taylorQuinneyObj);

	eosCompoundObj = compoundAttr.create("EOS", "eos");
	compoundAttr.addChild(eos_C0Obj);
	compoundAttr.addChild(eos_SObj);
	compoundAttr.addChild(eos_Gamma0Obj);

	addAttribute(elasticityCompoundObj);
	addAttribute(plasticityCompoundObj);
	addAttribute(damageCompoundObj);
	addAttribute(thermalCompoundObj);
	addAttribute(eosCompoundObj);
	addAttribute(materialObj);

	// dodawanie odddzia³ywania
	attributeAffects(densityObj, materialObj);
	attributeAffects(youngModulusObj, materialObj);
	attributeAffects(poissonRatioObj, materialObj);
	attributeAffects(AObj, materialObj);
	attributeAffects(BObj, materialObj);
	attributeAffects(nObj, materialObj);
	attributeAffects(CObj, materialObj);
	attributeAffects(mObj, materialObj);
	attributeAffects(D1Obj, materialObj);
	attributeAffects(D2Obj, materialObj);
	attributeAffects(D3Obj, materialObj);
	attributeAffects(D4Obj, materialObj);
	attributeAffects(D5Obj, materialObj);
	attributeAffects(T_roomObj, materialObj);
	attributeAffects(T_meltObj, materialObj);
	attributeAffects(specificHeatObj, materialObj);
	attributeAffects(eps_dot_0Obj, materialObj);
	attributeAffects(taylorQuinneyObj, materialObj);
	attributeAffects(eos_C0Obj, materialObj);
	attributeAffects(eos_SObj, materialObj);
	attributeAffects(eos_Gamma0Obj, materialObj);

	return (MS::kSuccess);
}

MTypeId BallisticMaterialNode::GetTypeId()
{
	return (TYPE_ID);
}

MString BallisticMaterialNode::GetTypeName()
{
	return (TYPE_NAME);
}

