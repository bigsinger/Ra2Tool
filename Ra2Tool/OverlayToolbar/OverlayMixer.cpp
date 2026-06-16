#include "OverlayMixer.h"

#include <algorithm>
#include <windows.h>
#include <Surface.h>

namespace {

inline unsigned short BlendRgb565(unsigned short dst, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	if (a == 255) {
		return static_cast<unsigned short>(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
	}

	const unsigned char dr = static_cast<unsigned char>(((dst >> 11) & 0x1F) * 255 / 31);
	const unsigned char dg = static_cast<unsigned char>(((dst >> 5) & 0x3F) * 255 / 63);
	const unsigned char db = static_cast<unsigned char>((dst & 0x1F) * 255 / 31);
	const unsigned int inv = 255 - a;

	const unsigned char nr = static_cast<unsigned char>((r * a + dr * inv) / 255);
	const unsigned char ng = static_cast<unsigned char>((g * a + dg * inv) / 255);
	const unsigned char nb = static_cast<unsigned char>((b * a + db * inv) / 255);

	return static_cast<unsigned short>(((nr >> 3) << 11) | ((ng >> 2) << 5) | (nb >> 3));
}

}

bool DrawOverlayRGBA(int x, int y, int width, int height, const unsigned char* rgba) {
	if (!rgba || width <= 0 || height <= 0) {
		return false;
	}

	__try {
		DSurface* surface = DSurface::Composite;
		if (!surface || surface->GetBytesPerPixel() != 2 || !surface->CanLock()) {
			return false;
		}

		const int surfaceWidth = surface->GetWidth();
		const int surfaceHeight = surface->GetHeight();
		const int left = std::max(0, x);
		const int top = std::max(0, y);
		const int right = std::min(surfaceWidth, x + width);
		const int bottom = std::min(surfaceHeight, y + height);
		if (left >= right || top >= bottom) {
			return false;
		}

		void* bits = surface->Lock(0, 0);
		if (!bits) {
			return false;
		}

		const int pitch = surface->GetPitch();
		for (int sy = top; sy < bottom; ++sy) {
			auto* dst = reinterpret_cast<unsigned short*>(reinterpret_cast<unsigned char*>(bits) + sy * pitch);
			const int srcY = sy - y;
			const unsigned char* src = rgba + (srcY * width + (left - x)) * 4;
			for (int sx = left; sx < right; ++sx) {
				const unsigned char r = src[0];
				const unsigned char g = src[1];
				const unsigned char b = src[2];
				const unsigned char a = src[3];
				if (a != 0) {
					dst[sx] = BlendRgb565(dst[sx], r, g, b, a);
				}
				src += 4;
			}
		}

		surface->Unlock();
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}
