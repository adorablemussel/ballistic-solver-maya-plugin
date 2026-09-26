#pragma once

#include <maya/MPxData.h>
#include <vector>

class bMeshData : public MPxData {
public:
	std::vector<float> vertices; // [x1, y1, z1, x2, y2, z2, x3, ...] - co trzy wspó³rzêdne jest nowy wêze³
	std::vector<int> tetrahedrons; // [[v1, v2, v3, v4], [v2, v3, v5, v6], [...]] - co cztery wêz³y jest nowy tet

	bMeshData();
	virtual ~bMeshData() override;

	// virtual methods:
	virtual void copy(const MPxData& src) override;
	virtual MTypeId typeId() const override;
	virtual MString name() const override;

	virtual MStatus readASCII(const MArgList& argList, unsigned int& endOfTheLastParsedElement) override;
	virtual MStatus readBinary(std::istream& in, unsigned int length) override;
	virtual MStatus writeASCII(std::ostream& out) override;
	virtual MStatus writeBinary(std::ostream& out) override;

	// static methods:
	static void* Creator();

	static MTypeId GetTypeId();
	static MString GetTypeName();

private:

};