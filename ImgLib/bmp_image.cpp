#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

	// функция вычисления отступа по ширине
	static int GetBMPStride(int w) {
		return 4 * ((w * 3 + 3) / 4);
	}

	PACKED_STRUCT_BEGIN BitmapFileHeader{
		// Подпись
		char b_symbol = 'B';
		char m_symbol = 'M';
		// Суммарный размер данных
		uint32_t data_size = 0;
		// Зарезервированное пространство
		uint32_t reserve_space = 0;
		// Зарезервированное пространство
		uint32_t offset = 54;
	}
		PACKED_STRUCT_END

		PACKED_STRUCT_BEGIN BitmapInfoHeader{
		// Размер заголовка
		uint32_t header_size = 40;
	// Ширина изображения в пикселях
	int32_t width = 0;
	// Высота изображения в пикселях
	int32_t height = 0;
	// Количество плоскостей
	uint16_t flatness = 1;
	// Количество бит на пиксель
	uint16_t bit_per_pixel = 24;
	// Тип сжатия
	uint32_t compress_type = 0;
	// Количество байт в данных
	uint32_t amount_bytes_in_data = 0;
	// Горизонтальное разрешение, пикселей на метр
	int32_t horizontal_ppm = 11811;
	// Вертикальное разрешение, пикселей на метр
	int32_t vertical_ppm = 11811;
	// Количество использованных цветов
	int32_t number_colors_used = 0;
	// Количество значимых цветов
	int32_t number_significant_colors = 0x1000000;
	}
		PACKED_STRUCT_END

		// напишите эту функцию
		bool SaveBMP(const Path& file, const Image& image) {
		const int width = image.GetWidth();
		const int height = image.GetHeight();
		const int stride = GetBMPStride(width);

		BitmapFileHeader file_header;
		file_header.data_size = stride * height + file_header.offset;

		BitmapInfoHeader info_header;
		info_header.width = width;
		info_header.height = height;
		info_header.amount_bytes_in_data = stride * height;

		ofstream out(file, ios::binary);
		if (!out) return false;
		out.write(reinterpret_cast<const char*>(&file_header), 14);
		out.write(reinterpret_cast<const char*>(&info_header), 40);

		std::vector<char> buff(stride);

		for (int y = image.GetHeight() - 1; y >= 0; --y) {
			const Color* line = image.GetLine(y);
			for (int x = 0; x < image.GetWidth(); ++x) {
				buff[x * 3 + 0] = static_cast<char>(line[x].b);
				buff[x * 3 + 1] = static_cast<char>(line[x].g);
				buff[x * 3 + 2] = static_cast<char>(line[x].r);
			}
			out.write(buff.data(), stride);
		}

		return out.good();
	}

	// напишите эту функцию
	Image LoadBMP(const Path& file) {
		ifstream ifs(file, ios::binary);
		if (!ifs) return {};

		std::array<char, 2> signature{};
		ifs.read(signature.data(), signature.size());
		if (signature[0] != 'B' || signature[1] != 'M') {
			return {};
		}

		int width, height;
		ifs.ignore(16);
		ifs.read(reinterpret_cast<char*>(&width), sizeof(width));
		ifs.read(reinterpret_cast<char*>(&height), sizeof(height));
		ifs.ignore(28);

		int stride = GetBMPStride(width);
		Image result(stride / 3, height, Color::Black());
		std::vector<char> buff(width * 3);

		for (int y = result.GetHeight() - 1; y >= 0; --y) {
			Color* line = result.GetLine(y);
			ifs.read(buff.data(), stride);
			for (int x = 0; x < width; ++x) {
				line[x].b = static_cast<byte>(buff[x * 3 + 0]);
				line[x].g = static_cast<byte>(buff[x * 3 + 1]);
				line[x].r = static_cast<byte>(buff[x * 3 + 2]);
			}
		}

		return result;
	}

}  // namespace img_lib