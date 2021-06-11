/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_FIREBOT_CLIENT_HPP
#define EDOPRO_FIREBOT_CLIENT_HPP
#include <boost/asio/ip/tcp.hpp>
#include <firebot/api.hpp>

#include "stocmsg.hpp"

class Client
{
public:
	Client(boost::asio::ip::tcp::socket socket,
	       Firebot::Core::Options const& core_opts);

private:
	YGOPro::STOCMsg incoming_;
	boost::asio::ip::tcp::socket socket_;
	Firebot::Core core_;

	auto do_read_header_() noexcept -> void;
	auto do_read_body_() noexcept -> void;

	auto handle_msg_() noexcept -> bool;
};

#endif // EDOPRO_FIREBOT_CLIENT_HPP
