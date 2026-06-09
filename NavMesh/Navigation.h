#pragma once

#include "Recast.h"
#include "InputGeom.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"

#include <memory>
#include <string>

class Navigation
{
public:
	Navigation();
	~Navigation() {}

	bool Build();
	bool FindPath(float* startPos, float* endPos, std::vector<float>& outPath);

	std::string basicLevelObjPath;

	std::unique_ptr<InputGeom> inputGeometry;
	std::unique_ptr<rcContext> buildContext;

	bool filterLowHangingObstacles = true;
	bool filterLedgeSpans = true;
	bool filterWalkableLowHeightSpans = true;

	rcConfig config{};

	unsigned char* triAreas = nullptr;
	rcHeightfield* heightfield = nullptr;
	rcCompactHeightfield* compactHeightfield = nullptr;
	rcContourSet* contourSet = nullptr;
	rcPolyMesh* polyMesh = nullptr;
	rcPolyMeshDetail* detailMesh = nullptr;

	dtNavMesh* navMesh = nullptr;
	dtNavMeshQuery* navQuery = nullptr;
};