#include <rabbit/core/compressor.hpp>

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include <miniz.h>

using namespace rb;

std::size_t compressor::bound(std::size_t uncompressed_size) const {
	return std::size_t(mz_compressBound(mz_ulong(uncompressed_size)));
}

std::size_t compressor::compress(const void* uncompressed_data, std::size_t uncompressed_size, void* compressed_data, std::size_t compressed_bound) const {
	mz_ulong compressed_size = mz_ulong(compressed_bound);

	int status = mz_compress2((unsigned char*)compressed_data,
		&compressed_size,
		(const unsigned char*)uncompressed_data,
		mz_ulong(uncompressed_size),
		MZ_BEST_SPEED);

	return status == MZ_OK ? compressed_size : 0;
}

std::size_t compressor::uncompress(const void* compressed_data, std::size_t compressed_size, void* uncompressed_data, std::size_t uncompressed_size) const {
	mz_ulong uncompressed_size2 = mz_ulong(uncompressed_size);

	int status = mz_uncompress((unsigned char*)uncompressed_data,
		&uncompressed_size2,
		(const unsigned char*)compressed_data,
		mz_ulong(compressed_size));

	return status == MZ_OK ? uncompressed_size2 : 0;
}