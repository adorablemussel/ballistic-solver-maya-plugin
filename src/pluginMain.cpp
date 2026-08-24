#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>

#include <gmsh.h>

#include <vector>
#include <string>
#include <exception>

class TestCmd : public MPxCommand {
public:
    TestCmd() {}
    virtual ~TestCmd() {}

    MStatus doIt(const MArgList& args) override;

    static void* creator() {
        return new TestCmd();
    }
};


MStatus TestCmd::doIt(const MArgList& args) {
    return (MS::kSuccess);
}

MStatus initializePlugin(MObject pluginObj) {
    const char* vendor = "Szymon Krzysztofik";
    const char* version = "1.0.0";

	MFnPlugin pluginFn(pluginObj, vendor, version);

    return (MS::kSuccess);
}

MStatus uninitializePlugin(MObject pluginObj) {
    return (MS::kSuccess);
}