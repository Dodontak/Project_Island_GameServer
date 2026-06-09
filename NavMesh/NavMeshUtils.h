#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "RecastDump.h"

#include <array>
#include <string>
#include <vector>

/// stdio file implementation.
class FileIO final : public duFileIO
{
public:
	FileIO() = default;
	FileIO(const FileIO&) = delete;
	FileIO& operator=(const FileIO&) = delete;
	FileIO(FileIO&&) = default;
	FileIO& operator=(FileIO&&) = default;
	~FileIO() override;

	bool openForWrite(const char* path);
	bool openForRead(const char* path);
	[[nodiscard]] bool isWriting() const override;
	[[nodiscard]] bool isReading() const override;
	bool write(const void* ptr, size_t size) override;
	bool read(void* ptr, size_t size) override;
	size_t getFileSize() const;

	static void scanDirectory(const std::string& path, const std::string& ext, std::vector<std::string>& fileList);
private:
	FILE* fp = nullptr;
	enum class Mode { none, reading, writing };
	Mode mode = Mode::none;
};