#include "bMaterialNode.h"
#include "bMaterialData.h"
#include "bMaterial.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnPluginData.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7FE);
static const MString TYPE_NAME = MString("bMaterialNode");


//////////////////////
// STATIC VARIABLES //
//////////////////////
MObject bMaterialNode::elasticityCompoundObj;
MObject bMaterialNode::plasticityCompoundObj;
MObject bMaterialNode::damageCompoundObj;
MObject bMaterialNode::thermalCompoundObj;
MObject bMaterialNode::eosCompoundObj;

MObject bMaterialNode::densityObj;
MObject bMaterialNode::youngModulusObj;
MObject bMaterialNode::poissonRatioObj;
MObject bMaterialNode::AObj;
MObject bMaterialNode::BObj;
MObject bMaterialNode::nObj;
MObject bMaterialNode::CObj;
MObject bMaterialNode::mObj;
MObject bMaterialNode::D1Obj;
MObject bMaterialNode::D2Obj;
MObject bMaterialNode::D3Obj;
MObject bMaterialNode::D4Obj;
MObject bMaterialNode::D5Obj;
MObject bMaterialNode::T_roomObj;
MObject bMaterialNode::T_meltObj;
MObject bMaterialNode::specificHeatObj;
MObject bMaterialNode::eps_dot_0Obj;
MObject bMaterialNode::taylorQuinneyObj;
MObject bMaterialNode::eos_C0Obj;
MObject bMaterialNode::eos_SObj;
MObject bMaterialNode::eos_Gamma0Obj;

MObject bMaterialNode::materialObj;

////////////////////
// PUBLIC METHODS //
////////////////////
bMaterialNode::bMaterialNode() : MPxNode()
{

}

bMaterialNode::~bMaterialNode()
{

}

MStatus bMaterialNode::compute(const MPlug& plug, MDataBlock& data)
{
    if (plug == materialObj)
    {
        MFnPluginData pluginDataFn;
        MObject newMaterialObject = pluginDataFn.create(bMaterialData::GetTypeId());

        MPxData* rawData = pluginDataFn.data();
        bMaterialData* outData = dynamic_cast<bMaterialData*>(rawData);
        if (!outData) {
            return (MS::kFailure);
        }

        outData->material.density = data.inputValue(densityObj).asFloat();
        outData->material.youngModulus = data.inputValue(youngModulusObj).asFloat();
        outData->material.poissonRatio = data.inputValue(poissonRatioObj).asFloat();

        outData->material.A = data.inputValue(AObj).asFloat();
        outData->material.B = data.inputValue(BObj).asFloat();
        outData->material.n = data.inputValue(nObj).asFloat();
        outData->material.C = data.inputValue(CObj).asFloat();
        outData->material.m = data.inputValue(mObj).asFloat();

        outData->material.D1 = data.inputValue(D1Obj).asFloat();
        outData->material.D2 = data.inputValue(D2Obj).asFloat();
        outData->material.D3 = data.inputValue(D3Obj).asFloat();
        outData->material.D4 = data.inputValue(D4Obj).asFloat();
        outData->material.D5 = data.inputValue(D5Obj).asFloat();

        outData->material.T_room = data.inputValue(T_roomObj).asFloat();
        outData->material.T_melt = data.inputValue(T_meltObj).asFloat();
        outData->material.specificHeat = data.inputValue(specificHeatObj).asFloat();
        outData->material.eps_dot_0 = data.inputValue(eps_dot_0Obj).asFloat();
        outData->material.taylorQuinney = data.inputValue(taylorQuinneyObj).asFloat();

        outData->material.eos_C0 = data.inputValue(eos_C0Obj).asFloat();
        outData->material.eos_S = data.inputValue(eos_SObj).asFloat();
        outData->material.eos_Gamma0 = data.inputValue(eos_Gamma0Obj).asFloat();

        MDataHandle materialDataHandle = data.outputValue(materialObj);
        materialDataHandle.set(newMaterialObject);

        data.setClean(plug);
    }
    else
    {
        return (MS::kFailure);
    }

    return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////

void* bMaterialNode::Creator()
{
    return (new bMaterialNode());
}

MStatus bMaterialNode::Initialize()
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
    materialObj = typedAttr.create("material", "mat", bMaterialData::GetTypeId());
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

MTypeId bMaterialNode::GetTypeId()
{
    return (TYPE_ID);
}

MString bMaterialNode::GetTypeName()
{
    return (TYPE_NAME);
}
