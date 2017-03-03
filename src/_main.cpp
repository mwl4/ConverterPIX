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
		   " Supported formats: pmg(0x13), pmd(0x04), pma(0x03), ppd(0x15), pmc, tobj, mat\n"
		   "\n\n"
		   " This is Open-Source software under the GNU LGPL License.\n\n"
		   " Enjoy!\n"
	);
}

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

	std::vector<std::string> basepath;
	std::string exportpath;
	std::string filepath;

	enum {
		DIRECTORY_LIST,
		SINGLE_MODEL,
		SINGLE_TOBJ,
		DEBUG_DDS
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
			parameter = &filepath;
		}
		else
		{
			optionalArgs.push_back(arg);
		}
	}

	for (const auto &base : basepath)
	{
		static int priority = 1;
		bool noneedslash = (base[base.length() - 1] == '/') || (base[base.length() - 1] == '\\');
		ufsMount(noneedslash ? base : base + '/', true, priority++);
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
				exportpath = basepath[0] + "_exp";
			}
			backslashesToSlashes(filepath);
			auto model = std::make_shared<Model>();
			if (!model->load(filepath))
			{
				printf("Failed to load: %s\n", filepath.c_str());
				return 1;
			}
			model->saveToMidFormat(exportpath, true);
			for (size_t i = 0; i < optionalArgs.size(); ++i)
			{
				if (optionalArgs[i] == "*")
				{
					auto files = getUFS()->readDir(model->fileDirectory(), true, false);
					files->erase(std::remove_if(files->begin(), files->end(), [](const std::string &s) {
						return s.substr(s.rfind('.')) != ".pma";
					}), files->end()); // remove files with no .pma extension
					std::for_each(files->begin(), files->end(), [&](std::string &s) {
						s = s.substr(0, s.rfind('.'));
					}); // remove extensions
					optionalArgs.insert(optionalArgs.end(), files->begin(), files->end());
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
				basepath.push_back(optionalArgs[0]);
			}
			if (exportpath.empty())
			{
				exportpath = basepath[0] + "_exp";
			}
			auto files = getSFS()->readDir(basepath[0], true, false);
			for (const auto &f : *files)
			{
				const std::string filename = f.substr(basepath[0].length());
				const std::string extension = f.substr(f.rfind('.'));
				if (extension == ".pmg")
				{
					const std::string modelPath = filename.substr(0, filename.length() - 4);
					Model model;
					if (!model.load(modelPath))
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
					if (tobj.load(filename))
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
				exportpath = basepath[0] + "_exp";
			}
			backslashesToSlashes(filepath);
			TextureObject tobj;
			if (tobj.load(filepath))
			{
				tobj.saveToMidFormats(exportpath);
				printf("%s: tobj: yes\n", filepath.substr(directory(filepath).length() + 1).c_str());
			}
		} break;
		case DEBUG_DDS:
		{
			if (!filepath.empty())
			{
				dds::print_debug(filepath);
			}
		} break;
	}
	return 0;
}

/* eof */
