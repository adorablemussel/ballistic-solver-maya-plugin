#pragma once 

#include <maya/MPxLocatorNode.h>

class SimpleLocatorNode : public MPxLocatorNode {
public:
	SimpleLocatorNode();
	virtual ~SimpleLocatorNode() override;

// static methods:
	static void* Creator();
	static MStatus Initialize();

	static MTypeId GetTypeId();
	static MString GetTypeName();

	static MString GetDrawDbClassification();
	static MString GetDrawRegistrationId();

private:
	static MObject shapeIndexObj;
};