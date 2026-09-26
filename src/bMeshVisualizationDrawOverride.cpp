#include "bMeshVisualizationDrawOverride.h"
#include "bMeshVisualizationData.h"
#include "bMeshVisualizationNode.h"
#include "bMeshData.h"

#include <maya/MFnPluginData.h>

////////////////////
// PUBLIC METHODS //
////////////////////
bMeshVisualizationDrawOverride::~bMeshVisualizationDrawOverride()
{
}

MHWRender::DrawAPI bMeshVisualizationDrawOverride::supportedDrawAPIs() const
{
	return (MHWRender::kAllDevices);
}

bool bMeshVisualizationDrawOverride::hasUIDrawables() const
{
	return (true);
}

// odczyt danych co klatkê
MUserData* bMeshVisualizationDrawOverride::prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath, const MFrameContext& frameContext, MUserData* oldData)
{
	bMeshVisualizationData* data = dynamic_cast<bMeshVisualizationData*>(oldData);
	if (!data) {
		data = new bMeshVisualizationData();
	}

	data->wireframeLines.clear();

	MObject thisNode = objPath.node();
	MPlug meshPlug(thisNode, bMeshVisualizationNode::inTetMeshObj);

	MObject meshDataObj;
	meshPlug.getValue(meshDataObj);

	if (meshDataObj.isNull()) {
		return data;
	}

	MFnPluginData pluginDataFn(meshDataObj);
	bMeshData* meshData = dynamic_cast<bMeshData*>(pluginDataFn.data());

	if (meshData) {
		for (size_t i = 0; i < meshData->tetrahedrons.size(); i += 4) {
			// id wierzcho³ków
			int idA = meshData->tetrahedrons[i];
			int idB = meshData->tetrahedrons[i + 1];
			int idC = meshData->tetrahedrons[i + 2];
			int idD = meshData->tetrahedrons[i + 3];

			// wspó³rzêdne wierzcho³ków
			MPoint A(meshData->vertices[idA * 3], meshData->vertices[idA * 3 + 1], meshData->vertices[idA * 3 + 2]);
			MPoint B(meshData->vertices[idB * 3], meshData->vertices[idB * 3 + 1], meshData->vertices[idB * 3 + 2]);
			MPoint C(meshData->vertices[idC * 3], meshData->vertices[idC * 3 + 1], meshData->vertices[idC * 3 + 2]);
			MPoint D(meshData->vertices[idD * 3], meshData->vertices[idD * 3 + 1], meshData->vertices[idD * 3 + 2]);

			// tworzenie krawêdzi
			//1
			data->wireframeLines.append(A);
			data->wireframeLines.append(B);
			//2
			data->wireframeLines.append(B);
			data->wireframeLines.append(C);
			//3
			data->wireframeLines.append(C);
			data->wireframeLines.append(A);
			//4
			data->wireframeLines.append(A);
			data->wireframeLines.append(D);
			//5
			data->wireframeLines.append(B);
			data->wireframeLines.append(D);
			//6
			data->wireframeLines.append(C);
			data->wireframeLines.append(D);

		}
	}
	return data;
}

void bMeshVisualizationDrawOverride::addUIDrawables(const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data)
{
	bMeshVisualizationData* visData = (bMeshVisualizationData*)data;
	if (!visData || visData->wireframeLines.length() == 0) {
		return;
	}

	drawManager.beginDrawable();

	drawManager.setColor(MColor(1.0f, 1.0f, 1.0f));
	drawManager.setLineWidth(1.0f);

	drawManager.lineList(visData->wireframeLines, false);

	drawManager.endDrawable();

}

////////////////////
// STATIC METHODS //
////////////////////
MHWRender::MPxDrawOverride* bMeshVisualizationDrawOverride::Creator(const MObject& obj) {
	return (new bMeshVisualizationDrawOverride(obj));
}

/////////////////////
// PRIVATE METHODS //
/////////////////////
bMeshVisualizationDrawOverride::bMeshVisualizationDrawOverride(const MObject& obj) : MHWRender::MPxDrawOverride(obj, nullptr, true) 
{

} 