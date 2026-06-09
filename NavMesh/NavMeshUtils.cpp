#include "NavMeshUtils.h"
#include <algorithm>
#include <io.h>

FileIO::~FileIO()
{
	if (fp)
	{
		fclose(fp);
	}
}

bool FileIO::openForWrite(const char* path)
{
	if (fp)
	{
		return false;
	}
	fp = fopen(path, "wb");
	if (!fp)
	{
		return false;
	}
	mode = Mode::writing;
	return true;
}

bool FileIO::openForRead(const char* path)
{
	if (fp)
	{
		return false;
	}
	fp = fopen(path, "rb");
	if (!fp)
	{
		return false;
	}
	mode = Mode::reading;
	return true;
}

bool FileIO::isWriting() const
{
	return mode == Mode::writing;
}

bool FileIO::isReading() const
{
	return mode == Mode::reading;
}

bool FileIO::write(const void* ptr, const size_t size)
{
	if (!fp || mode != Mode::writing)
	{
		return false;
	}
	fwrite(ptr, size, 1, fp);
	return true;
}

bool FileIO::read(void* ptr, const size_t size)
{
	if (!fp || mode != Mode::reading)
	{
		return false;
	}
	size_t readLen = fread(ptr, size, 1, fp);
	return readLen == 1;
}

size_t FileIO::getFileSize() const
{
	if (!fp || mode != Mode::reading)
	{
		return false;
	}
	const size_t currentPos = ftell(fp);
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		return 0;
	}
	const size_t size = ftell(fp);
	if (fseek(fp, 0, static_cast<int>(currentPos)) != 0)
	{
		return 0;
	}
	return size;
}

void FileIO::scanDirectory(const std::string& path, const std::string& ext, std::vector<std::string>& fileList)
{
	const std::string pathWithExt = path + "/*" + ext;

	_finddata_t dir;
	const intptr_t findHandle = _findfirst(pathWithExt.c_str(), &dir);
	if (findHandle == -1L)
	{
		return;
	}

	do
	{
		fileList.emplace_back(dir.name);
	} while (_findnext(findHandle, &dir) == 0);
	_findclose(findHandle);


	// Sort the list of files alphabetically.
	std::sort(fileList.begin(), fileList.end());
}