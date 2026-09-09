#include "BallisticBodyData.h"

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MGlobal.h>




//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MTypeId TYPE_ID = MTypeId(0x0007F7FD);
static const MString TYPE_NAME = MString("ballisticbodydata");


////////////////////
// PUBLIC METHODS //
////////////////////
BallisticBodyData::BallisticBodyData() : MPxData()
{

}

BallisticBodyData::~BallisticBodyData()
{

}



/////////////////////
// VIRTUAL METHODS //
/////////////////////
void BallisticBodyData::copy(const MPxData& src)
{
	const BallisticBodyData* srcData = dynamic_cast<const BallisticBodyData*>(&src);
	if (srcData) {
		this->material = srcData->material;
		this->vertices = srcData->vertices;
		this->tetrahedrons = srcData->tetrahedrons;
	}
	else {
		MGlobal::displayError("Failed to copy data to BallisticBodyData data type");
	}
}

MTypeId BallisticBodyData::typeId() const
{
	return (TYPE_ID);
}

MString BallisticBodyData::name() const
{
	return (TYPE_NAME);
}


////////////////////
// STATIC METHODS //
////////////////////
void* BallisticBodyData::Creator()
{
	return (new BallisticBodyData);
}

MTypeId BallisticBodyData::GetTypeId()
{
	return (TYPE_ID);
}

MString BallisticBodyData::GetTypeName()
{
	return (TYPE_NAME);
}

