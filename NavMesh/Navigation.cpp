#include "Navigation.h"
#include "DetourCommon.h"

#include <string>
#include <iostream>
#include <fstream>

Navigation::Navigation()
{
	//TODO env에서 정보 가져오거나 폴더에있는 파일 탐색해서 가져오면 좋을 듯.
	basicLevelObjPath = R"(D:\Projects\GameServerProject\Project_Island\MapData\BasicLevel.obj)";

	inputGeometry = std::make_unique<InputGeom>();
	buildContext = std::make_unique<rcContext>();
	inputGeometry->LoadMesh(nullptr, basicLevelObjPath);
}

bool Navigation::Build()
{
	if (!inputGeometry || inputGeometry->mesh.verts.empty())
	{
		return false;
	}

	const float* boundsMin = inputGeometry->getNavMeshBoundsMin();
	const float* boundsMax = inputGeometry->getNavMeshBoundsMax();
	const float* verts = inputGeometry->mesh.verts.data();
	const int numVerts = static_cast<int>(inputGeometry->mesh.verts.size()) / 3;
	const int* tris = inputGeometry->mesh.tris.data();
	const int numTris = static_cast<int>(inputGeometry->mesh.tris.size()) / 3;
	const float agentHeight = 180.f;
	const float agentRadius = 30.f;
	const float agentMaxClimb = 90.f;

	// Step 1. Initialize build config.
	memset(&config, 0, sizeof(rcConfig));
	config.cs = 30.f;//cellSize
	config.ch = 20.f;//cellHeight
	config.walkableSlopeAngle = 45.0f;//agentMaxSlope
	config.walkableHeight = static_cast<int>(ceilf(agentHeight / config.ch));;//agentHeight
	config.walkableClimb = static_cast<int>(floorf(agentMaxClimb / config.ch));//agentMaxClimb
	config.walkableRadius = static_cast<int>(ceilf(agentRadius / config.cs));//agentRadius
	config.maxEdgeLen = static_cast<int>(1200.f / config.cs);//edgeMaxLen / cellSize
	config.maxSimplificationError = 1.3f;//edgeMaxError
	config.minRegionArea = static_cast<int>(rcSqr(8));// regionMinSize Note: area = size*size
	config.mergeRegionArea = static_cast<int>(rcSqr(20));// regionMergeSize Note: area = size*size
	config.maxVertsPerPoly = 6;//vertsPerPoly
	config.detailSampleDist = 6.f < 0.9f ? 0 : config.cs * 6.f;//cellSize * cellSize
	config.detailSampleMaxError = config.ch * 1.f;//cellHeight * detailSampleMaxError

	// Set the area where the navigation will be built.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by a user defined box, etc.
	rcVcopy(config.bmin, boundsMin);
	rcVcopy(config.bmax, boundsMax);
	rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

	// Step 2. Rasterize input meshes.
	heightfield = rcAllocHeightfield();
	if (!heightfield)
	{
		return false;
	}
	if (!rcCreateHeightfield(
		buildContext.get(),
		*heightfield,
		config.width,
		config.height,
		config.bmin,
		config.bmax,
		config.cs,
		config.ch))
	{
		return false;
	}

	// Allocate array that can hold triangle area types.
	// This is used to store terrain type information and to mark
	// triangles as unwalkable.
	// If you have multiple meshes you need to process, allocate
	// an array which can hold the max number of triangles you need to process.
	triAreas = new unsigned char[numTris];
	if (!triAreas)
	{
		return false;
	}
	memset(triAreas, 0, numTris * sizeof(unsigned char));

	// Record which triangles in the input mesh are walkable.
	// This information is recorded in triAreas
	rcMarkWalkableTriangles(buildContext.get(), config.walkableSlopeAngle, verts, numVerts, tris, numTris, triAreas);

	// Rasterize the input mesh
	// If your have multiple meshes, you can transform them, calculate the
	// terrain type for each mesh and rasterize them here.
	if (!rcRasterizeTriangles(buildContext.get(), verts, numVerts, tris, triAreas, numTris, *heightfield, config.walkableClimb))
	{
		return false;
	}

	// Step 3. Filter walkable surfaces.

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as spans where the character cannot possibly stand.
	if (filterLowHangingObstacles)
	{
		rcFilterLowHangingWalkableObstacles(buildContext.get(), config.walkableClimb, *heightfield);
	}
	if (filterLedgeSpans)
	{
		rcFilterLedgeSpans(buildContext.get(), config.walkableHeight, config.walkableClimb, *heightfield);
	}
	if (filterWalkableLowHeightSpans)
	{
		rcFilterWalkableLowHeightSpans(buildContext.get(), config.walkableHeight, *heightfield);
	}

	// Step 4. Partition walkable surface into simple regions.

	// Compact the heightfield so that it is faster to work with.
	// This will result more cache coherent data.  This step will also
	// generate neighbor connection information between walkable cells.
	compactHeightfield = rcAllocCompactHeightfield();
	if (!compactHeightfield)
	{
		return false;
	}
	if (!rcBuildCompactHeightfield(
		buildContext.get(),
		config.walkableHeight,
		config.walkableClimb,
		*heightfield,
		*compactHeightfield))
	{
		return false;
	}

	// Erode the walkable area by agent radius.
	// This allows us to path an agent through the navmesh as if it was a single point
	if (!rcErodeWalkableArea(buildContext.get(), config.walkableRadius, *compactHeightfield))
	{
		return false;
	}

	// (Optional) Marks the surface type of voxels in an area defined by a convex volume.
	// Useful to mark areas of differing cost.
	for (ConvexVolume& vol : inputGeometry->convexVolumes)
	{
		rcMarkConvexPolyArea(
			buildContext.get(),
			vol.verts,
			vol.nverts,
			vol.hmin,
			vol.hmax,
			(unsigned char)vol.area,
			*compactHeightfield);
	}

	// Partition the heightfield into contiguous regions that will each be
	// triangulated into navigation polygons.
	//
	// There are 3 partitioning methods, each with their own pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - the are some corner cases where this method creates holes and
	//     overlaps in the resulting region data.
	//      - holes may appear when a small obstacle is close to a large open
	//        area.  This will not cause triangulation to fail.
	//      - overlaps may occur if you have narrow spiral corridors
	//        e.g. spiral stairs.  This will cause triangulation to fail.
	//   * Generally the best choice if you are precompute the navmesh and/or
	//     there are large open areas in the input geometry.
	// 2) Monotone partitioning
	//   - fastest
	//   - guaranteed to partition the heightfield into regions without holes
	//     or overlaps
	//   - Can create long, thin polygons which sometimes cause paths with detours
	//   * Use this if you want fast navmesh generation
	// 3) Layer partitioning
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes, which makes
	//     this slower than monotone partitioning
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a slightly ugly tessellation (still better
	//     than monotone) if you have large open areas with small obstacles.
	//     This is less of a problem if you use a tiled navmesh.
	//   * A good choice for a tiled navmesh with small to medium-sized tiles

	// Watershed partitioning
	if (!rcBuildDistanceField(buildContext.get(), *compactHeightfield))
	{
		return false;
	}

	// Partition the walkable surface into contiguous regions.
	if (!rcBuildRegions(buildContext.get(), *compactHeightfield, 0, config.minRegionArea, config.mergeRegionArea))
	{
		return false;
	}

	// Step 5. Trace and simplify region contours.

	// Create contour.
	contourSet = rcAllocContourSet();
	if (!contourSet)
	{
		return false;
	}
	if (!rcBuildContours(buildContext.get(), *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet))
	{
		return false;
	}

	// Step 6. Triangulate contours to build navmesh polygons.
	
	polyMesh = rcAllocPolyMesh();
	if (!polyMesh)
	{
		return false;
	}
	if (!rcBuildPolyMesh(buildContext.get(), *contourSet, config.maxVertsPerPoly, *polyMesh))
	{
		return false;
	}

	// Step 7. Create a navmesh from the triangulated polygons.
	// Calculates additional information necessary to run pathing queries.

	detailMesh = rcAllocPolyMeshDetail();
	if (!detailMesh)
	{
		return false;
	}
	if (!rcBuildPolyMeshDetail(
		buildContext.get(),
		*polyMesh,
		*compactHeightfield,
		config.detailSampleDist,
		config.detailSampleMaxError,
		*detailMesh))
	{
		return false;
	}

	// Step 8. Create Detour data from Recast poly mesh.
	if (config.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = nullptr;
		int navDataSize = 0;

		// 폴리곤 플래그 설정
		for (int i = 0; i < polyMesh->npolys; ++i)
		{
			if (polyMesh->areas[i] == RC_WALKABLE_AREA)
			{
				polyMesh->areas[i] = 0; // GROUND
				polyMesh->flags[i] = 0x01; // WALK
			}
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = polyMesh->verts;
		params.vertCount = polyMesh->nverts;
		params.polys = polyMesh->polys;
		params.polyAreas = polyMesh->areas;
		params.polyFlags = polyMesh->flags;
		params.polyCount = polyMesh->npolys;
		params.nvp = polyMesh->nvp;
		params.detailMeshes = detailMesh->meshes;
		params.detailVerts = detailMesh->verts;
		params.detailVertsCount = detailMesh->nverts;
		params.detailTris = detailMesh->tris;
		params.detailTriCount = detailMesh->ntris;
		params.walkableHeight = agentHeight;
		params.walkableRadius = agentRadius;
		params.walkableClimb = agentMaxClimb;
		rcVcopy(params.bmin, polyMesh->bmin);
		rcVcopy(params.bmax, polyMesh->bmax);
		params.cs = config.cs;
		params.ch = config.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			return false;
		}

		navMesh = dtAllocNavMesh();
		if (!navMesh)
		{
			dtFree(navData);
			return false;
		}

		dtStatus status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			return false;
		}

		navQuery = dtAllocNavMeshQuery();
		status = navQuery->init(navMesh, 2048);
		if (dtStatusFailed(status))
		{
			return false;
		}
	}

	return true;
}

bool Navigation::FindPath(float* startPos, float* endPos, std::vector<float>& outPath)
{
	dtQueryFilter filter;
	filter.setIncludeFlags(0x01);
	filter.setExcludeFlags(0);

	float extents[3] = { 100.f, 100.f, 100.f }; // 시작/끝 점 근처 탐색 범위

	// 시작점 근처 폴리곤 찾기
	dtPolyRef startRef;
	float startNearest[3];
	navQuery->findNearestPoly(startPos, extents, &filter, &startRef, startNearest);

	// 끝점 근처 폴리곤 찾기
	dtPolyRef endRef;
	float endNearest[3];
	navQuery->findNearestPoly(endPos, extents, &filter, &endRef, endNearest);

	if (!startRef || !endRef)
	{
		return false;
	}

	// 경로 탐색
	dtPolyRef polys[256];
	int npolys = 0;
	navQuery->findPath(startRef, endRef, startNearest, endNearest, &filter, polys, &npolys, 256);

	if (npolys == 0)
	{
		return false;
	}

	// 실제 경로 좌표 추출
	float path[256 * 3];
	int npath = 0;
	navQuery->findStraightPath(startNearest, endNearest, polys, npolys, path, nullptr, nullptr, &npath, 256);

	for (int i = 0; i < npath; ++i)
	{
		outPath.push_back(path[i * 3 + 0]);
		outPath.push_back(path[i * 3 + 1]);
		outPath.push_back(path[i * 3 + 2]);
	}

	return true;
}
