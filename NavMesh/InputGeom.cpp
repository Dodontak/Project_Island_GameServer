#include "InputGeom.h"
#include "NavMeshUtils.h"
#include "Recast.h"

#include <fstream>
#include <sstream>

namespace
{
	bool intersectSegmentTriangle(const float* sp, const float* sq, const float* a, const float* b, const float* c, float& t);
	bool isectSegAABB(const float* sp, const float* sq, const float* amin, const float* amax, float& tmin, float& tmax);
	char* parseRow(char* buf, char* bufEnd, char* row, int len);
	char* readRow(char* buf, char* bufEnd, char* row, int len);
	int readFace(char* row, int* data, int maxDataLen, int vertCount);
}

bool InputGeom::LoadMesh(rcContext* ctx, const std::string& filepath)
{
	FileIO file;
	if (!file.openForRead(filepath.c_str()))
	{
		ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not load '%s'", filepath.c_str());
		return false;
	}

	size_t bufferLen = file.getFileSize();
	char* buffer = new char[bufferLen];

	if (!file.read(buffer, bufferLen))
	{
		ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not load '%s'", filepath.c_str());
		return false;
	}

	filename = filepath;
	clearOffMeshConnections();

	convexVolumes.clear();

	mesh.reset();
	mesh.readFromObj(buffer, bufferLen);
	rcCalcBounds(mesh.verts.data(), mesh.getVertCount(), meshBoundsMin, meshBoundsMax);

	partitionedMesh = {}; // Reset the partitioned mesh
	partitionedMesh.PartitionMesh(mesh.verts.data(), mesh.tris.data(), mesh.getTriCount(), 256);
	return true;
}

void Mesh::readFromObj(char* buf, size_t bufLen)
{
	char* src = buf;
	char* srcEnd = buf + bufLen;
	char row[512];
	int face[32];
	float x, y, z;
	int numVertices;

	while (src < srcEnd)
	{
		// Parse one row
		row[0] = '\0';
		src = readRow(src, srcEnd, row, sizeof(row) / sizeof(row[0]));

		if (row[0] == '#')
		{
			// Comment
			continue;
		}
		if (row[0] == 'v' && row[1] != 'n' && row[1] != 't')
		{
			// Vertex pos
			sscanf(row + 1, "%f %f %f", &x, &y, &z);
			verts.push_back(x);
			verts.push_back(y);
			verts.push_back(z);
		}
		if (row[0] == 'f')
		{
			// Face
			const int vertCount = static_cast<int>(verts.size()) / 3;
			numVertices = readFace(row + 1, face, sizeof(face) / sizeof(face[0]), vertCount);
			for (int i = 2; i < numVertices; ++i)
			{
				const int a = face[0];
				const int b = face[i - 1];
				const int c = face[i];
				if (a < 0 || a >= vertCount || b < 0 || b >= vertCount || c < 0 || c >= vertCount)
				{
					continue;
				}

				tris.push_back(a);
				tris.push_back(b);
				tris.push_back(c);
			}
		}
	}

	// Calculate face normals.
	normals.resize(tris.size());
	for (int i = 0; i < static_cast<int>(tris.size()); i += 3)
	{
		const float* vertex0 = &verts[tris[i + 0] * 3];
		const float* vertex1 = &verts[tris[i + 1] * 3];
		const float* vertex2 = &verts[tris[i + 2] * 3];

		// Construct two triangle edges
		float edge0[3];
		float edge1[3];
		for (int j = 0; j < 3; ++j)
		{
			edge0[j] = vertex1[j] - vertex0[j];
			edge1[j] = vertex2[j] - vertex0[j];
		}

		float* normal = &normals[i];

		// Cross product
		normal[0] = edge0[1] * edge1[2] - edge0[2] * edge1[1];
		normal[1] = edge0[2] * edge1[0] - edge0[0] * edge1[2];
		normal[2] = edge0[0] * edge1[1] - edge0[1] * edge1[0];

		// Normalize
		float normalLength = sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
		if (normalLength > 0)
		{
			normalLength = 1.0f / normalLength;
			normal[0] *= normalLength;
			normal[1] *= normalLength;
			normal[2] *= normalLength;
		}
	}
}

namespace
{
	bool intersectSegmentTriangle(const float* sp, const float* sq, const float* a, const float* b, const float* c, float& t)
	{
		float ab[3];
		rcVsub(ab, b, a);
		float ac[3];
		rcVsub(ac, c, a);
		float qp[3];
		rcVsub(qp, sp, sq);

		// Compute triangle normal. Can be precalculated or cached if
		// intersecting multiple segments against the same triangle
		float norm[3];
		rcVcross(norm, ab, ac);

		// Compute denominator d. If d <= 0, segment is parallel to or points
		// away from triangle, so exit early
		float d = rcVdot(qp, norm);
		if (d <= 0.0f)
		{
			return false;
		}

		// Compute intersection t value of pq with plane of triangle. A ray
		// intersects iff 0 <= t. Segment intersects iff 0 <= t <= 1. Delay
		// dividing by d until intersection has been found to pierce triangle
		float ap[3];
		rcVsub(ap, sp, a);
		t = rcVdot(ap, norm);
		if (t < 0.0f)
		{
			return false;
		}
		if (t > d)
		{
			return false;
		}  // For segment; exclude this code line for a ray test

		// Compute barycentric coordinate components and test if within bounds
		float e[3];
		rcVcross(e, qp, ap);
		float v = rcVdot(ac, e);
		if (v < 0.0f || v > d)
		{
			return false;
		}
		float w = -rcVdot(ab, e);
		if (w < 0.0f || v + w > d)
		{
			return false;
		}

		// Segment/ray intersects triangle. Perform delayed division
		t /= d;

		return true;
	}

	bool isectSegAABB(const float* sp, const float* sq, const float* amin, const float* amax, float& tmin, float& tmax)
	{
		static constexpr float EPS = 1e-6f;

		float d[3];
		rcVsub(d, sq, sp);
		tmin = 0.0;
		tmax = 1.0f;

		for (int i = 0; i < 3; i++)
		{
			if (fabsf(d[i]) < EPS)
			{
				if (sp[i] < amin[i] || sp[i] > amax[i])
				{
					return false;
				}
			}
			else
			{
				const float ood = 1.0f / d[i];
				float t1 = (amin[i] - sp[i]) * ood;
				float t2 = (amax[i] - sp[i]) * ood;
				if (t1 > t2)
				{
					float tmp = t1;
					t1 = t2;
					t2 = tmp;
				}
				tmin = std::max(t1, tmin);
				tmax = std::min(t2, tmax);
				if (tmin > tmax)
				{
					return false;
				}
			}
		}

		return true;
	}

	char* parseRow(char* buf, char* bufEnd, char* row, int len)
	{
		bool start = true;
		bool done = false;
		int n = 0;
		while (!done && buf < bufEnd)
		{
			char c = *buf;
			buf++;
			// multirow
			switch (c)
			{
			case '\n':
				if (start)
				{
					break;
				}
				done = true;
				break;
			case '\r':
				break;
			case '\t':
			case ' ':
				if (start)
				{
					break;
				}
				// else falls through
			default:
				start = false;
				row[n++] = c;
				if (n >= len - 1)
				{
					done = true;
				}
				break;
			}
		}
		row[n] = '\0';
		return buf;
	}
	
	char* readRow(char* buf, char* bufEnd, char* row, int len)
	{
		// skip leading whitespace
		for (; buf < bufEnd; ++buf)
		{
			char c = *buf;
			if (c != '\\' && c != '\r' && c != '\n' && c != '\t' && c != ' ')
			{
				break;
			}
		}

		int n = 0;
		for (; buf < bufEnd; ++buf)
		{
			char c = *buf;

			if (c == '\n')
			{
				break;
			}

			if (c == '\\' || c == '\r')
			{
				// skip
				continue;
			}

			// Copy character
			row[n++] = c;
			if (n >= len - 1)
			{
				break;
			}
		}
		row[n] = '\0';
		return buf;
	}
	
	int readFace(char* row, int* data, int maxDataLen, int vertCount)
	{
		int numVertices = 0;
		while (*row != '\0')
		{
			// Skip initial white space
			while (*row != '\0' && (*row == ' ' || *row == '\t'))
			{
				row++;
			}

			char* s = row;
			// Find vertex delimiter and terminate the string there for conversion.
			while (*row != '\0' && *row != ' ' && *row != '\t')
			{
				if (*row == '/')
				{
					*row = '\0';
				}
				row++;
			}

			if (*s == '\0')
			{
				continue;
			}

			int vertexIndex = atoi(s);
			data[numVertices++] = vertexIndex < 0 ? vertexIndex + vertCount : vertexIndex - 1;
			if (numVertices >= maxDataLen)
			{
				break;
			}
		}
		return numVertices;
	}
}