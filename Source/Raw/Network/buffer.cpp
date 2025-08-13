#include "buffer.h"

namespace network {
	TMP_BUFFER::TMP_BUFFER(TSIZE _size) : m_pos(0) {
		v_data.resize(_size);
	}

	void TMP_BUFFER::clear_and_resize(TSIZE _size) {
		v_data.clear();
		v_data.resize(_size);
	}

	void TMP_BUFFER::write(const void* _data, TSIZE _size) {
		TSIZE needSpace = m_pos + _size;
		if (v_data.size() < needSpace) /*then need resize*/ {
			v_data.resize(needSpace);
		}
		std::memcpy(v_data.data() + m_pos, _data, _size);
		m_pos += _size;
	};

	std::vector<uint8_t>& TMP_BUFFER::get() { return v_data; }
}
