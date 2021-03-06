
#ifndef QWY2_HEADER_BITMAP_
#define QWY2_HEADER_BITMAP_

namespace qwy2
{

class PixelData;

void emit_bitmap(
	PixelData const* pixel_grid_data,
	unsigned int pixel_grid_width, unsigned int pixel_grid_height,
	char const* output_file_path);

} /* qwy2 */

#endif /* QWY2_HEADER_BITMAP_ */
