/**
 *  @file oglplus/imports/blend_file.hpp
 *  @brief Tools for reading Blender's .blend files
 *
 *  @author Matus Chochlik
 *
 *  Copyright 2010-2012 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef OGLPLUS_IMPORTS_BLEND_FILE_1107121519_HPP
#define OGLPLUS_IMPORTS_BLEND_FILE_1107121519_HPP

#include <oglplus/imports/blend_file/utils.hpp>
#include <oglplus/imports/blend_file/reader_client.hpp>
#include <oglplus/imports/blend_file/range.hpp>
#include <oglplus/imports/blend_file/info.hpp>
#include <oglplus/imports/blend_file/sdna.hpp>
#include <oglplus/imports/blend_file/pointer.hpp>
#include <oglplus/imports/blend_file/block.hpp>
#include <oglplus/imports/blend_file/type.hpp>
#include <oglplus/imports/blend_file/structure.hpp>
#include <oglplus/imports/blend_file/flattened.hpp>
#include <oglplus/imports/blend_file/block_data.hpp>
#include <oglplus/imports/blend_file/struct_block_data.hpp>
#include <cstring>

namespace oglplus {
namespace imports {

class BlendFileStructGlobBlock
 : public BlendFileFlatStructBlockData
{
private:

	friend class BlendFile;

	BlendFileStructGlobBlock(BlendFileFlatStructBlockData&& tmp)
	 : BlendFileFlatStructBlockData(std::move(tmp))
	 , curscreen(Field<void*>("curscreen"))
	 , curscene(Field<void*>("curscene"))
	{ }
public:
	BlendFileFlatStructTypedFieldData<void*> curscreen;
	BlendFileFlatStructTypedFieldData<void*> curscene;
};

/// Represents and allows access to the structures and data of a .blend file
/**
 *  @note The objects representing blocks, structures, structure fields, etc.
 *  created directly or indirectly from a BlendFile instance must not be used
 *  after their "parent" BlendFile is destroyed. Doing so results in undefined
 *  behaviour.
 */
class BlendFile
 : public BlendFileReaderClient
{
private:
	BlendFileReader _reader;

	BlendFileInfo _info;

	std::vector<BlendFileBlock> _blocks;
	std::map<BlendFilePointer::ValueType, std::size_t> _block_map;

	std::size_t _glob_block_index;

	std::shared_ptr<BlendFileSDNA> _sdna;

	// internal string equality comparison utility
	template <std::size_t N>
	bool _equal(const std::array<char, N>& a, const char* b)
	{
		return std::strncmp(a.data(), b, N) == 0;
	}

public:
	/// Parses the file from an input stream
	/**
	 *  @note The input stream must exist during the whole lifetime
	 *  of an instance of BlendFile
	 */
	BlendFile(std::istream& input)
	 : _reader(input)
	 , _info(_reader)
	 , _glob_block_index(std::size_t(-1))
	{
		std::size_t block_idx = 0;
		while(!_eof(_reader))
		{
			std::array<char,4> code  = _read_array<4>(
				_reader,
				"Failed to read file block code"
			);
			if(_equal(code, "GLOB"))
				_glob_block_index = block_idx;

			if(_equal(code, "DNA1"))
			{
				_blocks.emplace_back(
					_reader,
					_info,
					std::move(code),
					false
				);
				_sdna = std::make_shared<BlendFileSDNA>(
					_reader,
					_info
				);
			}
			else
			{
				_blocks.emplace_back(
					_reader,
					_info,
					std::move(code),
					true
				);
			}
			_block_map[_blocks.back()._old_ptr] = block_idx++;
		}
		if(_glob_block_index == std::size_t(-1))
		{
			throw std::runtime_error(
				"Blend file does not contain GLOB block"
			);
		}
		if(!_sdna)
		{
			throw std::runtime_error(
				"Blend file does not contain SDNA block"
			);
		}
	}

	/// Returns the basic file-level information
	const BlendFileInfo& Info(void) const
	{
		return _info;
	}

	/// Returns a range of meta-data describing structures unsed in the file
	BlendFileStructRange Structures(void) const
	{
		return BlendFileStructRange(_sdna.get());
	}

	/// Returns a block by its pointer
	const BlendFileBlock& BlockByPointer(BlendFilePointer pointer) const
	{
		auto ptr = pointer.Value();
		auto pos = _block_map.find(ptr);
		if(pos == _block_map.end())
		{
			auto pos2 = _block_map.lower_bound(ptr);
			if(pos2 != _block_map.end())
			{
				if(pos2 != _block_map.begin())
				{
					--pos2;
					assert(pos2->first < ptr);
					assert(pos2->second < _blocks.size());
					std::size_t size = _blocks[pos2->second].Size();
					if(ptr - pos2->first < size) pos = pos2;
				}
			}
		}
		if(pos == _block_map.end())
		{
			throw std::runtime_error(
				"Unable to find block by pointer"
			);
		}
		assert(pos->second < _blocks.size());

		return _blocks[pos->second];
	}

	/// Returns a structured block by its pointer
	BlendFileFlatStructBlockData StructuredBlockByPointer(
		BlendFilePointer pointer
	)
	{
		auto block = BlockByPointer(pointer);
		auto offset = pointer - block.Pointer();
		auto block_data = BlockData(block);
		auto flat_struct =
			(offset == 0)?
			BlockStructure(block).Flattened():
			Pointee(pointer).AsStructure().Flattened();

		return BlendFileFlatStructBlockData(
			std::move(flat_struct),
			std::move(block),
			std::move(block_data),
			offset
		);
	}

	/// Alias for StructuredBlockByPointer
	BlendFileFlatStructBlockData operator[](BlendFilePointer pointer)
	{
		return StructuredBlockByPointer(pointer);
	}

	/// Returns the global block of the file
	const BlendFileBlock& GlobalBlock(void) const
	{
		return _blocks[_glob_block_index];
	}

	/// Returns a pointer to the global block
	BlendFilePointer GlobalBlockPointer(void) const
	{
		return GlobalBlock().Pointer();
	}

	/// Returns a structured global block object
	BlendFileStructGlobBlock StructuredGlobalBlock(void)
	{
		return BlendFileStructGlobBlock(
			StructuredBlockByPointer(
				GlobalBlockPointer()
			)
		);
	}

	/// Returns a range allowing the traversal of file blocks
	BlendFileBlockRange Blocks(void) const
	{
		return BlendFileBlockRange(_blocks);
	}

	/// Returns the structures of a file block
	BlendFileStruct BlockStructure(const BlendFileBlock& block) const
	{
		return BlendFileStruct(_sdna.get(), block._sdna_index);
	}

	/// Returns the data of a block
	BlendFileBlockData BlockData(const BlendFileBlock& block)
	{
		std::vector<char> data;
		if(block.Size())
		{
			data.resize(block.Size());
			_go_to(_reader, block.DataPosition());
			_raw_read(
				_reader,
				data.data(),
				data.size(),
				"Failed to read blend file block data"
			);
		}
		return BlendFileBlockData(
			std::move(data),
			_info.ByteOrder(),
			_info.PointerSize(),
			_sdna->_type_sizes[
				_sdna->_structs[block._sdna_index]._type_index
			]
		);
	}

	/// Returns the pointee type for a pointer
	BlendFileType Pointee(const BlendFilePointer& pointer) const
	{
		return BlendFileType(
			_sdna.get(),
			pointer._type_index,
			_sdna->_type_structs[pointer._type_index]
		);
	}
};

} // imports
} // oglplus

#endif // include guard