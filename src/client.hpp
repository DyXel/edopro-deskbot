/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_DESKBOT_CLIENT_HPP
#define EDOPRO_DESKBOT_CLIENT_HPP
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <queue>
#include <string_view>

#include "ctosmsg.hpp"
#include "stocmsg.hpp"

namespace Deskbot
{

class Core;

}

class EncodeContext;

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
	Client(Client&&) noexcept = delete;
	auto operator=(const Client&) -> Client& = delete;
	auto operator=(Client&&) noexcept -> Client& = delete;

private:
	YGOPro::STOCMsg incoming_;
	std::queue<YGOPro::CTOSMsg> outgoing_;
	boost::asio::ip::tcp::socket socket_;

	std::vector<uint32_t> deck_;
	bool hosting_;
	uint8_t t0_count_;
	uint8_t team_;
	uint8_t duelist_;

	std::string_view script_;

	std::vector<uint8_t> answer_buffer_;
	std::unique_ptr<Deskbot::Core> core_;
	std::unique_ptr<EncodeContext> ctx_;

	auto send_msg_(YGOPro::CTOSMsg msg) noexcept -> void;
	auto do_write_() noexcept -> void;

	auto do_read_header_() noexcept -> void;
	auto do_read_body_() noexcept -> void;

	auto handle_msg_() noexcept -> bool;
	auto analyze_(uint8_t const* buffer, size_t size) noexcept -> void;
};

#endif // EDOPRO_DESKBOT_CLIENT_HPP
