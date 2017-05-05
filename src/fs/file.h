/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : file.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once

class File
{
public:
	enum Attrib
	{
		SeekSet = SEEK_SET,
		SeekCur = SEEK_CUR,
		SeekEnd = SEEK_END,
	};
public:
	File();
	File(const File&) = delete;
	File(File &&) = delete;
	virtual ~File();

	File &operator=(const File &) = delete;
	File &operator=(File &&) = delete;

	virtual size_t write(const void *buffer, size_t elementSize, size_t elementCount) = 0;
	virtual size_t read(void *buffer, size_t elementSize, size_t elementCount) = 0;
	virtual size_t size() const = 0;
	virtual bool seek(uint32_t offset, Attrib attr) = 0;
	virtual void rewind() = 0;
	virtual size_t tell() = 0;
	virtual void flush() = 0;

	bool blockRead(void *buffer, unsigned int offset, size_t size);

	File &operator<<(bool val);
	File &operator<<(short val);
	File &operator<<(unsigned short val);
	File &operator<<(int val);
	File &operator<<(unsigned int val);
	File &operator<<(long val);
	File &operator<<(unsigned long val);
	File &operator<<(long long val);
	File &operator<<(unsigned long long val);
	File &operator<<(float val);
	File &operator<<(double val);
	File &operator<<(long double val);

	File &operator>>(bool &val);
	File &operator>>(short &val);
	File &operator>>(unsigned short &val);
	File &operator>>(int &val);
	File &operator>>(unsigned int &val);
	File &operator>>(long &val);
	File &operator>>(unsigned long &val);
	File &operator>>(long long &val);
	File &operator>>(unsigned long long &val);
	File &operator>>(float &val);
	File &operator>>(double &val);
	File &operator>>(long double &val);
};

File &operator<<(File &fp, char c);
File &operator<<(File &fp, signed char c);
File &operator<<(File &fp, unsigned char c);

File &operator<<(File &fp, const char *s);
File &operator<<(File &fp, const signed char *s);
File &operator<<(File &fp, const unsigned char *s);
File &operator<<(File &fp, const String &s);

File &operator>>(File &fp, char &c);
File &operator>>(File &fp, signed char &c);
File &operator>>(File &fp, unsigned char &c);

File &operator>>(File &fp, char *s);
File &operator>>(File &fp, signed char *s);
File &operator>>(File &fp, unsigned char *s);

/* eof */
