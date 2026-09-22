#include "bMaterialData.h"

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7FF);
static const MString TYPE_NAME = MString("bMaterialData");

////////////////////
// PUBLIC METHODS //
////////////////////
bMaterialData::bMaterialData() : MPxData() 
{

}

bMaterialData::~bMaterialData()
{

}

/////////////////////
// VIRTUAL METHODS //
/////////////////////
void bMaterialData::copy(const MPxData& src)
{
	const bMaterialData* srcData = dynamic_cast<const bMaterialData*>(&src);
	if (srcData) {
		this->material = srcData->material;
	}
	else {
		MGlobal::displayError("Failed to copy data to " + bMaterialData::GetTypeName() + " data type");
	}
}

MTypeId bMaterialData::typeId() const
{
	return (TYPE_ID);
}

MString bMaterialData::name() const
{
	return (TYPE_NAME);
}

MStatus bMaterialData::readASCII(const MArgList& argList, unsigned int& endOfTheLastParsedElement)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bMaterialData::readBinary(std::istream& in, unsigned int length)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bMaterialData::writeASCII(std::ostream& out)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bMaterialData::writeBinary(std::ostream& out)
{
	// TO DO
	return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////
void* bMaterialData::Creator() {
	return (new bMaterialData());
}

MTypeId bMaterialData::GetTypeId() {
	return (TYPE_ID);
}

MString bMaterialData::GetTypeName() {
	return (TYPE_NAME);
}