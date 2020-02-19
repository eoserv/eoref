#ifndef EO_PE_READER_HPP
#define EO_PE_READER_HPP

#include "cio/cio.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>

class pe_reader
{
	public:
		struct BitmapInfo
		{
			std::size_t start;
			std::size_t size;
			int width;
			int height;
		};

	private:
		enum class ResourceType : std::uint32_t
		{
			Cursor = 1,
			Bitmap = 2,
			Icon = 3,
			Menu = 4,
			Dialog = 5,
			StringTable = 6,
			FontDirectory = 7,
			Font = 8,
			Accelerator = 9,
			Unformatted = 10,
			MessageTable = 11,
			GroupCursor = 12,
			GroupIcon = 14,
			VersionInformation = 16
		};

		struct ResourceDirectory
		{
			std::uint32_t Characteristics;
			std::uint32_t TimeDateStamp;
			std::uint16_t MajorVersion;
			std::uint16_t MinorVersion;
			std::uint16_t NumberOfNamedEntries;
			std::uint16_t NumberOfIdEntries;
		};

		struct ResourceDirectoryEntry
		{
			ResourceType ResourceType_;
			std::uint32_t SubDirectoryOffset;
		};

		struct ResourceDataEntry
		{
			std::uint32_t OffsetToData;
			std::uint32_t Size;
			std::uint32_t CodePage;
			std::uint32_t unused;
		};

		cio::stream file;
		
		std::uint32_t root_address = 0;
		std::uint32_t virtual_address = 0;
		ResourceDirectoryEntry bitmap_directory_entry = {ResourceType{}, 0};

		std::uint16_t read_u16_le();
		std::uint32_t read_u32_le();

		ResourceDirectory read_ResourceDirectory();
		ResourceDirectoryEntry read_ResourceDirectoryEntry();
		ResourceDataEntry read_ResourceDataEntry();

	public:
		pe_reader(cio::stream&& file)
			: file(std::move(file))
		{ }

		bool read_header();

		std::map<int, BitmapInfo> read_bitmap_table();

		bool read_resource(char* buf, std::size_t start, std::size_t size);

		cio::stream& get_file()
		{
			return file;
		}

		cio::stream&& finish()
		{
			return std::move(file);
		}
};

#endif // EO_PE_READER_HPP
