/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_FIREBOT_STOCMSG_HPP
#define EDOPRO_FIREBOT_STOCMSG_HPP
#include <array>
#include <cassert>
#include <cstddef> // size_t
#include <cstring> // std::memcpy

#include "common_msg.hpp"

namespace YGOPro
{

class STOCMsg
{
public:
	using SizeType = uint16_t;
	enum class IdType : uint8_t
	{
		GAME_MSG = 0x1,
		ERROR_MSG = 0x2,
		CHOOSE_RPS = 0x3,
		CHOOSE_ORDER = 0x4,
		// RPS_RESULT    = 0x5,
		ORDER_RESULT = 0x6,
		// CHANGE_SIDE   = 0x7, // TODO: Are we going to handle side decking?
		// WAITING_SIDE  = 0x8,
		// CREATE_GAME   = 0x11,
		JOIN_GAME = 0x12,
		TYPE_CHANGE = 0x13,
		// LEAVE_GAME    = 0x14,
		DUEL_START = 0x15,
		DUEL_END = 0x16,
		// REPLAY        = 0x17,
		// TIME_LIMIT    = 0x18,
		PLAYER_ENTER = 0x20,
		PLAYER_CHANGE = 0x21,
		// WATCH_CHANGE  = 0x22,
		// NEW_REPLAY    = 0x30,
		// CATCHUP       = 0xF0,
		REMATCH = 0xF1,
		// REMATCH_WAIT  = 0xF2,
		CHAT_2 = 0xF3,
	};

	static constexpr size_t HEADER_SIZE = sizeof(SizeType) + sizeof(IdType);
	static constexpr size_t MAX_LENGTH = 1U << 14U;

	struct ErrorMsg
	{
		static constexpr auto ID = IdType::ERROR_MSG;
		uint8_t msg;
		uint32_t code;
	};

	struct JoinGame
	{
		static constexpr auto ID = IdType::JOIN_GAME;
		HostInfo host_info;
	};

	struct TypeChange
	{
		static constexpr auto ID = IdType::TYPE_CHANGE;
		uint8_t value;
	};

	struct PlayerChange
	{
		static constexpr auto ID = IdType::PLAYER_CHANGE;
		uint8_t value;
	};

	constexpr STOCMsg() noexcept : bytes_() {}

	[[nodiscard]] auto body_data() const noexcept -> uint8_t const*
	{
		return bytes_.data() + HEADER_SIZE;
	}

	[[nodiscard]] auto body_size() const noexcept -> SizeType
	{
		SizeType r{};
		std::memcpy(&r, bytes_.data(), sizeof(SizeType));
		return r - 1U;
	}

	[[nodiscard]] auto type() const noexcept -> IdType
	{
		IdType r{};
		std::memcpy(&r, bytes_.data() + sizeof(SizeType), sizeof(IdType));
		return r;
	}

	template<typename T>
	[[nodiscard]] auto as_fixed() const noexcept -> T
	{
		static_assert(std::is_standard_layout_v<T>);
		assert(T::ID == type());
		assert(body_size() == sizeof(T));
		T r{};
		std::memcpy(&r, body_data(), sizeof(T));
		return r;
	}

	auto header_data() noexcept -> uint8_t* { return bytes_.data(); }

	auto body_data() noexcept -> uint8_t*
	{
		return bytes_.data() + HEADER_SIZE;
	}

private:
	std::array<uint8_t, MAX_LENGTH> bytes_;
};

} // namespace YGOPro

#endif // EDOPRO_FIREBOT_STOCMSG_HPP
