#pragma once
#include <cstdint>
#include <limits>

namespace network {
	typedef uint8_t THEADER;
	typedef uint32_t TSIZE;

	static constexpr auto MAX_SIZE_PACKET = std::numeric_limits<TSIZE>::max();

	inline bool advance_cursor(const uint8_t*& cursor, size_t advance, const uint8_t* end) {
		if (cursor + advance > end)
			return false;
		cursor += advance;
		return true;
	}

	enum ERawMaxs {
		HEADER_SIZE = sizeof(THEADER),
		SIZE_SIZE = sizeof(TSIZE),
	};

	enum class EActivityState {
		NONE,
		READ,
		WRITE,
		BOTH,
	};

	enum class ErrorCategory {
		NETWORK,
		PROTOCOL,
		VALIDATION,
		BUSINESS,
		SYSTEM
	};

    enum class ESendResult {
        SUCCESS,
        EMPTY_PACKET,
        NOT_CONNECTED,
		NOT_INITIALIZED,
		ENCRYPTION_FAILED,
    };
}
