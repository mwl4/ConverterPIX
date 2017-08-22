/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : file.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include "file.h"

#include "uberfilesystem.h"
#include "sysfilesystem.h"

File::File()
{
}

File::~File()
{
}

File &File::operator<<(bool val)
{
	write(&val, sizeof(bool), 1);
	return *this;
}

File &File::operator<<(short val)
{
	write(&val, sizeof(short), 1);
	return *this;
}

File &File::operator<<(unsigned short val)
{
	write(&val, sizeof(unsigned short), 1);
	return *this;
}

File &File::operator<<(int val)
{
	write(&val, sizeof(int), 1);
	return *this;
}

File &File::operator<<(unsigned int val)
{
	write(&val, sizeof(unsigned int), 1);
	return *this;
}

File &File::operator<<(long val)
{
	write(&val, sizeof(long), 1);
	return *this;
}

File &File::operator<<(unsigned long val)
{
	write(&val, sizeof(unsigned long), 1);
	return *this;
}

File &File::operator<<(long long val)
{
	write(&val, sizeof(long long), 1);
	return *this;
}

File &File::operator<<(unsigned long long val)
{
	write(&val, sizeof(unsigned long long), 1);
	return *this;
}

File &File::operator<<(float val)
{
	write(&val, sizeof(float), 1);
	return *this;
}

File &File::operator<<(double val)
{
	write(&val, sizeof(double), 1);
	return *this;
}

File &File::operator<<(long double val)
{
	write(&val, sizeof(long double), 1);
	return *this;
}

File &operator<<(File &fp, char c)
{
	fp.write(&c, sizeof(char), 1);
	return fp;
}

File &operator<<(File &fp, signed char c)
{
	fp.write(&c, sizeof(signed char), 1);
	return fp;
}

File &operator<<(File &fp, unsigned char c)
{
	fp.write(&c, sizeof(unsigned char), 1);
	return fp;
}

File &operator<<(File &fp, const char *s)
{
	fp.write(s, sizeof(char), strlen(s));
	return fp;
}

File &operator<<(File &fp, const signed char *s)
{
	fp.write(s, sizeof(signed char), strlen((const char *)s));
	return fp;
}

File &operator<<(File &fp, const unsigned char *s)
{
	fp.write(s, sizeof(unsigned char), strlen((const char *)s));
	return fp;
}

File &operator<<(File &fp, const String &s)
{
	fp.write(s.c_str(), sizeof(char), s.length());
	return fp;
}

File &File::operator>>(bool &val)
{
	read(&val, sizeof(bool), 1);
	return *this;
}

File &File::operator>>(short &val)
{
	read(&val, sizeof(short), 1);
	return *this;
}

File &File::operator>>(unsigned short &val)
{
	read(&val, sizeof(unsigned short), 1);
	return *this;
}

File &File::operator>>(int &val)
{
	read(&val, sizeof(int), 1);
	return *this;
}

File &File::operator>>(unsigned int &val)
{
	read(&val, sizeof(unsigned int), 1);
	return *this;
}

File &File::operator>>(long &val)
{
	read(&val, sizeof(long), 1);
	return *this;
}

File &File::operator>>(unsigned long &val)
{
	read(&val, sizeof(unsigned long), 1);
	return *this;
}

File &File::operator>>(long long &val)
{
	read(&val, sizeof(long long), 1);
	return *this;
}

File &File::operator>>(unsigned long long &val)
{
	read(&val, sizeof(unsigned long long), 1);
	return *this;
}

File &File::operator>>(float &val)
{
	read(&val, sizeof(float), 1);
	return *this;
}

File &File::operator>>(double &val)
{
	read(&val, sizeof(double), 1);
	return *this;
}

File &File::operator>>(long double &val)
{
	read(&val, sizeof(long double), 1);
	return *this;
}

File &operator>>(File &fp, char &c)
{
	fp.read(&c, sizeof(char), 1);
	return fp;
}

File &operator>>(File &fp, signed char &c)
{
	fp.read(&c, sizeof(signed char), 1);
	return fp;
}

File &operator>>(File &fp, unsigned char &c)
{
	fp.read(&c, sizeof(unsigned char), 1);
	return fp;
}

File &operator>>(File &fp, char *s)
{
	/* TODO */
	return fp;
}

File &operator>>(File &fp, signed char *s)
{
	/* TODO */
	return fp;
}

File &operator>>(File &fp, unsigned char *s)
{
	/* TODO */
	return fp;
}

bool File::blockRead(void *buffer, unsigned int offset, size_t size)
{
	if (tell() != offset)
	{
		if (!seek(offset, Attrib::SeekSet))
		{
			return false;
		}
	}
	return read(buffer, 1, size) == size;
}

bool copyFile(File *const input, File *const output)
{
	input->rewind();
	size_t toCopy = input->size();
	const size_t bufferSize = 1 * 1024 * 1024;
	uint8_t *buffer = new uint8_t[bufferSize];
	for (size_t readed = 0; (readed = input->read((char *)buffer, 1, std::min(bufferSize, toCopy))) != 0; toCopy -= readed)
	{
		output->write(buffer, 1, readed);
	}
	delete[] buffer;
	return true;
}

/* eof */
