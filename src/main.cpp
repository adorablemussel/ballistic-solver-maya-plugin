#include <maya/MDrawRegistry.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>

#include <gmsh.h>

#include <vector>
#include <string>
#include <exception>

#include "MyCallbacks.h"
#include "HelloWorldCommand.h"
#include "HelloWorldDrawOverride.h"
#include "HelloWorldNode.h"
#include "MultiplyNode.h"
#include "RollingNode.h"
#include "SimpleDeformerNode.h"
#include "BlendDeformerNode.h"
#include "AttractDeformerNode.h"
#include "SimpleContext.h"
#include "SimpleContextCommand.h"
#include "SimpleLocatorNode.h"
#include "SimpleLocatorDrawOverride.h"

#include "bMaterialData.h"
#include "bMaterialNode.h"
#include "bMeshData.h"
#include "bMesherNode.h"
#include "bBodyData.h"

MStatus initializePlugin(MObject pluginObj) {

    const char* vendor = "Szymon Krzysztofik";
#ifndef _DEBUG
    const char* version = "1.0.0";
#else
    const char* version = "1.0.0 DEBUG";
#endif
    const char* requiredApiVersion = "Any"; 

    MStatus status;

	MFnPlugin pluginFn(pluginObj, vendor, version, requiredApiVersion, &status);
    if (!status) {
        MGlobal::displayError("Failed to initialize plugin: " + status.errorString());
        return(status);
    }

    /*
    // CALLBACKS
    MyCallbacks::CallbacksAppend();

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

    // ROLLING NODE
    status = pluginFn.registerNode(
        RollingNode::GetTypeName(),
        RollingNode::GetTypeId(),
        RollingNode::Creator,
        RollingNode::Initialize,
        RollingNode::kDependNode);
    if (!status) {
        MGlobal::displayError("Failed to register node: " + RollingNode::GetTypeName());
        return (status);
    }

    // SIMPLE DEFORMER NODE
    status = pluginFn.registerNode(
        SimpleDeformerNode::GetTypeName(),
        SimpleDeformerNode::GetTypeId(),
        SimpleDeformerNode::Creator,
        SimpleDeformerNode::Initialize,
        SimpleDeformerNode::kDeformerNode);
    if (!status) {
		MGlobal::displayError("Failed to register node: " + SimpleDeformerNode::GetTypeName());
		return (status);
    }

    // BLEND DEFORMER NODE
    status = pluginFn.registerNode(
        BlendDeformerNode::GetTypeName(),
        BlendDeformerNode::GetTypeId(),
        BlendDeformerNode::Creator,
        BlendDeformerNode::Initialize,
        BlendDeformerNode::kDeformerNode);
	if (!status) {
		MGlobal::displayError("Failed to register node: " + BlendDeformerNode::GetTypeName());
		return (status);
	}
    // wstrzykniêty skrypt pythonowy dla opcji paintable (w C++ API nie ma tej funkcji)
	MString cmd = MString("import maya.cmds as cmds\n");
	cmd += MString("cmds.makePaintable(\"") + BlendDeformerNode::GetTypeName() + MString("\", \"weights\", attrType=\"multiFloat\", shapeMode=\"deformer\")");
	MGlobal::executePythonCommand(cmd);

    // ATTRACT DEFORMER NODE
    status = pluginFn.registerNode(
        AttractDeformerNode::GetTypeName(),
        AttractDeformerNode::GetTypeId(),
        AttractDeformerNode::Creator,
        AttractDeformerNode::Initialize,
        AttractDeformerNode::kDeformerNode);
    if (!status) {
		MGlobal::displayError("Failed to register node: " + AttractDeformerNode::GetTypeName());
		return (status);
    }

    // SIMPLE CONTEXT
    status = pluginFn.registerContextCommand(
        SimpleContextCommand::GetCommandName(), 
        SimpleContextCommand::Creator);
    if (!status) {
        MGlobal::displayError("Failed to register context command: " + SimpleContextCommand::GetCommandName());
    }
    */

    // SIMPLE LOCATOR NODE & SIMPLE LOCATOR DRAW OVERRIDE
    MString drawDbClassification = SimpleLocatorNode::GetDrawDbClassification();
    
    status = pluginFn.registerNode(
        SimpleLocatorNode::GetTypeName(),
        SimpleLocatorNode::GetTypeId(),
        SimpleLocatorNode::Creator,
        SimpleLocatorNode::Initialize,
        SimpleLocatorNode::kLocatorNode,
        &drawDbClassification);
    if (!status) {
        MGlobal::displayError("Failed to register node: " + SimpleLocatorNode::GetTypeName());
        return (status);
    }

    status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
        drawDbClassification,
        SimpleLocatorNode::GetDrawRegistrationId(),
        SimpleLocatorDrawOverride::Creator);
    if (!status) {
        MGlobal::displayError("Failed to register node: SimpleLocatorDrawOverride.");
        return (status);
    }

    // BALLISITC MATERIAL DATA
    status = pluginFn.registerData(
        bMaterialData::GetTypeName(),
        bMaterialData::GetTypeId(),
        bMaterialData::Creator,
        bMaterialData::kData);
    if (!status) {
        MGlobal::displayError("Failed to register data: " + bMaterialData::GetTypeName());
        return (status); 
    } 
     
	// BALLISTIC MATERIAL NODE
	status = pluginFn.registerNode(
		bMaterialNode::GetTypeName(),
		bMaterialNode::GetTypeId(),
		bMaterialNode::Creator,
		bMaterialNode::Initialize,
		bMaterialNode::kDependNode);
	if (!status) {
		MGlobal::displayError("Failed to register node: " + bMaterialNode::GetTypeName());
		return (status);
	}

    // BALLISTIC MESH DATA
    status = pluginFn.registerData(
        bMeshData::GetTypeName(),
        bMeshData::GetTypeId(),
        bMeshData::Creator,
        bMeshData::kData);
    if (!status) {
        MGlobal::displayError("Failed to register data: " + bMeshData::GetTypeName());
        return (status);
    }

    // BALLISTIC MESHER NODE
    status = pluginFn.registerNode(
        bMesherNode::GetTypeName(),
        bMesherNode::GetTypeId(),
        bMesherNode::Creator,
        bMesherNode::Initialize,
        bMesherNode::kDependNode);
    if (!status) {
        MGlobal::displayError("Failed to register node: " + bMesherNode::GetTypeName());
        return (status);
    }

    // BALLISTIC BODY DATA
    status = pluginFn.registerData(
        bBodyData::GetTypeName(),
        bBodyData::GetTypeId(),
        bBodyData::Creator,
        bBodyData::kData); 
	if (!status) {
		MGlobal::displayError("Failed to register data: " + bBodyData::GetTypeName());
        return (status);
    }


    return (status);
}

MStatus uninitializePlugin(MObject pluginObj) {
    MStatus status;

    MFnPlugin pluginFn(pluginObj);

    /*
    // CALLBACKS
    MyCallbacks::CallbacksRemove();

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

    // ROLLING NODE
    status = pluginFn.deregisterNode(RollingNode::GetTypeId());
    if (!status) {
        MGlobal::displayError("Failed to deregister node: " + RollingNode::GetTypeName());
        return(status);
    }

    // SIMPLE DEFORMER NODE
    status = pluginFn.deregisterNode(SimpleDeformerNode::GetTypeId());
    if (!status) {
		MGlobal::displayError("Failed to deregister node: " + SimpleDeformerNode::GetTypeName());
		return(status);
	}

	// BLEND DEFORMER NODE
	MString cmd = MString("import maya.cmds as cmds\n");
	cmd += MString("cmds.makePaintable(\"") + BlendDeformerNode::GetTypeName() + MString("\", \"weights\", remove=True)");
	MGlobal::executePythonCommand(cmd); 
	status = pluginFn.deregisterNode(BlendDeformerNode::GetTypeId());
	if (!status) {
		MGlobal::displayError("Failed to deregister node: " + BlendDeformerNode::GetTypeName());
		return(status);
	}

    // ATTRACT DEFORMER NODE
    status = pluginFn.deregisterNode(AttractDeformerNode::GetTypeId());
    if (!status) {
		MGlobal::displayError("Failed to deregister node: " + AttractDeformerNode::GetTypeName());
		return(status);
    }

    // SIMPLE CONTEXT
    status = pluginFn.deregisterContextCommand(SimpleContextCommand::GetCommandName());
    if (!status) {
		MGlobal::displayError("Failed to deregister context command: " + SimpleContextCommand::GetCommandName());
		return(status);
    }
    */

    // SIMPLE LOCATOR NODE & SIMPLE LOCATOR DRAW OVERRIDE
    status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(SimpleLocatorNode::GetDrawDbClassification(), SimpleLocatorNode::GetDrawRegistrationId());
    if (!status) {
        MGlobal::displayError("Failed to deregister node: SimpleLocatorDrawOverride.");
        return(status);
    }

    status = pluginFn.deregisterNode(SimpleLocatorNode::GetTypeId());
    if (!status) {
        MGlobal::displayError("Failed to deregister node: " + SimpleLocatorNode::GetTypeName());
        return(status);
    }

    // BALLISTIC MATERIAL DATA
    status = pluginFn.deregisterData(bMaterialData::GetTypeId());
    if (!status) {
        MGlobal::displayError("Failed to deregister data: " + bMaterialData::GetTypeName());
        return(status);
    }

	// BALLISTIC MATERIAL NODE
	status = pluginFn.deregisterNode(bMaterialNode::GetTypeId());
	if (!status) {
		MGlobal::displayError("Failed to deregister node: " + bMaterialNode::GetTypeName());
		return(status);
	}

    // BALLISTIC MESH DATA
    status = pluginFn.deregisterData(bMeshData::GetTypeId());
    if (!status) {
        MGlobal::displayError("Failed to deregister data: " + bMeshData::GetTypeName());
        return(status);
    }

    // BALLISTIC MESHER NODE
    status = pluginFn.deregisterNode(bMesherNode::GetTypeId());
    if (!status) {
        MGlobal::displayError("Failed to deregister node: " + bMesherNode::GetTypeName());
        return(status);
    }

    // BALLISTIC BODY DATA
    status = pluginFn.deregisterData(bBodyData::GetTypeId());
    if (!status) {
		MGlobal::displayError("Failed to deregister data: " + bBodyData::GetTypeName());
		return(status);
    }

    return (status);
}