#include "bBodyData.h"

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>




//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7FD);
static const MString TYPE_NAME = MString("bBodyData");


////////////////////
// PUBLIC METHODS //
////////////////////
bBodyData::bBodyData() : MPxData()
{

}

bBodyData::~bBodyData()
{

}



/////////////////////
// VIRTUAL METHODS //
/////////////////////
void bBodyData::copy(const MPxData& src)
{
	const bBodyData* srcData = dynamic_cast<const bBodyData*>(&src);
	if (srcData) {
		this->material = srcData->material;
		this->vertices = srcData->vertices;
		this->tetrahedrons = srcData->tetrahedrons;
		this->velocity = srcData->velocity;
	}
	else {
		MGlobal::displayError("Failed to copy data to " + bBodyData::GetTypeName()  + " data type");
	}
}

MTypeId bBodyData::typeId() const
{
	return (TYPE_ID);
}

MString bBodyData::name() const
{
	return (TYPE_NAME);
}


MStatus bBodyData::readASCII(const MArgList& argList, unsigned int& endOfTheLastParsedElement)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bBodyData::readBinary(std::istream& in, unsigned int length)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bBodyData::writeASCII(std::ostream& out)
{
	// TO DO
	return (MS::kSuccess);
}

MStatus bBodyData::writeBinary(std::ostream& out)
{
	// TO DO
	return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////
void* bBodyData::Creator()
{
	return (new bBodyData());
}

MTypeId bBodyData::GetTypeId()
{
	return (TYPE_ID);
}

MString bBodyData::GetTypeName()
{
	return (TYPE_NAME);
}

