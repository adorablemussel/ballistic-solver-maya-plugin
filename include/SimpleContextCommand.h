#pragma once

#include <maya/MPxContextCommand.h>


// do testowania MEL:
// setToolTo `simpleContext`;
class SimpleContextCommand : public MPxContextCommand {
public: 
	SimpleContextCommand();
	virtual ~SimpleContextCommand() override;

	virtual MPxContext* makeObj() override;

// static methods:
	static void* Creator();

	static MString GetCommandName();
};