#pragma once

#include <maya/MPxCommand.h>

class HelloWorldCommand : public MPxCommand {
public:
	HelloWorldCommand();
	virtual ~HelloWorldCommand() override;

	virtual MStatus doIt(const MArgList& args) override;

// static methods:
	static void* Creator();

	static MString CommandName();
};