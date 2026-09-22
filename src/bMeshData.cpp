#include "bMeshData.h"

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F800);
static const MString TYPE_NAME = MString("bMeshData");


////////////////////
// PUBLIC METHODS //
////////////////////
bMeshData::bMeshData() : MPxData()
{
}

bMeshData::~bMeshData()
{
}

/////////////////////
// VIRTUAL METHODS //
/////////////////////
void bMeshData::copy(const MPxData& src)
{
	const bMeshData* srcData = dynamic_cast<const bMeshData*>(&src);
	if (srcData) {
		this->vertices = srcData->vertices;
		this->tetrahedrons = srcData->tetrahedrons;
	}
	else {
		MGlobal::displayError("Failed to copy data to " + bMeshData::GetTypeName() + " data type");
	}
}

MTypeId bMeshData::typeId() const
{
	return (TYPE_ID);
}

MString bMeshData::name() const
{
	return (TYPE_NAME);
}

MStatus bMeshData::readASCII(const MArgList& argList, unsigned int& endOfTheLastParsedElement)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bMeshData::readBinary(std::istream& in, unsigned int length)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bMeshData::writeASCII(std::ostream& out)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bMeshData::writeBinary(std::ostream& out)
{
	// TO DO
	return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////
void* bMeshData::Creator() {
	return (new bMeshData());
}

MTypeId bMeshData::GetTypeId() {
	return (TYPE_ID);
}

MString bMeshData::GetTypeName() {
	return (TYPE_NAME);
}