/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : file.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

#include <prerequisites.h>

class IFile
{
public:
	IFile() { }
	virtual ~IFile() { }

	virtual bool open(const std::string &file, const char *attr) = 0;
	virtual void close() = 0;

	virtual IFile& write(const void *buffer, size_t elementSize, size_t size) = 0;
	virtual IFile& operator<<(int) = 0;
	virtual IFile& operator<<(const char*) = 0;
	virtual IFile& operator<<(std::string const &) = 0;

	virtual IFile& read(char *buffer, size_t elementSize, size_t size) = 0;
	virtual IFile& operator>>(int&) = 0;

	virtual size_t getSize() const = 0;
};

class FileStd : public IFile
{
private:
	FILE * m_file;
	size_t m_size;
public:
	FileStd();
	FileStd(const std::string &file, const char *attr);
	~FileStd();
	
	bool open(const std::string &file, const char *attr) override;
	void close() override;
	
	IFile& write(const void *buffer, size_t elementSize, size_t size) override;
	IFile& operator<<(int) override;
	IFile& operator<<(const char*) override;
	IFile& operator<<(std::string const&) override;

	IFile& read(char *buffer, size_t elementSize, size_t size);
	IFile& operator>>(int&);

	const char *getLine(std::string &out)
	{
		char buffer[1024] = { 0 };
		const char *result = fgets(buffer, sizeof(buffer), m_file);
		if (result)
		{
			out = buffer;
			return out.c_str();
		}
		out = "";
		return nullptr;
	}

	enum attrib
	{
		SeekCur = SEEK_CUR,
		SeekSet = SEEK_SET,
		SeekEnd = SEEK_END,
	};

	int seek(uint32_t offset, attrib attr);
	void rewind();
	size_t tell();

	size_t getSize() const { return m_size; }

private:
	void updateSize();
};

typedef FileStd File;

/* eof */
