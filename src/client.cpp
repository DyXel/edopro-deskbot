/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "client.hpp"

#include <boost/asio/read.hpp>
#include <iostream>

Client::Client(boost::asio::ip::tcp::socket socket,
               Firebot::Core::Options const& core_opts)
	: socket_(std::move(socket)), core_(core_opts)
{
	do_read_header_();
}

auto Client::do_read_header_() noexcept -> void
{
	auto b = boost::asio::buffer(incoming_.header_data(),
	                             YGOPro::STOCMsg::HEADER_SIZE);
	boost::asio::async_read(
		socket_, b,
		[this](boost::system::error_code ec, size_t /*unused*/)
		{
			if(ec)
			{
				std::cerr << "do_read_header_: " << ec.message() << '\n';
				return;
			}
			do_read_body_();
		});
}

auto Client::do_read_body_() noexcept -> void
{
	// TODO: Check if following socket read would be too long to store and
	// report accordingly.
	auto b = boost::asio::buffer(incoming_.body_data(), incoming_.body_size());
	boost::asio::async_read(
		socket_, b,
		[this](boost::system::error_code ec, size_t /*unused*/)
		{
			if(ec)
			{
				std::cerr << "do_read_body_: " << ec.message() << '\n';
				return;
			}
			if(handle_msg_())
				do_read_header_();
		});
}

auto Client::handle_msg_() noexcept -> bool
{
	switch(incoming_.type())
	{
	case YGOPro::STOCMsg::IdType::ERROR_MSG:
	{
		auto const error_msg = incoming_.as_fixed<YGOPro::STOCMsg::ErrorMsg>();
		std::cerr << "Server reported error with type ";
		std::cerr << static_cast<int>(error_msg.msg) << ' ';
		std::cerr << "and code " << error_msg.code << '\n';
		return false;
	}
	default:
	{
		std::cout << "Unknown message type ";
		std::cout << static_cast<int>(incoming_.type());
		std::cout << " with size " << incoming_.body_size() << '\n';
		break;
	}
	}
	return true;
}
