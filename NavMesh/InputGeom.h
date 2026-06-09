#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>

#include "PartitionedMesh.h"

struct PartitionedMesh;
class rcContext;
struct duDebugDraw;

static constexpr int MAX_CONVEXVOL_PTS = 12;
struct ConvexVolume
{
	float verts[MAX_CONVEXVOL_PTS * 3] = {};
	int nverts = 0;
	float hmin = 0.0f;
	float hmax = 0.0f;
	int area = 0;
};

struct BuildSettings
{
	/// Cell size in world units
	float cellSize = 0;
	/// Cell height in world units
	float cellHeight = 0;
	/// Agent height in world units
	float agentHeight = 0;
	/// Agent radius in world units
	float agentRadius = 0;
	/// Agent max climb in world units
	float agentMaxClimb = 0;
	/// Agent max slope in degrees
	float agentMaxSlope = 0;
	/// Region minimum size in voxels.
	float regionMinSize = 0;
	/// Region merge size in voxels. regionMergeSize = sqrt(regionMergeArea)
	float regionMergeSize = 0;
	/// Edge max length in world units
	float edgeMaxLen = 0;
	/// Edge max error in voxels
	float edgeMaxError = 0;
	int vertsPerPoly = 0;
	/// Detail sample distance in voxels
	float detailSampleDist = 0;
	/// Detail sample max error in voxel heights.
	float detailSampleMaxError = 0;
	/// Partition type, see SamplePartitionType
	int partitionType = 0;
	/// Bounds of the area to mesh
	float navMeshBMin[3]{};
	float navMeshBMax[3]{};
	/// Size of the tiles in voxels
	float tileSize = 0;
};

struct Mesh
{
	std::vector<float> verts;
	std::vector<int> tris;
	std::vector<float> normals;  // face normals

	void reset()
	{
		verts.clear();
		tris.clear();
		normals.clear();
	}

	[[nodiscard]] int getVertCount() const { return static_cast<int>(verts.size()) / 3; }
	[[nodiscard]] int getTriCount() const { return static_cast<int>(tris.size()) / 3; }

	void readFromObj(char* buf, size_t bufLen);
};

class InputGeom
{
	BuildSettings buildSettings;
	bool hasBuildSettings = false;

public:
	std::string filename;

	Mesh mesh;
	float meshBoundsMin[3] = {};
	float meshBoundsMax[3] = {};

	PartitionedMesh partitionedMesh;

	bool LoadMesh(rcContext* ctx, const std::string& filepath);

	std::vector<float> offmeshConnVerts;
	std::vector<float> offmeshConnRadius;
	std::vector<unsigned char> offmeshConnBidirectional;
	std::vector<unsigned char> offmeshConnArea;
	std::vector<unsigned short> offmeshConnFlags;
	std::vector<unsigned int> offmeshConnId;

	std::vector<ConvexVolume> convexVolumes;

	/// Method to return static mesh data.
	[[nodiscard]] const float* getNavMeshBoundsMin() const { return hasBuildSettings ? buildSettings.navMeshBMin : meshBoundsMin; }
	[[nodiscard]] const float* getNavMeshBoundsMax() const { return hasBuildSettings ? buildSettings.navMeshBMax : meshBoundsMax; }
	[[nodiscard]] const BuildSettings* getBuildSettings() const { return hasBuildSettings ? &buildSettings : nullptr; }

private:


	void clearOffMeshConnections()
	{
		offmeshConnVerts.clear();
		offmeshConnRadius.clear();
		offmeshConnBidirectional.clear();
		offmeshConnArea.clear();
		offmeshConnFlags.clear();
		offmeshConnId.clear();
	}
};