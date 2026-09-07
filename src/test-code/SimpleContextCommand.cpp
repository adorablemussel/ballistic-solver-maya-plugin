#include "SimpleContextCommand.h"
#include "SimpleContext.h"

//////////////////////
// STATIC CONSTANTS //
//////////////////////

static const MString COMMAND_NAME = "simpleContext";

////////////////////
// PUBLIC METHODS //
////////////////////

SimpleContextCommand::SimpleContextCommand() : MPxContextCommand()
{

}

SimpleContextCommand::~SimpleContextCommand()
{

}


MPxContext* SimpleContextCommand::makeObj() {
	return (new SimpleContext());
}

////////////////////
// STATIC METHODS //
////////////////////

void* SimpleContextCommand::Creator() {
	return (new SimpleContextCommand());
}

MString SimpleContextCommand::GetCommandName() {
	return (COMMAND_NAME);
}