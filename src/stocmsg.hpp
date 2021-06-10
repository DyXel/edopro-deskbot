/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_FIREBOT_STOCMSG_HPP
#define EDOPRO_FIREBOT_STOCMSG_HPP
#include <array>

namespace YGOPro
{

class STOCMsg
{
public:
	using SizeType = uint16_t;
	using MsgType = uint8_t;

	static constexpr size_t HEADER_SIZE = sizeof(SizeType) + sizeof(MsgType);
	static constexpr size_t MAX_LENGTH = 1U << 14U;

	constexpr STOCMsg() noexcept : bytes_() {}

	constexpr auto header_data() noexcept -> uint8_t* { return bytes_.data(); }

	constexpr auto body_data() noexcept -> uint8_t*
	{
		return bytes_.data() + HEADER_SIZE;
	}

	constexpr auto body_size() const noexcept -> SizeType
	{
		SizeType r{};
		std::memcpy(&r, bytes_.data(), sizeof(SizeType));
		return r;
	}

	constexpr auto type() const noexcept -> MsgType
	{
		MsgType r{};
		std::memcpy(&r, bytes_.data() + sizeof(SizeType), sizeof(MsgType));
		return r;
	}

private:
	std::array<uint8_t, MAX_LENGTH> bytes_;
};

} // namespace YGOPro

#endif // EDOPRO_FIREBOT_STOCMSG_HPP
