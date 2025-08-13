#pragma once

#include <vector>
#include <functional>

#include "Network/common.h"

namespace network {
	struct TPacketDescriptor final {
		THEADER header;                 // Packet type
		size_t fixed_size;             // Fixed size (including header)
		bool is_dynamic;               // Is dynamic size?
		size_t size_offset;            // Location of dynamic size information

		// Sabit boyutlu paketler icin constructor
		TPacketDescriptor(THEADER h, size_t size)
			: header(h)
			, fixed_size(size)
			, is_dynamic(false)
			, size_offset(0) {
		}

		// Dinamik boyutlu paketler icin constructor
		TPacketDescriptor(THEADER h, size_t size, size_t offset)
			: header(h)
			, fixed_size(size)
			, is_dynamic(true)
			, size_offset(offset){
		}
	};
}; // namespace network 
