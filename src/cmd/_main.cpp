/******************************************************************************
 *
 *  Project:	ConverterPIX @ CMD Application
 *  File:		/cmd/_main.cpp
 *
 *		  _____                          _            _____ _______   __
 *		 / ____|                        | |          |  __ \_   _\ \ / /
 *		| |     ___  _ ____   _____ _ __| |_ ___ _ __| |__) || |  \ V /
 *		| |    / _ \| '_ \ \ / / _ \ '__| __/ _ \ '__|  ___/ | |   > <
 *		| |___| (_) | | | \ V /  __/ |  | ||  __/ |  | |    _| |_ / . \
 *		 \_____\___/|_| |_|\_/ \___|_|   \__\___|_|  |_|   |_____/_/ \_\
 *
 *
 *  Copyright (C) 2017 Michal Wojtowicz.
 *  All rights reserved.
 *
 *   This software is ditributed WITHOUT ANY WARRANTY; without even
 *   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE. See the copyright file for more information.
 *
 *****************************************************************************/

#include <prerequisites.h>

#include <resource_lib.h>
#include <model/model.h>
#include <model/animation.h>
#include <texture/texture_object.h>
#include <texture/texture.h>
#include <utils/string_utils.h>
#include <structs/dds.h>
#include <fs/file.h>
#include <fs/sysfilesystem.h>
#include <fs/uberfilesystem.h>
#include <structs/pmg_0x15.h>
#include <structs/pma_0x05.h>

#include <chrono>

void print_help()
{
	printf(" Parameters:\n"
		   "  -h, --help  prints help text\n"
		   "\n"
		   "  <base_path>\n"
		   "              mode: converts entire base(directory or archive) to mid-formats\n"
		   "\n"
		   "  -b, --base <base_path>\n"
		   "              specify base path and mount it (can be used multiple times)\n"
		   "\n"
		   "  -e, --export <export_path>\n"
		   "              specifies export path\n"
		   "\n"
		   "  -m, --model <model_path> [<anim_path_1> <anim_path_2> <anim_path_3> ...]\n"
		   "              mode: convert single model to mid-formats (model_path is relative to base, without extension)\n"
		   "\n"
		   "  --find-model-animations <model_path>\n"
		   "              mode: finds and prints all matching animations file path to model (model_path is relative to base, without extension)\n"
		   "\n"
		   "  -t, --tobj <tobj_path>\n"
		   "              mode: convert single tobj to mid-formats (tobj_path is relative to base)\n"
		   "\n"
		   "  -d, --debug-dds <dds_path>\n"
		   "              mode: print debug info about given DDS file (dds_path is absolute path)\n"
		   "\n"
		   "  --extract-file <file_path>\n"
		   "              mode: extracts given file and saves it in export path (file_path is relative to base)\n"
		   "\n"
		   "  --show-file <file_path>\n"
		   "              mode: extracts given file and prints it to stdout (file_path is relative to base)\n"
		   "\n"
		   "  --extract-directory <dir_path>\n"
		   "              mode: extracts given directory and saves it in export path (dir_path is relative to base)\n"
		   "\n"
		   "  --list-directory <dir_path>\n"
		   "              mode: lists entries of given directory (dir_path is relative to base)\n"
		   "\n"
		   "  --list-directory-recursive <dir_path>\n"
		   "              mode: lists entries of given directory recursively (dir_path is relative to base)\n"
		   "\n"
		   "  --calc-cityhash64 <string>\n"
		   "              mode: calculate and print to stdout cityhash64 of given string\n"
		   "\n"
		   "  --calc-cityhash64-file <file_path>\n"
		   "              mode: calculate and print to stdout cityhash64 of given file (file_path is absolute path)\n"
		   "\n"
		   "  --output-material-format147\n"
		   "              switch: output materials in 1.47 mid-format\n"
		   "\n"
		   "  --output-dds-dxt10\n"
		   "              switch: output DDS files in DXT10 format\n"
		   "\n"
		   " Usage:\n"
		   "\n"
		   "  converter_pix -b C:\\ets2_base -m /vehicle/truck/man_tgx/interior/anim s_wheel\n"
		   "    ^ export into C:\\ets2_base_exp single model with s_wheel animation.\n"
		   "    ^ instead of exact animation name you can use * to convert every anim file from model directory.\n"
		   "    ^ when anim name is not started by /, then converter is looking for it in model directory.\n"
		   "\n"
		   "  converter_pix C:\\ets2_base\n"
		   "    ^ will convert whole base, it will export it into: <base_path>_exp (C:\\ets2_base_exp in this example).\n"
		   "    ^ you can also specify export path using the -e parameter.\n"
		   "\n"
		   "  converter_pix C:\\base.scs\n"
		   "    ^ will convert whole base, it will export it into: <base_path>_exp (C:\\base.scs_exp in this example).\n"
		   "\n"
		   "  converter_pix -b C:\\base.scs -b C:\\base_share.scs C:\\base_vehicle.scs\n"
		   "    ^ will convert whole base(base_vehicle.scs). It will export it into <base_path>_exp (C:\\base_vehicle.scs_exp in this example).\n"
		   "    ^ mounting other archives might be important as files inside base_vehicle.scs may depend on files in other archives.\n"
		   "\n"
		   "  converter_pix -b C:\\ets2_base -t /material/environment/vehicle_reflection.tobj\n"
		   "    ^ will convert tobj file and copy texture to export path.\n"
		   "\n"
		   "  converter_pix -b C:\\ets2_base -m /model/mover/characters/models/generic/m_afam_01 /model/mover/characters/animations/man/walk/walk_01\n"
		   "    ^ animations are located in another directory than the models. These animations can be used for multiple models.\n"
		   "\n"
		   "  converter_pix -b C:\\ets2_base -b C:\\mod_base -m /vehicle/truck/man_tgx/interior/model\n"
		   "    ^ multiple base paths (you can specify more paths in the same way)\n"
		   "\n"
		   "  converter_pix -b C:\\base.scs -extract_f /material/environment/vehicle_reflection.tobj\n"
		   "    ^ will extract tobj file into <base_path>_exp (if path is not set by -e parameter). C:\\base.scs_exp/material/environment/vehicle_reflection.tobj in this example.\n"
		   "\n"
		   "  converter_pix -b C:\\base.scs -extract_d /material\n"
		   "    ^ will extract entire directory into <base_path>_exp (if path is not set by -e parameter). C:\\base.scs_exp/material in this example.\n"
		   "\n"
		   "  converter_pix -b C:\\base.scs -listdir /material\n"
		   "    ^ will show entries (files and subdirectories) of /material directory\n"
		   "\n"
		   "  converter_pix -b C:\\def.scs -show_f /def/ui.sii\n"
		   "    ^ will show content of /def/ui.sii file stored in def.scs archive\n"
		   "\n"
		   " Note that animations will not be converted when converting the whole base.\n"
		   " This is caused by lack of information, so you have to convert each model individually to edit animations.\n"
		   "\n"
		   " Supported formats: pmg(0x13, 0x14, 0x15), pmd(0x04), pma(0x03, 0x04, 0x05), ppd(0x15, 0x16, 0x17), pmc(0x06), tobj, mat,\n"
		   "                    scs(hashfs, v1, v2, hash: CITY), zip(uncompressed & compressed)\n"
		   "\n\n"
		   " This is Open-Source software under the GNU LGPL License.\n\n"
		   " Enjoy!\n"
	);
}

bool convertSingleModel(String filepath, String exportpath, Array<String> optionalArgs);
bool convertWholeBase(FileSystem *fs, String exportpath);
bool printMatchingAnimations( String modelFilePath );

int main(int argc, char *argv[])
{
	printf("\n"
		   " ******************************************\n"
		   " **        Converter PMX to PIX          **\n"
		   " **       Copyright (C) 2024 mwl4        **\n"
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
	bool showElapsedTime = false;

	enum {
		WHOLE_BASE,
		SINGLE_MODEL,
		SINGLE_TOBJ,
		DEBUG_DDS,
		SHOW_FILE,
		EXTRACT_FILE,
		EXTRACT_DIRECTORY,
		LIST_DIR,
		CALC_CITYHASH64,
		CALC_CITYHASH64_FILE,
		FIND_MODEL_ANIMATIONS,
	} mode = WHOLE_BASE;

	String *parameter = nullptr;
	Array<String> optionalArgs;

	for( int i = 1; i < argc; ++i )
	{
		String arg = argv[ i ];
		if( parameter )
		{
			*parameter = arg;
			parameter = nullptr;
			continue;
		}
		if( arg == "-h" || arg == "--help" )
		{
			print_help();
			return 0;
		}
		else if( arg == "-m" || arg == "--model" )
		{
			mode = SINGLE_MODEL;
			parameter = &path;
		}
		else if( arg == "-t" || arg == "--tobj" )
		{
			mode = SINGLE_TOBJ;
			parameter = &path;
		}
		else if( arg == "-b" || arg == "--base" )
		{
			basepath.push_back( "" );
			parameter = &basepath.back();
		}
		else if( arg == "-e" || arg == "--export" )
		{
			parameter = &exportpath;
		}
		else if( arg == "-d" || arg == "--debug-dds" )
		{
			mode = DEBUG_DDS;
			parameter = &path;
		}
		else if( arg == "--find-model-animations" )
		{
			mode = FIND_MODEL_ANIMATIONS;
			parameter = &path;
		}
		else if( arg == "-extract_f" || arg == "--extract-file" )
		{
			mode = EXTRACT_FILE;
			parameter = &path;
		}
		else if( arg == "-extract_d" || arg == "--extract-directory" )
		{
			mode = EXTRACT_DIRECTORY;
			parameter = &path;
		}
		else if( arg == "-listdir" || arg == "--list-directory" )
		{
			mode = LIST_DIR;
			parameter = &path;
		}
		else if( arg == "-listdir_r" || arg == "--list-directory-recursive" )
		{
			mode = LIST_DIR;
			listdir_r = true;
			parameter = &path;
		}
		else if( arg == "-show_f" || arg == "--show-file" )
		{
			mode = SHOW_FILE;
			parameter = &path;
		}
		else if( arg == "--calc-cityhash64" )
		{
			mode = CALC_CITYHASH64;
			parameter = &path;
		}
		else if( arg == "--calc-cityhash64-file" )
		{
			mode = CALC_CITYHASH64_FILE;
			parameter = &path;
		}
		else if( arg == "-matFormat147" || arg == "--output-material-format147" )
		{
			Material::s_outputMatFormat147Enabled = true;
		}
		else if( arg == "-ddsDxt10" || arg == "--output-dds-dxt10" )
		{
			s_ddsDxt10 = true;
		}
		else if( arg == "-showElapsedTime" || arg == "--show-elapsed-time" )
		{
			showElapsedTime = true;
		}
		else
		{
			optionalArgs.push_back( arg );
		}
	}

	Map<String, FileSystem *> mountedBases;

	int ufsPriority = 1;

	for( const String &base : basepath )
	{
		mountedBases[ base ] = ufsMount( base, true, ufsPriority++ );
	}

	long long startTime =
		std::chrono::duration_cast<std::chrono::microseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();

	int exitCode = 0;

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
		case WHOLE_BASE:
		{
			if (optionalArgs.size() != 1)
			{
				error("system", "", "Invalid parameters!");
				return 1;
			}
			const String &basePathToConvert = optionalArgs[ 0 ];
			FileSystem *fsToConvert = nullptr;
			auto baseToConvertIt = mountedBases.find( basePathToConvert );
			if( baseToConvertIt == mountedBases.end() )
			{
				basepath.push_back( basePathToConvert );
				fsToConvert = mountedBases[ basePathToConvert ] = ufsMount( basePathToConvert, true, ufsPriority++ );
			}
			else
			{
				fsToConvert = baseToConvertIt->second;
			}
			if( exportpath.empty() )
			{
				exportpath = basepath.back() + "_exp";
			}
			exitCode = convertWholeBase( fsToConvert, exportpath ) ? 0 : 1;
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
			SysFileSystem outputFileSystem( exportpath );
			extractFile( *getUFS(), path, outputFileSystem );
		} break;
		case SHOW_FILE:
		{
			if (basepath.empty())
			{
				error("system", "", "Not specified base path!");
				return 1;
			}
			auto file = getUFS()->open(path, FileSystem::read | FileSystem::binary);
			if (file)
			{
				String data(static_cast<size_t>(file->size()), '\0');
				file->blockRead(&data[0], 0, file->size());
				printf("-----------------------\n");
				printf("%s\n", data.c_str());
				printf("-----------------------\n");
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
			SysFileSystem outputFileSystem( exportpath );
			auto files = getUFS()->readDir(path, true, true);
			if (!files)
			{
				error("system", "", "readDir returned null!");
				return 1;
			}
			for (const auto &f : *files)
			{
				if( !f.IsDirectory() )
				{
					extractFile( *getUFS(), f.GetPath(), outputFileSystem );
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
		case CALC_CITYHASH64:
		{
			const u64 calculatedHash = prism::city_hash_64( path.c_str(), path.length() );
			printf( "CityHash64(string:%s) = %llu (0x%llx)\n", path.c_str(), calculatedHash, calculatedHash );
		} break;
		case CALC_CITYHASH64_FILE:
		{
			if( UniquePtr<File> file = getSFS()->open( path, FileSystem::read | FileSystem::binary ) )
			{
				Array<u8> buffer;
				if( file->getContents( buffer ) )
				{
					const u64 calculatedHash = prism::city_hash_64( buffer.data(), buffer.size() );
					printf( "CityHash64(file:%s) = %llu (0x%llx)\n", path.c_str(), calculatedHash, calculatedHash );
				}
				else
				{
					printf( "Unable to read file \'%s\'!\n", path.c_str() );
				}
			}
			else
			{
				printf( "Unable to open file \'%s\'!\n", path.c_str() );
			}
		} break;
		case FIND_MODEL_ANIMATIONS:
		{
			exitCode = printMatchingAnimations( path ) ? 0 : 1;
		} break;
	}

	long long endTime =
		std::chrono::duration_cast<std::chrono::microseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();

	if( showElapsedTime )
	{
		printf( "Elapsed time: %lluus | %llums | %f s\n", endTime - startTime, ( endTime - startTime ) / 1000, static_cast<float>( endTime - startTime ) / 1000.f / 1000.f );
	}

	return exitCode;
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

bool convertWholeBase( FileSystem *fs, String exportpath )
{
	auto files = fs->readDir("/", true, true);
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

		const Optional<StringView> extension = extractExtension(f.GetPath());
		if (extension.has_value() && (extension.value() == ".pmg" || extension.value() == ".tobj"))
		{
			++size;
		}
	}

	int i = 0;
	for (const auto &f : *files)
	{
		if (f.IsDirectory())
			continue;

		const String filename = f.GetPath();
		const Optional<StringView> extension = extractExtension(f.GetPath());
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
	return true;
}

bool printMatchingAnimations( String modelFilePath )
{
	u64 skeletonHash = 0;
	{
		const String modelGeomFilePath = modelFilePath + ".pmg";
		const UniquePtr<File> modelGeomFile = getUFS()->open( modelGeomFilePath, FileSystem::read | FileSystem::binary );
		if( modelGeomFile == nullptr )
		{
			printf( "Unable to open model file \'%s\'!\n", modelGeomFilePath.c_str() );
			return false;
		}
		prism::pmg_0x15::pmg_header_t header;
		if( !modelGeomFile->blockRead( &header, 0, sizeof( header ) ) )
		{
			printf( "Unable to read model file \'%s\'!\n", modelGeomFilePath.c_str() );
			return false;
		}
		if( !( header.m_signature[ 2 ] == 'P' && header.m_signature[ 1 ] == 'm' && header.m_signature[ 0 ] == 'g' ) )
		{
			printf( "Model geometry file \'%s\' has invalid signature!\n", modelGeomFilePath.c_str() );
			return false;
		}
		if( header.m_version != header.SUPPORTED_VERSION )
		{
			printf( "Model geometry file \'%s\' is in unexpected version (%u/%u)!\n", modelGeomFilePath.c_str(), header.m_version, header.SUPPORTED_VERSION );
			return false;
		}
		skeletonHash = header.m_skeleton_hash;
	}

	if( skeletonHash == 0 )
	{
		printf( "Model \'%s\' does not have skeleton!\n", modelFilePath.c_str() );
		return false;
	}

	const auto entries = getUFS()->readDir( "/", true, true );
	if( entries == nullptr )
	{
		printf( "Unable to list root directory recursively!\n" );
		return false;
	}

	for( const FileSystem::Entry &entry : *entries )
	{
		if( entry.IsDirectory() )
		{
			continue;
		}

		const String &filePath = entry.GetPath();

		const Optional<StringView> extension = extractExtension( filePath );
		if( extension.has_value() && extension.value() == ".pma" )
		{
			const UniquePtr<File> animationFile = getUFS()->open( filePath, FileSystem::read | FileSystem::binary );
			if( animationFile == nullptr )
			{
				continue;
			}
			prism::pma_0x05::pma_header_t header;
			if( !animationFile->blockRead( &header, 0, sizeof( header ) ) )
			{
				continue;
			}
			if( header.m_version != header.SUPPORTED_VERSION )
			{
				continue;
			}
			if( header.m_skeleton_hash == skeletonHash )
			{
				printf( "%s\n", filePath.c_str() );
			}
		}
	}

	return true;
}

/* eof */
