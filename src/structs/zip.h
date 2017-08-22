/*********************************************************************
 *           Copyright (C) 2017 mwl4 - All rights reserved           *
 *********************************************************************
 * File       : zip.h
 * Project    : ConverterPIX
 * Developers : Michal Wojtowicz (mwl450@gmail.com)
 			  : Piotr Krupa (piotrkrupa06@gmail.com)
 *********************************************************************/

#pragma once
#pragma pack(push, 1)

namespace zip
{
	struct COMPRESSION_METHOD
	{
		enum : uint16_t
		{
			STORED				= 0,
			SHRUNK				= 1,
			REDUCED_FACTOR_1	= 2,
			REDUCED_FACTOR_2	= 3,
			REDUCED_FACTOR_3	= 4,
			REDUCED_FACTOR_4	= 5,
			IMPLODED			= 6,
			TOKENZIED			= 7,
			DEFLATED			= 8,
			DEFLATE64			= 9,
			LIB_IMPLODE			= 10,
			RESERVED_0			= 11,
			BZIP2				= 12,
			RESERVED_1			= 13,
			LZMA				= 14,
			RESERVED_2			= 15,
			RESERVED_3			= 16,
			RESERVED_4			= 17,
			IBM_TERSE			= 18,
			IBM_LZ77_PFS		= 19,
			WAVPACK				= 97,
			PPMD_VER_I_REV_1	= 98
		};
	};

	struct VERSION_MADE_BY
	{
		enum : uint16_t
		{
			DOS_OS2				= 0,
			AMIGA				= 1,
			OpenVMS				= 2,
			UNIX				= 3,
			VM_CMS				= 4,
			ATARI_ST			= 5,
			OS2_HPFS			= 6,
			MACINTOSH			= 7,
			Z_SYSTEM			= 8,
			CPM					= 9,
			WINDOWS_NTFS		= 10,
			MVS					= 11,
			VSE					= 12,
			ACORN_RISC			= 13,
			VFAT				= 14,
			ALTERNATIVE_MVS		= 15,
			BEOS				= 16,
			TANDEM				= 17,
			OS_400				= 18,
			DARWIN				= 19
		};
	};

	constexpr uint32_t MAKE_ZIP_SIGNATURE(const uint8_t a, const uint8_t b)
	{
		return ('P' << 0) | ('K' << 8) | (a << 16) | (b << 24);
	}

	struct LocalFileHeader
	{
		static constexpr uint32_t SIGNATURE = MAKE_ZIP_SIGNATURE(3, 4);

		uint32_t signature;					// +0
		uint16_t versionNeededToExtract;	// +4
		uint16_t generalPurposeBitFlag;		// +6
		uint16_t compressionMethod;			// +8
		uint16_t lastModTime;				// +10
		uint16_t lastModDate;				// +12
		uint32_t crc32;						// +14
		uint32_t compressedSize;			// +18
		uint32_t uncompressedSize;			// +22
		uint16_t filenameLength;			// +26
		uint16_t extrafieldLength;			// +28
		/*
			char filename[filenameLength];
			char extraField[extrafieldLength];
			char fileData[compressedSize];
		*/
	};	static_assert(sizeof(LocalFileHeader) == 30, "Unexpected structure size");

	struct DataDescriptor
	{
		uint32_t crc32;						// +0
		uint32_t compressedSize;			// +4
		uint32_t uncompressedSize;			// +8
	};	static_assert(sizeof(DataDescriptor) == 12, "Unexpected structure size");

	struct ArchiveExtraData
	{
		static constexpr uint32_t SIGNATURE = MAKE_ZIP_SIGNATURE(6, 8);

		uint32_t signature;					// +0
		uint32_t length;					// +4
		/*
			char data[length];
		*/
	};	static_assert(sizeof(ArchiveExtraData) == 8, "Unexpected structure size");

	struct EndOfCentralDirectory
	{
		static constexpr uint32_t SIGNATURE = MAKE_ZIP_SIGNATURE(5, 6);

		uint32_t signature;					// +0
		uint16_t diskNumber;				// +4
		uint16_t startDisk;					// +6
		uint16_t startOffset;				// +8
		uint16_t numEntries;				// +10
		uint32_t size;						// +12
		uint32_t offset;					// +16
		uint16_t zipCommentLength;			// +20
		/*
			char zipCommentData[zipCommentLength];
		*/
	};	static_assert(sizeof(EndOfCentralDirectory) == 22, "Unexpected structure size");

	struct ZIP64EndOfCentralDirectory
	{
		static constexpr uint32_t SIGNATURE = MAKE_ZIP_SIGNATURE(6, 6);

		uint32_t signature;					// +0
		uint64_t sizeOfCentralDirRecord;	// +4
		uint16_t versionMadeBy;				// +12
		uint16_t versionNeededToExtract;	// +14
		uint32_t diskNumber;				// +16
		uint32_t startCentralDirDiskNumber;	// +20
		uint64_t centralDirEntriesOnDisk;	// +24
		uint64_t centralDirTotalEntries;	// +32
		uint64_t centralDirSize;			// +40
		uint64_t centralDirOffsetStartDisk;	// +48
	};	static_assert(sizeof(ZIP64EndOfCentralDirectory) == 56, "Unexpected structure size");

	struct DigitalSignature
	{
		static constexpr uint32_t SIGNATURE = MAKE_ZIP_SIGNATURE(5, 5);

		uint32_t signature;					// +0
		uint16_t sizeOfData;				// +4
		/*
			char content[sizeOfData];
		*/
	};	static_assert(sizeof(DigitalSignature) == 6, "Unexpected structure size");

	struct CentralDirectoryFileHeader
	{
		static constexpr uint32_t SIGNATURE = MAKE_ZIP_SIGNATURE(1, 2);

		uint32_t signature;					// +0
		uint16_t versionMadeBy;				// +4
		uint16_t versionNeededToExtract;	// +6
		uint16_t generalPurposeBitFlag;		// +8
		uint16_t compressionMethod;			// +10
		uint16_t lastModTime;				// +12
		uint16_t lastModDate;				// +14
		uint32_t crc32;						// +16
		uint32_t compressedSize;			// +20
		uint32_t uncompressedSize;			// +24
		uint16_t filenameLength;			// +28
		uint16_t extrafieldLength;			// +30
		uint16_t fileCommentLength;			// +32
		uint16_t diskNumberStart;			// +34
		uint16_t internalFileAttributes;	// +36
		uint32_t externalFileAttributes;	// +38
		uint32_t relOffsetOfLocalHeader;	// +42
		/*
			char filename[filenameLength];
			char extrafield[extrafieldLength];
			char fileComment[fileCommentLength];
		*/
	};	static_assert(sizeof(CentralDirectoryFileHeader) == 46, "Unexpected structure size");

	struct ZIP64EndOfCentralDirectoryLocator
	{
		static constexpr uint32_t SIGNATURE = MAKE_ZIP_SIGNATURE(6, 7);

		uint32_t signature;					// +0
		uint32_t diskNumber;				// +4
		uint64_t relativeOffset;			// +8
		uint32_t totalNumDisks;				// +16
	};	static_assert(sizeof(ZIP64EndOfCentralDirectoryLocator) == 20, "Unexpected structure size");
} // namespace Zip

#pragma pack(pop)

/* eof */
