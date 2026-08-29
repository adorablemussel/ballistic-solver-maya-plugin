#include "HelloWorldCommand.h"

#include <maya/MGlobal.h>
#include <maya/MIOStream.h>

//////////////////////
// STATIC CONSTANTS //
//////////////////////
static const MString MEL_COMMAND = "HelloWorld";

////////////////////
// PUBLIC METHODS //
////////////////////
HelloWorldCommand::HelloWorldCommand() {

}

HelloWorldCommand::~HelloWorldCommand() {

}

MStatus HelloWorldCommand::doIt(const MArgList& args) {
	MGlobal::displayInfo("Hello World!");
	cout << "Hello World!\n";

	return (MS::kSuccess);
}

////////////////////
// STATIC METHODS //
////////////////////
void* HelloWorldCommand::Creator() {
	return(new HelloWorldCommand());
}

MString HelloWorldCommand::CommandName() {
	return (MEL_COMMAND);
}
