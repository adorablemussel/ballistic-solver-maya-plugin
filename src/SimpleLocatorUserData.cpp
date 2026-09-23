#include "SimpleLocatorUserData.h"

////////////////////
// PUBLIC METHODS //
////////////////////
SimpleLocatorUserData::SimpleLocatorUserData(bool deleteAfterUse) : 
	MUserData(deleteAfterUse),
	shapeIndex(0),
	wireframeColor(MColor(1.0f, 1.0f, 1.0f))
{

}

SimpleLocatorUserData::~SimpleLocatorUserData() {

}