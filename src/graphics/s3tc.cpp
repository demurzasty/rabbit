#include <rabbit/graphics/s3tc.hpp>
#include <rabbit/graphics/color.hpp>

#include <rgbcx.hpp>

#include <mutex>
#include <memory>

using namespace rb;

static std::once_flag s_flag;

static void ensure() {
	std::call_once(s_flag, [] {
		rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal);
	});
}

void s3tc::bc1(const void* uncompressed_pixels, std::size_t uncompressed_size, std::size_t stride, void* compressed_pixels) const {
	ensure();

	struct block {
		color colors[2];
	};

	const auto pixels = reinterpret_cast<const color*>(uncompressed_pixels);
	const auto blocks = reinterpret_cast<block*>(compressed_pixels);

	color input_block[16];

	const auto size_x = stride / 4;
	const auto blocks_x = size_x / 4;
	const auto size_y = uncompressed_size / stride;

	for (auto y = 0u; y < size_y; y += 4) {
		for (auto x = 0u; x < size_x; x += 4) {
			input_block[0] = pixels[y * size_x + x];
			input_block[1] = pixels[y * size_x + x + 1];
			input_block[2] = pixels[y * size_x + x + 2];
			input_block[3] = pixels[y * size_x + x + 3];

			input_block[4] = pixels[(y + 1) * size_x + x];
			input_block[5] = pixels[(y + 1) * size_x + x + 1];
			input_block[6] = pixels[(y + 1) * size_x + x + 2];
			input_block[7] = pixels[(y + 1) * size_x + x + 3];

			input_block[8] = pixels[(y + 2) * size_x + x];
			input_block[9] = pixels[(y + 2) * size_x + x + 1];
			input_block[10] = pixels[(y + 2) * size_x + x + 2];
			input_block[11] = pixels[(y + 3) * size_x + x + 3];

			input_block[12] = pixels[(y + 3) * size_x + x];
			input_block[13] = pixels[(y + 3) * size_x + x + 1];
			input_block[14] = pixels[(y + 3) * size_x + x + 2];
			input_block[15] = pixels[(y + 3) * size_x + x + 3];

			const auto by = y / 4;
			const auto bx = x / 4;

			rgbcx::encode_bc1(&blocks[by * blocks_x + bx], (const std::uint8_t*)&input_block);
		}
	}
}

void s3tc::bc3(const void* uncompressed_pixels, std::size_t uncompressed_size, std::size_t stride, void* compressed_pixels) const {
	ensure();

	struct block {
		color colors[4];
	};

	const auto pixels = reinterpret_cast<const color*>(uncompressed_pixels);
	const auto blocks = reinterpret_cast<block*>(compressed_pixels);

	color input_block[16];

	const auto size_x = stride / 4;
	const auto blocks_x = size_x / 4;
	const auto size_y = uncompressed_size / stride;

	for (auto y = 0u; y < size_y; y += 4) {
		for (auto x = 0u; x < size_x; x += 4) {
			input_block[0] = pixels[y * size_x + x];
			input_block[1] = pixels[y * size_x + x + 1];
			input_block[2] = pixels[y * size_x + x + 2];
			input_block[3] = pixels[y * size_x + x + 3];

			input_block[4] = pixels[(y + 1) * size_x + x];
			input_block[5] = pixels[(y + 1) * size_x + x + 1];
			input_block[6] = pixels[(y + 1) * size_x + x + 2];
			input_block[7] = pixels[(y + 1) * size_x + x + 3];

			input_block[8] = pixels[(y + 2) * size_x + x];
			input_block[9] = pixels[(y + 2) * size_x + x + 1];
			input_block[10] = pixels[(y + 2) * size_x + x + 2];
			input_block[11] = pixels[(y + 3) * size_x + x + 3];

			input_block[12] = pixels[(y + 3) * size_x + x];
			input_block[13] = pixels[(y + 3) * size_x + x + 1];
			input_block[14] = pixels[(y + 3) * size_x + x + 2];
			input_block[15] = pixels[(y + 3) * size_x + x + 3];

			const auto by = y / 4;
			const auto bx = x / 4;

			rgbcx::encode_bc3(&blocks[by * blocks_x + bx], (const std::uint8_t*)&input_block);
		}
	}
}

std::vector<std::uint8_t> s3tc::bc1(const image& image) const {
	const auto compressed_size = (image.size().x * image.size().y) / 2;
	auto compressed_pixels = std::make_unique<std::uint8_t[]>(compressed_size);
	bc1(image.pixels().data(), image.pixels().size_bytes(), image.stride(), compressed_pixels.get());
	return { compressed_pixels.get(), compressed_pixels.get() + compressed_size };
}

std::vector<std::uint8_t> s3tc::bc3(const image& image) const {
	const auto compressed_size = (image.size().x * image.size().y);
	auto compressed_pixels = std::make_unique<std::uint8_t[]>(compressed_size);
	bc3(image.pixels().data(), image.pixels().size_bytes(), image.stride(), compressed_pixels.get());
	return { compressed_pixels.get(), compressed_pixels.get() + compressed_size };
}