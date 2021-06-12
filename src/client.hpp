/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_FIREBOT_CLIENT_HPP
#define EDOPRO_FIREBOT_CLIENT_HPP
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <queue>
#include <string_view>

#include "ctosmsg.hpp"
#include "stocmsg.hpp"

namespace Firebot
{

class Core;

}

class Client
{
public:
	struct Options
	{
		uint32_t const* deck_ptr;
		size_t deck_size;
		std::string_view script;
	};

	Client(boost::asio::ip::tcp::socket socket, Options const& options);
	~Client();

	Client(const Client&) = delete;
	constexpr Client(Client&&) noexcept = delete;
	auto operator=(const Client&) -> Client& = delete;
	constexpr auto operator=(Client&&) noexcept -> Client& = delete;

private:
	YGOPro::STOCMsg incoming_;
	std::queue<YGOPro::CTOSMsg> outgoing_;
	boost::asio::ip::tcp::socket socket_;

	std::vector<uint32_t> deck_;
	std::string script_;
	bool hosting_;
	uint8_t t0_count_;
	uint8_t team_;
	uint8_t duelist_;

	std::unique_ptr<Firebot::Core> core_;

	auto send_msg_(YGOPro::CTOSMsg msg) noexcept -> void;
	auto do_write_() noexcept -> void;

	auto do_read_header_() noexcept -> void;
	auto do_read_body_() noexcept -> void;

	auto handle_msg_() noexcept -> bool;
};

#endif // EDOPRO_FIREBOT_CLIENT_HPP
