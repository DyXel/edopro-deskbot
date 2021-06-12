/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_FIREBOT_CTOSMSG_HPP
#define EDOPRO_FIREBOT_CTOSMSG_HPP
#include <array>
#include <cassert>
#include <cstddef> // size_t
#include <cstring> // std::memcpy
#include <tuple>
#include <type_traits>

#include "common_msg.hpp"

namespace YGOPro
{

constexpr size_t PASS_NAME_MAX_LENGTH = 20U;
constexpr size_t NOTES_MAX_LENGTH = 200U;

constexpr auto split_flags(uint64_t flags) noexcept
	-> std::tuple<uint32_t, uint32_t>
{
	constexpr uint32_t UINT32_MASK = 0xFFFFFFFFU;
	constexpr uint32_t UINT32_SHIFT = 32U;
	return {static_cast<uint32_t>(flags & UINT32_MASK),
	        static_cast<uint32_t>((flags >> UINT32_SHIFT) & UINT32_MASK)};
}

class CTOSMsg
{
public:
	using SizeType = int16_t;
	enum IdType : uint8_t
	{
		RESPONSE = 0x01U,
		UPDATE_DECK = 0x02U,
		RPS_CHOICE = 0x03U,
		TURN_CHOICE = 0x04U,
		PLAYER_INFO = 0x10U,
		CREATE_GAME = 0x11U,
		JOIN_GAME = 0x12U,
		// LEAVE_GAME    = 0x13U,
		// SURRENDER     = 0x14U, // Chad AI never surrenders!
		// TIME_CONFIRM  = 0x15U,
		CHAT = 0x16U,
		TO_DUELIST = 0x20U,
		// TO_OBSERVER   = 0x21U,
		READY = 0x22U,
		// NOT_READY     = 0x23U,
		// TRY_KICK      = 0x24U,
		TRY_START = 0x25U,
		REMATCH = 0xF0U,
	};

	struct RPSChoice
	{
		static constexpr auto ID = IdType::RPS_CHOICE;
		uint8_t value;
	};

	struct TurnChoice
	{
		static constexpr auto ID = IdType::TURN_CHOICE;
		uint8_t value;
	};

	struct PlayerInfo
	{
		static constexpr auto ID = IdType::PLAYER_INFO;
		std::array<uint16_t, PASS_NAME_MAX_LENGTH> name;
	};

	struct CreateGame
	{
		static constexpr auto ID = IdType::CREATE_GAME;
		HostInfo host_info;
		std::array<uint16_t, PASS_NAME_MAX_LENGTH> name;
		std::array<uint16_t, PASS_NAME_MAX_LENGTH> pass;
		std::array<char, NOTES_MAX_LENGTH> notes;
	};

	struct JoinGame
	{
		static constexpr auto ID = IdType::JOIN_GAME;
		uint16_t version2;
		uint32_t id;
		std::array<uint16_t, PASS_NAME_MAX_LENGTH> pass;
		ClientVersion version;
	};

	struct Ready
	{
		static constexpr auto ID = IdType::READY;
	};

	struct TryStart
	{
		static constexpr auto ID = IdType::TRY_START;
	};

	struct Rematch
	{
		static constexpr auto ID = IdType::REMATCH;
		uint8_t value;
	};

	static constexpr size_t HEADER_SIZE = sizeof(SizeType) + sizeof(IdType);
	static constexpr size_t MAX_LENGTH = 1U << 10U;

	template<typename T>
	static auto make_fixed(T t) noexcept -> CTOSMsg
	{
		static_assert(std::is_standard_layout_v<T>);
		IdType const id = T::ID;
		CTOSMsg msg(false);
		msg.write_body_size_(sizeof(T));
		std::memcpy(msg.bytes_.data() + sizeof(SizeType), &id, sizeof(id));
		std::memcpy(msg.bytes_.data() + sizeof(SizeType) + sizeof(IdType), &t,
		            sizeof(T));
		return msg;
	}

	static auto make_dynamic(IdType id) noexcept -> CTOSMsg
	{
		CTOSMsg msg(true);
		std::memcpy(msg.bytes_.data() + sizeof(SizeType), &id, sizeof(id));
		return msg;
	}

	[[nodiscard]] auto size() const noexcept -> size_t
	{
		return body_size_() + sizeof(SizeType) + sizeof(IdType);
	}

	[[nodiscard]] auto data() const noexcept -> uint8_t const*
	{
		// Make sure we've written something before attempting to access the
		// full buffer if message is dynamic.
		assert(!dynamic_ || body_size_() != 0U);
		return bytes_.data();
	}

	auto write(uint8_t const* ptr, size_t size) noexcept -> void
	{
		assert(dynamic_);
		assert(body_size_() + size <= MAX_LENGTH);
		auto const body_size = std::max(SizeType{0}, body_size_());
		std::memcpy(
			bytes_.data() + sizeof(SizeType) + sizeof(IdType) + body_size, ptr,
			size);
		write_body_size_(static_cast<SizeType>(body_size + size));
	}

	template<typename T>
	auto write(T t) noexcept -> void
	{
		static_assert(std::is_standard_layout_v<T>);
		assert(dynamic_);
		assert(body_size_() + sizeof(T) <= MAX_LENGTH);
		auto const body_size = std::max(SizeType{0}, body_size_());
		std::memcpy(
			bytes_.data() + sizeof(SizeType) + sizeof(IdType) + body_size, &t,
			sizeof(T));
		write_body_size_(static_cast<SizeType>(body_size + sizeof(T)));
	}

private:
	bool const dynamic_;
	std::array<uint8_t, MAX_LENGTH> bytes_;

	constexpr explicit CTOSMsg(bool dynamic) noexcept
		: dynamic_(dynamic), bytes_({0U})
	{}

	[[nodiscard]] auto body_size_() const noexcept -> SizeType
	{
		SizeType r{};
		std::memcpy(&r, bytes_.data(), sizeof(SizeType));
		return static_cast<SizeType>(r - sizeof(IdType));
	}

	auto write_body_size_(SizeType size) noexcept -> void
	{
		size += sizeof(IdType);
		std::memcpy(bytes_.data(), &size, sizeof(size));
	}
};

} // namespace YGOPro

#endif // EDOPRO_FIREBOT_CTOSMSG_HPP
