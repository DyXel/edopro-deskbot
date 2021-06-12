/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <boost/asio/connect.hpp>
#include <cstdio>
#include <string_view>

#include "client.hpp"

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
		client.emplace(std::move(socket), Firebot::Core::Options{});
	}
	catch(std::exception& e)
	{
		std::fprintf(stderr, "Error while initializing client: %s\n", e.what());
		return 1;
	}
	io_context.run();
	return 0;
}
