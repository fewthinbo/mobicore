#pragma once
#include <cstdint>
#include <variant>
#include <unordered_map>
#include <memory>
#include <vector>
#include <utility>
#include <stdexcept>
#include <string>


#include <Network/common.h>
#include "packet_constants.h"

//this file should be synced with bridgeServer
namespace mobi_game {
	/*
		System Info:
		* This system designed for:
		* - Ease server management (manage with mobile devices).
		* - Writing custom packets independently from us.

		Usage:
		* - Define your packet id in enum class ECustomPackets
		* - Write details in g_custom_packets via looking other examples.
		* - Write a handler to ../packet_handlers.cpp::HandleForwardPacket for handle received data from mobile
		* - Send uptodate "custom_packets.h" file to mobicore.io@gmail.com to put in bridgeServer vds.
		* - Build your mt and run
		* - Mobile side will be adjusted automatically.
		* - You're ready to send data from admin panel on your mobile device.
	*/

	namespace custom_packets {
		//Custom packets'leri runtime'da guncellenebilir hale getirmeli miyim?
		//Handler'lar lazim oldugundan runtime'da yapilacak bir islem kalmiyor.

		//mobil arayuzde gosterim icin belirtmek gerekir.
		enum class EPrimitiveTypeMobile {
			CHAR,
			BOOL,
			INT,
			UINT,
			FLOAT,
		};

		struct TCustomParam final {
			TSIZE size{};
			std::variant<bool, char, int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, float, double, std::monostate> type_var;
			TSIZE step_count{};
			const char* desc{};
			EPrimitiveTypeMobile mobile_type{};
			constexpr TCustomParam() noexcept : type_var(std::monostate{}), desc("") {}

			template<typename T>
			constexpr explicit TCustomParam(TSIZE expected_size, std::in_place_type_t<T>, const char* description)
				: size(expected_size), desc(description != nullptr ? description : "")
			{
				step_count = expected_size / sizeof(T);
				//static_assert(expected_size >= sizeof(T), "Type size cannot be bigger than expected size");
				type_var.emplace<T>(T{});

				using TPrm = std::decay_t<T>;
				if constexpr (std::is_same_v<TPrm, char>) {
					mobile_type = EPrimitiveTypeMobile::CHAR;
				}
				else if constexpr (std::is_same_v<TPrm, bool>) {
					mobile_type = EPrimitiveTypeMobile::BOOL;
				}
				else if constexpr (std::is_same_v<TPrm, uint8_t> || std::is_same_v<TPrm, uint16_t> || std::is_same_v<TPrm, uint32_t>) {
					mobile_type = EPrimitiveTypeMobile::UINT;
				}
				else if constexpr (std::is_same_v<TPrm, int> || std::is_same_v<TPrm, long> || std::is_same_v<TPrm, short>) {
					mobile_type = EPrimitiveTypeMobile::INT;
				}
				else if constexpr (std::is_same_v<TPrm, float> || std::is_same_v<TPrm, double>) {
					mobile_type = EPrimitiveTypeMobile::FLOAT;
				}
			}
		};


		struct ICustomPacket {
			size_t param_c{};
			std::string description;
			uint8_t min_authority{ 5 }; //EGMLevels::GM_IMPLEMENTOR
			ICustomPacket& AdjustMinAuthority(uint8_t auth) {
				min_authority = auth;
				return *this;
			}
			ICustomPacket& AdjustDescription(const std::string& desc) {
				description = desc;
				return *this;
			}

			ICustomPacket(size_t p) : param_c(p) {}
			std::unique_ptr<TCustomParam[]> params;
			virtual ~ICustomPacket() noexcept = default;
		};

		struct TCustomPacket final : public ICustomPacket {
			template<typename ... Parameters>
			explicit TCustomPacket(Parameters... prms) : ICustomPacket{ sizeof...(Parameters) } {
				params = std::make_unique<TCustomParam[]>(sizeof...(Parameters));
				assign_params(std::make_index_sequence<sizeof...(Parameters)>{}, prms...);

				const TCustomParam* arr = params.get();
				size_t last_idx = param_c - 1;
				for (size_t i = 0; i < param_c; ++i) {
					if (arr[i].size == 0 && last_idx > i) {
						throw std::invalid_argument("Unlimited length parameter must be last placed argument");
					}
				}
			}

		private:
			template<std::size_t... Is, typename... Parameters>
			void assign_params(std::index_sequence<Is...>, Parameters... prms) {
				((*(params.get() + Is) = std::move(prms)), ...);
			}
		};

		//these subheaders comes from mobile device of admin/gms to game server via bridge server
		enum class ECustomPackets : uint16_t {
			EXAMPLE_EVENT, //open, close, restart, adjust duration etc whatever you want.
			EXAMPLE_NOTIFICATION, //custom notification message to game
			EXAMPLE_COMPLEX, //tournament managament example
		};

		static inline std::unordered_map<ECustomPackets, std::unique_ptr<ICustomPacket>> g_custom_packets = [] {
			std::unordered_map<ECustomPackets, std::unique_ptr<ICustomPacket>> m;

			//add your packets like this
			/*
				m.emplace(
					ECustomPackets::PACKET_ID, //sub header id
					std::make_unique<TCustomPacket>(
						TCustomParam(PARAMETER_BYTE_SIZE, std::in_place_type<PARAMETER_TYPE>, "PARAMETER_DESCRIPTION"),
						TCustomParam(PARAMETER_BYTE_SIZE, std::in_place_type<PARAMETER_TYPE>, "PARAMETER_DESCRIPTION"),
						... other parameters
					)
				).first->second->
					AdjustDescription("Your packet description title which will shown in mobile app for you")
					.AdjustMinAuthority(MINIMUM_AUTHORITY_LEVEL_TO_SEND_THAT_PACKET, check EGMLevels in game);
			*/

			m.emplace(
				ECustomPackets::EXAMPLE_EVENT, //sub header id
				std::make_unique<TCustomPacket>(
					TCustomParam(2, std::in_place_type<uint16_t>, "Event id"), //parameters with expected sizes -> this is example of uint16_t[1]
					TCustomParam(1, std::in_place_type<bool>, "Open close setting")
				)
			).first->second->
				AdjustDescription("Change event status")
				.AdjustMinAuthority(consts::EGMAuthorityLevel::GM_HIGH_WIZARD);

			/*First example brief:
			* data<unsigned char> = {uint16_t(1) --event_id, bool(1) --open, close };
				forexample you want to open an event, you can open your mobile device as admin
				go admin panel, click send custom packet tab and select sub_id (ECustomPackets::EXAMPLE_EVENT)
				fill shown inputboxs and click 'Send Packet' button.
				Bridge server will receive that packet from mobile and validate it for you then the
				server will forward to mt server directly.
				In mt server you can handle packets with 'HandleForwardPacket' function in ../packet_handlers.cpp
			*/

			m.emplace(
				ECustomPackets::EXAMPLE_NOTIFICATION,
				std::make_unique<TCustomPacket>(
					TCustomParam(4, std::in_place_type<uint32_t>, "Player ID"), //player id 
					TCustomParam(32 + 1, std::in_place_type<char>, "Message title"), //title
					TCustomParam(0, std::in_place_type<char>, "Message body") //message (unlimited length) must be last parameter
				)
			).first->second->
				AdjustDescription("Send notification to player");

			m.emplace(
				ECustomPackets::EXAMPLE_COMPLEX,
				std::make_unique<TCustomPacket>(
					TCustomParam(4, std::in_place_type<uint32_t>, "Sub header id"), //extra sub header id 
					TCustomParam(0, std::in_place_type<uint8_t>, "extra data") //data (unlimited length)
				)
			).first->second->
				AdjustDescription("Random example!");

			return m;
			}();
	}
}