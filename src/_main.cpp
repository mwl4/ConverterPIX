/*********************************************************************
 *           Copyright (C) 2016 mwl4 - All rights reserved           *
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

void print_help()
{
	printf(" Parameters:\n");
	printf("  -h                   - prints help text\n");
	printf("  -m <model_path>      - turns into single model mode and specifies model path (relative to base)\n");
	printf("  -t <tobj_path>       - turns into single tobj mode and specifies tobj path (relative to base)\n");
	printf("  -b <base_path>       - specify base path\n");
	printf("  -e <export_path>     - specify export path\n");
	printf("\n");
	printf(" Usage:\n");
	printf("  converter_pix -b C:\\ets2_base -m /vehicle/truck/man_tgx/interior/anim s_wheel\n");
	printf("    ^ will export into C:\\ets2_base_exp single model with s_wheel animation.\n");
	printf("    ^ instead of exact animation name you can use * to convert every anim file from model directory.\n");
	printf("    ^ when anim name is not started by /, then converter is looking for it in model directory.\n");
	printf("\n");
	printf("  converter_pix C:\\ets2_base\n");
	printf("    ^ will convert whole base, it will export it into: <base_path>_exp (C:\\ets2_base_exp in this example).\n");
	printf("    ^ you can also specify export path using the -e parameter.\n");
	printf("\n");
	printf("  converter_pix -b C:\\ets2_base -t /material/environment/vehicle_reflection.tobj\n");
	printf("    ^ will convert tobj file and copy texture to export path.\n");
	printf("\n");
	printf("  converter_pix -b C:\\ets2_base -m /model/mover/characters/models/generic/m_afam_01 /model/mover/characters/animations/man/walk/walk_01\n");
	printf("    ^ animations are located in another directory than the models. These animations can be used for multiple models.\n");
	printf("\n");
	printf(" Note that animations will not be converted when converting the whole base.\n");
	printf(" This is caused by lack of information, so you have to convert each model individually to edit animations.\n");
	printf("\n");
	printf(" Supported formats: pmg(0x13), pmd(0x04), pma(0x03), ppd(0x15), tobj, mat\n");
	printf(" TODO: pmc format support, multi base paths\n");
	printf("\n\n");
	printf(" This is Open-Source software under the GNU LGPL License.\n\n");
	printf(" Enjoy!\n");
}

int main(int argc, char *argv[])
{
	printf("\n");
	printf(" ******************************************\n");
	printf(" **        Converter PMX to PIX          **\n");
	printf(" **       Copyright (C) 2016 mwl4        **\n");
	printf(" ******************************************\n");
	printf("\n");

	if (argc < 2)
	{
		printf("Not enough parameters.\n");
		print_help();
		return 1;
	}

	auto resLib = std::make_unique<ResourceLibrary>();

	std::string basepath;
	std::string exportpath;
	std::string filepath;

	enum {
		DIRECTORY_LIST,
		SINGLE_MODEL,
		SINGLE_TOBJ,
	} mode = DIRECTORY_LIST;
	
	std::string *parameter = nullptr;
	std::vector<std::string> optionalArgs;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
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
			parameter = &filepath;
		}
		else if (arg == "-t")
		{
			mode = SINGLE_TOBJ;
			parameter = &filepath;
		}
		else if (arg == "-b")
		{
			parameter = &basepath;
		}
		else if (arg == "-e")
		{
			parameter = &exportpath;
		}
		else
		{
			optionalArgs.push_back(arg);
		}
	}

	switch (mode)
	{
		case SINGLE_MODEL:
		{
			if (basepath.empty())
			{
				printf("Not specified base path!");
				return 1;
			}
			if (exportpath.empty())
			{
				exportpath = basepath + "_exp";
			}
			std::replace(filepath.begin(), filepath.end(), '\\', '/'); // backslashes to slashes
			auto model = std::make_shared<Model>();
			if (!model->load(basepath, filepath))
			{
				printf("Failed to load: %s\n", filepath.c_str());
				return 1;
			}
			model->saveToMidFormat(exportpath, true);
			for (size_t i = 0; i < optionalArgs.size(); ++i)
			{
				if (optionalArgs[i] == "*")
				{
					auto files = listFilesInDirectory(model->basePath() + model->fileDirectory(), false);
					files.erase(std::remove_if(files.begin(), files.end(), [](const std::string &s) {
						return s.substr(s.rfind('.')) != ".pma";
					}), files.end()); // remove files with no .pma extension
					std::for_each(files.begin(), files.end(), [&](std::string &s) {
						s = s.substr(model->basePath().length(), s.rfind('.') - model->basePath().length());
					}); // remove extensions
					optionalArgs.insert(optionalArgs.end(), files.begin(), files.end());
					continue;
				}
				std::replace(optionalArgs[i].begin(), optionalArgs[i].end(), '\\', '/');
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
		} break;
		case DIRECTORY_LIST:
		{
			if (optionalArgs.size() == 0)
			{
				printf("Invalid parameters\n");
				return 1;
			}
			if (basepath.empty())
			{
				basepath = optionalArgs[0];
			}
			if (exportpath.empty())
			{
				exportpath = basepath + "_exp";
			}
			auto files = listFilesInDirectory(basepath);
			for (const auto &f : files)
			{
				const std::string filename = f.substr(basepath.length());
				const std::string extension = f.substr(f.rfind('.'));
				if (extension == ".pmg")
				{
					const std::string modelPath = filename.substr(0, filename.length() - 4);
					Model model;
					if (!model.load(basepath, modelPath))
					{
						printf("Failed to load: %s\n", modelPath.c_str());
					}
					else
					{
						model.saveToMidFormat(exportpath, false);
					}
				}
				else if (extension == ".tobj")
				{
					TextureObject tobj;
					if (tobj.load(basepath, filename))
					{
						tobj.saveToMidFormats(exportpath);
						printf("%s: tobj: yes\n", filename.substr(directory(filename).length() + 1).c_str());
					}
				}
			}
			printf("\nBase converted: %s\n", exportpath.c_str());
		} break;
		case SINGLE_TOBJ:
		{
			if (basepath.empty())
			{
				printf("Not specified base path!");
				return 1;
			}
			if (exportpath.empty())
			{
				exportpath = basepath + "_exp";
			}
			std::replace(filepath.begin(), filepath.end(), '\\', '/'); // backslashes to slashes
			TextureObject tobj;
			if (tobj.load(basepath, filepath))
			{
				tobj.saveToMidFormats(exportpath);
				printf("%s: tobj: yes\n", filepath.substr(directory(filepath).length() + 1).c_str());
			}
		} break;
	}
	return 0;
}

/* eof */
