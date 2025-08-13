#pragma once
#include <memory>
#include <type_traits>
namespace NUtility {
	namespace NTemplates {
		// Bir tipin akilli isaretci olup olmadigini denetleyen trait
		template<typename T>
		struct is_smart_pointer : std::false_type {};

		template<typename T>
		struct is_smart_pointer<std::shared_ptr<T>> : std::true_type {};

		template<typename T, typename D>
		struct is_smart_pointer<std::unique_ptr<T, D>> : std::true_type {};

		// akilli isaretcileri tespit eden helper
		template<typename T>
		constexpr bool is_smart_pointer_v = is_smart_pointer<std::decay_t<T>>::value;

		template<typename T>
		constexpr bool is_unique_ptr_v = std::is_same_v<T, std::unique_ptr<typename T::element_type>>;

		template<typename T>
		constexpr bool is_shared_ptr_v = std::is_same_v<T, std::shared_ptr<typename T::element_type>>;
	}
}