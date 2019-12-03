#pragma once

namespace tsm
{
struct MemoryWeight
{
	struct MByteUnit
	{
		BYTE _[1024][1024];
	};

	MByteUnit _[MEMORY_WEIGHT_MBYTE];
};
}
