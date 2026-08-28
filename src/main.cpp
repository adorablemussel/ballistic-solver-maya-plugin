#include <maya/MDrawRegistry.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>

#include <gmsh.h>

#include <vector>
#include <string>
#include <exception>

#include "HelloWorldCommand.h"
#include "HelloWorldDrawOverride.h"
#include "HelloWorldNode.h"
#include "MultiplyNode.h"

MStatus initializePlugin(MObject pluginObj) {

#ifdef _DEBUG
    MGlobal::displayInfo("Initialized Plugin!");
#endif
    const char* vendor = "Szymon Krzysztofik";
    const char* version = "1.0.0";
    const char* requiredApiVersion = "Any";

    MStatus status;

	MFnPlugin pluginFn(pluginObj, vendor, version, requiredApiVersion, & status);
    if (!status) {
        MGlobal::displayError("Failed to initialize plugin: " + status.errorString());
        return(status);
    }

    // HELLO WORLD COMMAND

    status = pluginFn.registerCommand(
        HelloWorldCommand::CommandName(), 
        HelloWorldCommand::Creator);
    if (!status) {
        MGlobal::displayError("Failed to register HelloWorldCommand.");
        return(status);
    }

    // HELLO WORLD NODE (+DRAW OVERRIDE)

    MString drawDbClassification = HelloWorldNode::GetDrawDbClassification();

    status = pluginFn.registerNode(
        HelloWorldNode::GetTypeName(),
        HelloWorldNode::GetTypeId(),
        HelloWorldNode::Creator,
        HelloWorldNode::Initialize,
        HelloWorldNode::kLocatorNode,
        &drawDbClassification);
    if (!status) {
        MGlobal::displayError("Failed to register HelloWorldNode.");
            return (status);
    }

    status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
        HelloWorldNode::GetDrawDbClassification(),
        HelloWorldNode::GetDrawingRegistrationId(),
        HelloWorldDrawOverride::Creator);
    if (!status) {
		MGlobal::displayError("Failed to register HelloWorldDrawOverride.");
		return(status);
    }

    // MULTIPLY NODE

	status = pluginFn.registerNode(
		MultiplyNode::GetTypeName(),
		MultiplyNode::GetTypeId(),
		MultiplyNode::Creator,
		MultiplyNode::Initialize,
        MultiplyNode::kDependNode);
	if (!status) {
		MGlobal::displayError("Failed to register node: " + MultiplyNode::GetTypeName());
		return (status);
	}

    return (status);
}

MStatus uninitializePlugin(MObject pluginObj) {
    MStatus status;

    MFnPlugin pluginFn(pluginObj);

    // HELLO WORLD COMMAND

    status = pluginFn.deregisterCommand(HelloWorldCommand::CommandName());
    if (!status) {
        MGlobal::displayError("Failed to deregister HelloWorldCommand.");
        return(status);
    }

    // HELLO WORLD NODE (+ DRAW OVERRIDE)

    status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(HelloWorldNode::GetDrawDbClassification(), HelloWorldNode::GetDrawingRegistrationId());
    if (!status) {
		MGlobal::displayError("Failed to deregister HelloWorldDrawOverride.");
		return(status);
    }

    status = pluginFn.deregisterNode(HelloWorldNode::GetTypeId());
    if (!status) {
		MGlobal::displayError("Failed to deregister HelloWorldNode.");
		return(status);
    }

    // MULTIPLY NODE

	status = pluginFn.deregisterNode(MultiplyNode::GetTypeId());
	if (!status) {
		MGlobal::displayError("Failed to deregister node: " + MultiplyNode::GetTypeName());
		return(status);
	}

    return (status);
}