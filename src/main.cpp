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
#include "bMeshVisualizationNode.h"
#include "bMeshVisualizationDrawOverride.h"
#include "bBodyNode.h"
#include "bSolverNode.h"

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

    // BALLISTIC BODY NODE
    status = pluginFn.registerNode(
        bBodyNode::GetTypeName(),
        bBodyNode::GetTypeId(),
        bBodyNode::Creator,
        bBodyNode::Initialize,
        bBodyNode::kDependNode);
    if (!status) {
        MGlobal::displayError("Failed to register node: " + bBodyNode::GetTypeName());
        return (status);
    }

    // BALLISTIC MESH VISUALIZATION NODE & BALLISTIC MESH VISUALIZATION DRAW OVERRIDE
    MString drawDbVisClassification = bMeshVisualizationNode::GetDrawDbClassification();

    status = pluginFn.registerNode(
        bMeshVisualizationNode::GetTypeName(),
        bMeshVisualizationNode::GetTypeId(),
        bMeshVisualizationNode::Creator,
        bMeshVisualizationNode::Initialize,
        bMeshVisualizationNode::kLocatorNode,
        &drawDbVisClassification);
    if (!status) {
        MGlobal::displayError("Failed to register node: " + bMeshVisualizationNode::GetTypeName());
        return (status);
    }

    status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(
        drawDbVisClassification,
        bMeshVisualizationNode::GetDrawRegistrationId(),
        bMeshVisualizationDrawOverride::Creator);
    if (!status) {
        MGlobal::displayError("Failed to register node: bMeshVisualizationDrawOverride.");
        return (status);
    }
        
    // SOLVER NODE
    status = pluginFn.registerNode(
        bSolverNode::GetTypeName(),
        bSolverNode::GetTypeId(),
        bSolverNode::Creator,
        bSolverNode::Initialize,
        bSolverNode::kDependNode);
    if (!status) {
        MGlobal::displayError("Failed to register node: " + bSolverNode::GetTypeName());
        return (status);
    }

    return (status);
}

MStatus uninitializePlugin(MObject pluginObj) {
    MStatus status;

    MFnPlugin pluginFn(pluginObj);

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

    // BALLISTIC BODY NODE
    status = pluginFn.deregisterNode(bBodyNode::GetTypeId());
    if (!status) {
        MGlobal::displayError("Failed to deregister node: " + bBodyNode::GetTypeName());
        return(status); 
    }

    // BALLISTIC MESH VISUALIZATION NODE & BALLISTIC MESH VISUALIZATION DRAW OVERRIDE
    status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(bMeshVisualizationNode::GetDrawDbClassification(), bMeshVisualizationNode::GetDrawRegistrationId());
    if (!status) {
        MGlobal::displayError("Failed to deregister node: bMeshVisualizationDrawOverride.");
        return(status);
    }

    status = pluginFn.deregisterNode(bMeshVisualizationNode::GetTypeId());
    if (!status) {
        MGlobal::displayError("Failed to deregister node: " + bMeshVisualizationNode::GetTypeName());
        return(status); 
    }

    // SOLVER NODE
    status = pluginFn.deregisterNode(bSolverNode::GetTypeId());
    if (!status) {
        MGlobal::displayError("Failed to deregister node: " + bSolverNode::GetTypeName());
        return(status);
    }

    return (status);
}