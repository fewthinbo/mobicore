#pragma once

#include <cstdint>
namespace NUtility {
	class CFlagWrapper {
		uint32_t m_flags;
	public:
		CFlagWrapper(uint32_t flags = 0) noexcept : m_flags(flags) {} //not explicit, you can use = operator to construct

		void AddFlag(uint32_t flag) noexcept {
			m_flags |= flag;
		}

		void AddFlag(const CFlagWrapper& other) noexcept {
			m_flags |= other.GetFlags();
		}

		void RemoveFlag(uint32_t flag) noexcept {
			m_flags &= ~flag;
		}

		void RemoveFlag(const CFlagWrapper& other) noexcept {
			m_flags &= ~other.GetFlags();
		}

		bool HasFlag(uint32_t flag) const noexcept {
			return (m_flags & flag) != 0;
		}

		void Clear() noexcept {
			m_flags = 0;
		}

		uint32_t GetFlags() const noexcept {
			return m_flags;
		}

		void SetFlags(uint32_t flags) noexcept {
			m_flags = flags;
		}

		//if flag is already set, it will be removed, otherwise it will be added.
		void ToggleFlag(uint32_t flag) noexcept {
			m_flags ^= flag;
		}

		uint32_t operator=(uint32_t flags) noexcept {
			m_flags = flags;
			return m_flags;
		}
	};
}
