#pragma once
#include <vector>
#include <cstdio>

#include "common.h"

namespace network {
	class TMP_BUFFER {
		std::vector<uint8_t> v_data;
		TSIZE m_pos;
	public:
		TMP_BUFFER(TSIZE _size = 0);
		
		void write(const void* _data, TSIZE _size);
		std::vector<uint8_t>& get();
		void clear_and_resize(TSIZE _size);

		//takes null-terminated string
		static inline void str_copy(char* buffer, const size_t& size, const char* data) {
			if (!buffer || !data || size == 0)
				return;

			std::snprintf(buffer, size, "%s", data);
		}
	};
};

