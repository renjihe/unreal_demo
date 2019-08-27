#include "SprayMask.h"
#include "m_public.h"


int USprayMask::GetMask(float u, float v) const
{
	unsigned x = Width * (u + 1) * 0.5, y = Height * (v + 1) * 0.5;
	CLAMP(x, 0, Width-1);
	CLAMP(y, 0, Height-1);

	int index = x + y *Width;
	if (index < Pixels.Num()) {
		return Pixels[index].A;
	}

	return 0;
}


