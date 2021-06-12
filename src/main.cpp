/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <array>
#include <boost/asio/connect.hpp>
#include <cstdio>
#include <optional>

#include "client.hpp"

static std::array const infinitrains_deck{
	51126152U, 51126152U, 51126152U, 87074380U, 87074380U, 52481437U, 52481437U,
	52481437U, 13647631U, 13647631U, 13647631U, 97316367U, 76830505U, 24919805U,
	24919805U, 88875132U, 88875132U, 88875132U, 62034800U, 62034800U, 73628505U,
	83764719U, 25274141U, 25274141U, 25274141U, 34721681U, 34721681U, 34721681U,
	76136345U, 76136345U, 76136345U, 17427333U, 17427333U, 17427333U, 26708437U,
	26708437U, 26708437U, 51369889U, 51369889U, 82732705U, 90448279U, 26096328U,
	26096328U, 3814'632U, 49032236U, 49032236U, 49032236U, 56910167U, 56910167U,
	56910167U, 90162951U, 97584719U, 24701066U, 1'46'746U, 23689428U,
};

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> int
{
	auto const host = std::string_view("localhost");
	auto const port = std::string_view("7911");
	boost::asio::io_context io_context;
	std::optional<Client> client;
	try
	{
		boost::asio::ip::tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(host, port);
		boost::asio::ip::tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);
		client.emplace(
			std::move(socket),
			Client::Options{infinitrains_deck.data(), infinitrains_deck.size(),
		                    std::string_view()});
	}
	catch(std::exception& e)
	{
		std::fprintf(stderr, "Error while initializing client: %s\n", e.what());
		return 1;
	}
	io_context.run();
	return 0;
}
