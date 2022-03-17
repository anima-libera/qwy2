
#include "nature.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>

namespace qwy2
{

namespace
{

/* Bitmap file format header (without the 2 bytes magic number),
 * for more information, see
 * - https://en.wikipedia.org/wiki/BMP_file_format
 * - https://www.digicamsoft.com/bmp/bmp.html */
class BitmapHeader
{
public:
	/* BITMAPFILEHEADER members (without the 2 bytes magic number). */
	/* The magic number 'BM' should be written just before this header,
	 * without padding (that is why it is not a member of this type definition). */
	std::uint32_t file_size;
	std::uint16_t reserved_1, reserved_2; /* Can both be 0. */
	std::uint32_t file_offset_to_pixel_array;

	/* BITMAPINFOHEADER members. */
	std::uint32_t dbi_header_size; /* Size of the BITMAPINFOHEADER header only. */
	std::int32_t image_width;
	std::int32_t image_height;
	std::uint16_t planes; /* Must be 1. */
	std::uint16_t bits_per_pixel;
	std::uint32_t compression; /* 0 means no compression, 3 means BI_BITFIELDS. */
	std::uint32_t image_size;
	std::int32_t x_pixel_per_meter;
	std::int32_t y_pixel_per_meter;
	std::uint32_t colors_in_color_table; /* Can be 0. */
	std::uint32_t important_color_count; /* Can be 0. */
};

} /* anonymous namespace */

void emit_bitmap(
	PixelData const* pixel_grid_data,
	unsigned int pixel_grid_width, unsigned int pixel_grid_height,
	char const* output_file_path)
{
	std::ofstream file{output_file_path, std::ios::binary};
	if (not file.is_open())
	{
		std::cerr << "Error: "
			<< "Could not open \"" << output_file_path << "\"" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	unsigned int const magic_number_size = 2;
	unsigned int const channel_mask_size = 3 * sizeof(PixelData);
	unsigned int const image_size = pixel_grid_width * pixel_grid_height * sizeof(PixelData);

	/* Magic number. */
	file.write("BM", magic_number_size);

	/* Header (using the BITMAPINFOHEADER header version). */
	BitmapHeader header;
	header.file_size =
		magic_number_size + sizeof(BitmapHeader) + channel_mask_size + image_size;
	header.file_offset_to_pixel_array =
		magic_number_size + sizeof(BitmapHeader) + channel_mask_size;
	header.dbi_header_size =
		sizeof(BitmapHeader) - offsetof(BitmapHeader, dbi_header_size);
	header.image_width = pixel_grid_width;
	header.image_height = pixel_grid_height;
	header.planes = 1;
	header.bits_per_pixel = sizeof(PixelData) * 8;
	header.compression = 3; /* BI_BITFIELDS */
		/* Note:
		* BI_BITFIELDS is used here instead of BI_ALPHABITFIELDS due ot a lack
		* of support for the latter option. */
	header.image_size = image_size;
	header.x_pixel_per_meter = 5000; /* Is it good ? It seems to be ok... */
	header.y_pixel_per_meter = 5000; /* Is it good ? It seems to be ok... */
	header.colors_in_color_table = 0; /* For some reason it must be 0, even though it is 3. */
	header.important_color_count = 0;
	file.write(reinterpret_cast<char const*>(&header), sizeof header);

	/* Channel masks. */
	PixelData const mask_r{0xff, 0x00, 0x00, 0x00};
	PixelData const mask_g{0x00, 0xff, 0x00, 0x00};
	PixelData const mask_b{0x00, 0x00, 0xff, 0x00};
	file.write(reinterpret_cast<char const*>(&mask_r), sizeof(PixelData));
	file.write(reinterpret_cast<char const*>(&mask_g), sizeof(PixelData));
	file.write(reinterpret_cast<char const*>(&mask_b), sizeof(PixelData));
	
	/* Pixel array. As the size of a pixel is 32 bits, the padding between rows
	 * of the bitmap pixel array to alling on 32 bits is of size 0
	 * and does not need to be taken care of explicitly. */
	file.write(reinterpret_cast<char const*>(pixel_grid_data), image_size);
}

} /* qwy2 */
