/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : _main.cpp
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#include <prerequisites.h>

#include <resource_lib.h>
#include <model/model.h>
#include <model/animation.h>
#include <texture/texture_object.h>
#include <texture/texture.h>

#include <structs/dds.h>
#include <fs/file.h>
#include <fs/sysfilesystem.h>
#include <fs/uberfilesystem.h>

#include <chrono>

void print_help()
{
	printf(" Parameters:\n"
		   "  -h                   - prints help text\n"
		   "  -m <model_path>      - turns into single model mode and specifies model path (relative to base)\n"
		   "  -t <tobj_path>       - turns into single tobj mode and specifies tobj path (relative to base)\n"
		   "  -d <dds_path>        - turns into single dds mode and prints debug info (absolute path)\n"
		   "  -b <base_path>       - specify base path\n"
		   "  -e <export_path>     - specify export path\n"
		   "\n"
		   " Usage:\n"
		   "  converter_pix -b C:\\ets2_base -m /vehicle/truck/man_tgx/interior/anim s_wheel\n"
		   "    ^ will export into C:\\ets2_base_exp single model with s_wheel animation.\n"
		   "    ^ instead of exact animation name you can use * to convert every anim file from model directory.\n"
		   "    ^ when anim name is not started by /, then converter is looking for it in model directory.\n"
		   "\n"
		   "  converter_pix C:\\ets2_base\n"
		   "    ^ will convert whole base, it will export it into: <base_path>_exp (C:\\ets2_base_exp in this example).\n"
		   "    ^ you can also specify export path using the -e parameter.\n"
		   "\n"
		   "  converter_pix -b C:\\ets2_base -t /material/environment/vehicle_reflection.tobj\n"
		   "    ^ will convert tobj file and copy texture to export path.\n"
		   "\n"
		   "  converter_pix -b C:\\ets2_base -m /model/mover/characters/models/generic/m_afam_01 /model/mover/characters/animations/man/walk/walk_01\n"
		   "    ^ animations are located in another directory than the models. These animations can be used for multiple models.\n"
		   "\n"
		   " Note that animations will not be converted when converting the whole base.\n"
		   " This is caused by lack of information, so you have to convert each model individually to edit animations.\n"
		   "\n"
		   " Supported formats: pmg(0x13, 0x14), pmd(0x04), pma(0x03), ppd(0x15), pmc(0x06), tobj, mat\n"
		   "\n\n"
		   " This is Open-Source software under the GNU LGPL License.\n\n"
		   " Enjoy!\n"
	);
}

bool convertSingleModel(String filepath, String exportpath, Array<String> optionalArgs);
bool convertWholeBase(String basepath, String exportpath);

int main(int argc, char *argv[])
{
	printf("\n"
		   " ******************************************\n"
		   " **        Converter PMX to PIX          **\n"
		   " **       Copyright (C) 2017 mwl4        **\n"
		   " ******************************************\n"
		   "\n"
	);

	if (argc < 2)
	{
		printf("Not enough parameters.\n");
		print_help();
		return 1;
	}

	auto resLib = std::make_unique<ResourceLibrary>();

	Array<String> basepath;
	String exportpath;
	String path;
	bool listdir_r = false;

	enum {
		DIRECTORY_LIST,
		SINGLE_MODEL,
		SINGLE_TOBJ,
		DEBUG_DDS,
		EXTRACT_FILE,
		EXTRACT_DIRECTORY,
		LIST_DIR,
	} mode = DIRECTORY_LIST;

	String *parameter = nullptr;
	Array<String> optionalArgs;

	for (int i = 1; i < argc; ++i)
	{
		String arg = argv[i];
		if (parameter)
		{
			*parameter = arg;
			parameter = nullptr;
			continue;
		}
		if (arg == "-h")
		{
			print_help();
			return 0;
		}
		else if (arg == "-m")
		{
			mode = SINGLE_MODEL;
			parameter = &path;
		}
		else if (arg == "-t")
		{
			mode = SINGLE_TOBJ;
			parameter = &path;
		}
		else if (arg == "-b")
		{
			basepath.push_back("");
			parameter = &basepath.back();
		}
		else if (arg == "-e")
		{
			parameter = &exportpath;
		}
		else if (arg == "-d")
		{
			mode = DEBUG_DDS;
			parameter = &path;
		}
		else if (arg == "-extract_f")
		{
			mode = EXTRACT_FILE;
			parameter = &path;
		}
		else if (arg == "-extract_d")
		{
			mode = EXTRACT_DIRECTORY;
			parameter = &path;
		}
		else if (arg == "-listdir")
		{
			mode = LIST_DIR;
			parameter = &path;
		}
		else if (arg == "-listdir_r")
		{
			mode = LIST_DIR;
			listdir_r = true;
			parameter = &path;
		}
		else
		{
			optionalArgs.push_back(arg);
		}
	}

	for (const auto &base : basepath)
	{
		static int priority = 1;
		ufsMount(base, true, priority++);
	}

	long long startTime =
		std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();

	switch (mode)
	{
		case SINGLE_MODEL:
		{
			if (basepath.empty())
			{
				error("system", "", "Not specified base path!");
				return 1;
			}
			if (exportpath.empty())
			{
				exportpath = basepath.back() + "_exp";
			}
			convertSingleModel(path, exportpath, optionalArgs);
		} break;
		case DIRECTORY_LIST:
		{
			if (optionalArgs.size() == 0)
			{
				error("system", "", "Invalid parameters!");
				return 1;
			}
			if (basepath.empty())
			{
				basepath.push_back(optionalArgs[0]);
				static int priority = 1;
				ufsMount(basepath[0], true, priority++);
			}
			if (exportpath.empty())
			{
				exportpath = basepath[0] + "_exp";
			}
			convertWholeBase(basepath[0], exportpath);
		} break;
		case SINGLE_TOBJ:
		{
			if (basepath.empty())
			{
				error("system", "", "Not specified base path!");
				return 1;
			}
			if (exportpath.empty())
			{
				exportpath = basepath[0] + "_exp";
			}
			backslashesToSlashes(path);
			TextureObject tobj;
			if (tobj.load(path))
			{
				tobj.saveToMidFormats(exportpath);
				printf("%s: tobj: yes\n", path.substr(directory(path).length() + 1).c_str());
			}
		} break;
		case DEBUG_DDS:
		{
			if (!path.empty())
			{
				dds::print_debug(path);
			}
		} break;
		case EXTRACT_FILE:
		{
			if (basepath.empty())
			{
				error("system", "", "Not specified base path!");
				return 1;
			}
			if (exportpath.empty())
			{
				exportpath = basepath[0] + "_exp";
			}
			auto file = getUFS()->open(path, FileSystem::read | FileSystem::binary);
			if (file)
			{
				auto output = getSFS()->open(exportpath + path, FileSystem::write | FileSystem::binary);
				if (output)
				{
					copyFile(file.get(), output.get());
				}
				else
				{
					printf("Unable to open file to write: %s\n", (exportpath + path).c_str());
				}
			}
			else
			{
				printf("Unable to open file to read: %s\n", path.c_str());
			}
		} break;
		case EXTRACT_DIRECTORY:
		{
			if (basepath.empty())
			{
				error("system", "", "Not specified base path!");
				return 1;
			}
			if (exportpath.empty())
			{
				exportpath = basepath[0] + "_exp";
			}
			auto files = getUFS()->readDir(path, true, true);
			if (!files)
			{
				error("system", "", "readDir returned null!");
				return 1;
			}
			for (const auto &f : *files)
			{
				if (f.IsDirectory())
				{
					continue;
				}

				auto file = getUFS()->open(f.GetPath(), FileSystem::read | FileSystem::binary);
				if (file)
				{
					auto output = getSFS()->open(exportpath + f.GetPath(), FileSystem::write | FileSystem::binary);
					if (output)
					{
						copyFile(file.get(), output.get());
					}
					else
					{
						printf("Unable to open file to write: %s\n", (exportpath + f.GetPath()).c_str());
					}
				}
				else
				{
					printf("Unable to open file to read: %s\n", f.GetPath().c_str());
				}
			}
		} break;
		case LIST_DIR:
		{
			if (basepath.empty())
			{
				error("system", "", "Not specified base path!");
				return 1;
			}

			auto files = getUFS()->readDir(path, true, listdir_r);
			if (!files)
			{
				error("system", "", "readDir returned null!");
				return 1;
			}
			for (const auto &f : *files)
			{
				printf("[%s]%s %s\n", f.IsDirectory() ? "D" : "F", f.IsEncrypted() ? " (encrypted)" : "", f.GetPath().c_str());
			}

			printf("-- done --\n");
		} break;
	}

	long long endTime =
		std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();

	//printf("Time : %llums\n", endTime - startTime);

	return 0;
}

bool convertSingleModel(String filepath, String exportpath, Array<String> optionalArgs)
{
	backslashesToSlashes(filepath);
	auto model = std::make_shared<Model>();
	if (!model->load(filepath))
	{
		printf("Failed to load: %s\n", filepath.c_str());
		return false;
	}
	model->saveToMidFormat(exportpath, true);
	for (size_t i = 0; i < optionalArgs.size(); ++i)
	{
		if (optionalArgs[i] == "*")
		{
			auto files = getUFS()->readDir(model->fileDirectory(), true, false);

			// remove files with no .pma extension
			files->erase(
				std::remove_if(files->begin(), files->end(),
					[](const FileSystem::Entry &s) {
						return s.IsDirectory() || s.GetPath().substr(s.GetPath().rfind('.')) != ".pma";
					}
				), files->end()
			);

			// remove extensions
			std::for_each(files->begin(), files->end(),
				[&](FileSystem::Entry &s) {
					s.SetPath(s.GetPath().substr(0, s.GetPath().rfind('.')));
				}
			);

			for (const auto &f : (*files))
			{
				optionalArgs.push_back(f.GetPath());
			}
			continue;
		}
		backslashesToSlashes(optionalArgs[i]);
		Animation anim;
		if (!anim.load(model, optionalArgs[i]))
		{
			printf("Failed to load: %s\n", optionalArgs[i].c_str());
		}
		else
		{
			anim.saveToPia(exportpath);
		}
	}
	return true;
}

bool convertWholeBase(String basepath, String exportpath)
{
	auto files = getSFS()->readDir(basepath, true, true);
	if (!files)
	{
		printf("No files to convert!\n");
		return false;
	}

	int size = 0;
	for (const auto &f : *files)
	{
		if (f.IsDirectory())
			continue;

		const String extension = f.GetPath().substr(f.GetPath().rfind('.'));
		if (extension == ".pmg" || extension == ".tobj")
		{
			++size;
		}
	}

	int i = 0;
	for (const auto &f : *files)
	{
		if (f.IsDirectory())
			continue;

		const String filename = f.GetPath().substr(basepath.length());
		const String extension = f.GetPath().substr(f.GetPath().rfind('.'));
		if (extension == ".pmg")
		{
			const String modelPath = filename.substr(0, filename.length() - 4);
			Model model;
			if (!model.load(modelPath))
			{
				printf("Failed to load: %s\n", modelPath.c_str());
			}
			else
			{
				printf("[%u/%u = %u%%]: ", i, size, (unsigned)(100.f * i / size));
				model.saveToMidFormat(exportpath, false);
			}
			++i;
		}
		else if (extension == ".tobj")
		{
			printf("[%u/%u = %u%%]: ", i, size, (unsigned)(100.f * i / size));
			printf("%s: tobj: ", filename.substr(directory(filename).length() + 1).c_str());

			TextureObject tobj;
			if (tobj.load(filename))
			{
				tobj.saveToMidFormats(exportpath);
				printf("ok\n");
			}
			++i;
		}
	}
	printf("\nBase converted: %s\n", exportpath.c_str());
	return false;
}

/* eof */
