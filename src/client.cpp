/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "client.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <iostream>

constexpr uint32_t HANDSHAKE = 4043399681U;
constexpr auto CLIENT_VERSION = YGOPro::ClientVersion{{39U, 1U}, {9U, 0U}};

Client::Client(boost::asio::ip::tcp::socket socket,
               Firebot::Core::Options const& core_opts)
	: socket_(std::move(socket))
	, core_(core_opts)
	, hosting_(false)
	, t0_count_(0)
	, team_(0U)
	, duelist_(0)
{
	{
		auto player_info = YGOPro::CTOSMsg::PlayerInfo{};
		player_info.name[0U] = L'虚';
		send_msg_(YGOPro::CTOSMsg::make_fixed(player_info));
	}
	if(hosting_)
	{
		auto create_game = YGOPro::CTOSMsg::CreateGame{};
		create_game.host_info.handshake = HANDSHAKE;
		create_game.host_info.version = CLIENT_VERSION;
		send_msg_(YGOPro::CTOSMsg::make_fixed(create_game));
	}
	else
	{
		auto join_game = YGOPro::CTOSMsg::JoinGame{};
		join_game.id = 1U;
		join_game.version = CLIENT_VERSION;
		send_msg_(YGOPro::CTOSMsg::make_fixed(join_game));
	}
	do_read_header_();
}

auto Client::send_msg_(YGOPro::CTOSMsg&& msg) noexcept -> void
{
	const bool write_in_progress = !outgoing_.empty();
	outgoing_.emplace(msg);
	if(!write_in_progress)
		do_write_();
}

auto Client::do_write_() noexcept -> void
{
	auto const& msg = outgoing_.front();
	auto b = boost::asio::buffer(msg.data(), msg.size());
	boost::asio::async_write(
		socket_, b,
		[this](boost::system::error_code ec, size_t /*unused*/)
		{
			if(ec)
			{
				std::cerr << "do_write_: " << ec.message() << '\n';
				return;
			}
			outgoing_.pop();
			if(!outgoing_.empty())
				do_write_();
		});
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
	using namespace YGOPro;
	switch(incoming_.type())
	{
	case STOCMsg::IdType::ERROR_MSG:
	{
		auto const error_msg = incoming_.as_fixed<STOCMsg::ErrorMsg>();
		std::cerr << "Server reported error with type ";
		std::cerr << static_cast<int>(error_msg.msg) << ' ';
		std::cerr << "and code " << error_msg.code << '\n';
		return false;
	}
	case STOCMsg::IdType::JOIN_GAME:
	{
		auto const join_game = incoming_.as_fixed<STOCMsg::JoinGame>();
		t0_count_ = join_game.host_info.t0_count;
		return true;
	}
	case STOCMsg::IdType::TYPE_CHANGE:
	{
		auto const type_change = incoming_.as_fixed<STOCMsg::TypeChange>();
		uint8_t index = (type_change.value & 0xFU); // NOLINT
		if(index > 6U)                              // NOLINT
		{
			std::cout << "Room is full. Bailing out.\n";
			return false;
		}
		team_ = static_cast<uint8_t>(index > t0_count_ - 1U);
		duelist_ = (index > t0_count_ - 1U) ? index - t0_count_ : index;
		return true;
	}
	default:
	{
		std::cout << "Unknown message type ";
		std::cout << static_cast<int>(incoming_.type());
		std::cout << " with size " << incoming_.body_size() << '\n';
		return true;
	}
	}
}
