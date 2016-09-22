/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : file.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include "file.h"

FileStd::FileStd()
{
	m_file = 0;
	m_size = 0;
}

FileStd::~FileStd()
{
	close();
}

FileStd::FileStd(const std::string& filepath, const char *attr)
{
	open(filepath, attr);
}

bool FileStd::open(const std::string& filepath, const char *attr)
{
	if (m_file = fopen(filepath.c_str(), attr))
	{
		updateSize();
	}
	else
	{
		if (!directoryExists(directory(filepath)) && strchr(attr, 'w') != nullptr)
		{
			if (createDirectory(directory(filepath)))
			{
				open(filepath, attr);
			}
		}
	}
	return !!m_file;
}

void FileStd::close()
{
	if (m_file)
		fclose(m_file);
	m_file = 0;
	m_size = 0;
}

void FileStd::updateSize()
{
	size_t currentPos = tell();
	{
		seek(0, SeekEnd);
		m_size = tell();
		rewind();
	}
	seek(currentPos, SeekSet);
}

IFile& FileStd::write(const void *buffer, size_t elementSize, size_t size)
{
	fwrite(buffer, elementSize, size, m_file);
	return *this;
}

IFile& FileStd::operator<<(int value)
{
	return write(&value, sizeof(int), 1);
}

IFile& FileStd::operator<<(const char *value)
{
	return write(value, sizeof(char), strlen(value));
}

IFile& FileStd::operator<<(const std::string &value)
{
	return write(value.c_str(), sizeof(char), value.length());
}

IFile& FileStd::read(char* buffer, size_t elementSize, size_t size)
{
	fread(buffer, elementSize, size, m_file);
	return *this;
}

IFile& FileStd::operator>>(int& dest)
{
	return read((char *)&dest, sizeof(int), 1);
}

int FileStd::seek(uint32_t offset, attrib attr)
{
	return m_file ? fseek(m_file, offset, attr) : -1;
}

size_t FileStd::tell()
{
	return m_file ? ftell(m_file) : 0;
}

void FileStd::rewind()
{
	seek(0, SeekSet);
}

bool createDirectory(const std::string &dir)
{
	std::string dirr(dir.c_str());
	std::replace_if(dirr.begin(), dirr.end(), [](char ch)->bool { return ch == '\\'; }, '/');

	if (directoryExists(dirr.c_str())) {
		return true;
	}
	dirr += '/';
	for (size_t pos = dirr.find('/', dirr.find('/') + 1); pos != -1; pos = dirr.find('/', pos + 1))
	{
		if (!directoryExists(dirr.substr(0, pos).c_str()))
		{
		#ifdef _WIN32
			if (mkdir(dirr.substr(0, pos).c_str()) != 0)
				return false;
		#else
			if (mkdir(dirr.substr(0, pos).c_str(), 0775) != 0)
				return false;
		#endif
		}
	}
	return true;
}

std::string directory(const std::string &filepath)
{
	size_t slashpos = filepath.rfind('/');
	if (slashpos == -1) slashpos = 0;
	size_t backslackpos = filepath.rfind('\\');
	if (backslackpos == -1) backslackpos = 0;
	return filepath.substr(0, std::max(slashpos, backslackpos)).c_str();
}

std::string relativePath(const std::string &filepath, const std::string &directory)
{
	std::string dir = (directory + "/").c_str();
	std::string path = filepath.c_str();

	std::replace(dir.begin(), dir.end(), '\\', '/');
	std::replace(path.begin(), path.end(), '\\', '/');

	std::string result;

	size_t up = 0;
	size_t curpos = 0;
	size_t lastpos = 0;
	while ((curpos = dir.find('/', curpos + 1)) != std::string::npos)
	{
		if (up == 0)
		{
			if (dir.substr(0, curpos) != path.substr(0, curpos))
			{
				++up;
			}
			else
			{
				lastpos = path[curpos] == '/' ? curpos + 1 : curpos;
			}
		}
		else
		{
			++up;
		}
	}

	for (size_t i = 0; i < up; ++i)
	{
		result += "../";
	}
	result += path.substr(lastpos);
	return result.c_str();
}

bool directoryExists(const std::string &dirpath)
{
	struct stat buffer;
	return (stat(dirpath.c_str(), &buffer) == 0 && ((buffer.st_mode & S_IFDIR) != 0));
}

bool fileExists(const std::string &filepath)
{
	FILE *fp = fopen(filepath.c_str(), "rb");
	if (fp)
	{
		fclose(fp);
		return true;
	}
	return false;
}

bool copyFile(const char *input, const char *output)
{
	if (fileExists(output))
		return false;

	FILE *fr = fopen(input, "rb");
	if (fr)
	{
		File fw;
		if (fw.open(output, "wb"))
		{
			const int bufferSize = 1 * 1024 * 1024;
			uint8_t *buffer = new uint8_t[bufferSize];
			size_t readed = 0;

			while ((readed = fread(buffer, 1, bufferSize, fr)) != 0)
			{
				fw.write(buffer, 1, readed);
			}
			fclose(fr);
			fw.close();

			delete[] buffer;
			return true;
		}
		else
		{
			printf("Could not open file: \"%s\" to copy-write! %s\n", output, strerror(errno));
		}
	}
	else
	{
		printf("Could not open file: \"%s\" to copy-read! %s\n", input, strerror(errno));
	}
	return false;
}

/**
 * source: http://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
 */
std::vector<std::string> listFilesInDirectory(const std::string &directory, bool recursive)
{
	std::vector<std::string> result;
#ifdef _WIN32
	HANDLE dir;
	WIN32_FIND_DATA file_data;

	if ((dir = FindFirstFileA((directory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return result;
	do
	{
		const std::string file_name = file_data.cFileName;
		const std::string full_file_name = directory + "/" + file_name;

		if (file_name[0] == '.')
			continue;

		if ((file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			if (recursive)
			{
				auto subdir = listFilesInDirectory(full_file_name);
				result.insert(result.begin(), subdir.begin(), subdir.end());
			}
			continue;
		}
		result.push_back(full_file_name);
	} while (FindNextFileA(dir, &file_data));
	FindClose(dir);
#else
	DIR *dir;
	struct dirent *ent;
	struct stat st;

	dir = opendir(directory.c_str());
	while ((ent = readdir(dir)) != 0)
	{
		const std::string file_name = ent->d_name;
		const std::string full_file_name = directory + "/" + file_name;

		if (file_name[0] == '.')
			continue;

		if (stat(full_file_name.c_str(), &st) == -1)
			continue;

		if ((st.st_mode & S_IFDIR) != 0)
		{
			if (recursive)
			{
				auto subdir = listFilesInDirectory(full_file_name);
				result.insert(result.begin(), subdir.begin(), subdir.end());
			}
			continue;
		}
		result.push_back(full_file_name);
	}
	closedir(dir);
#endif
	return result;
}

/* eof */
