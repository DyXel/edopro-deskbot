/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <array>
#include <boost/asio/connect.hpp>
#include <cstdio>
#include <fstream>
#include <google/protobuf/stubs/common.h>
#include <optional>

#include "client.hpp"
#include "load_script.hpp"
#include "parse_ydk.hpp"

auto main(int argc, char* argv[]) -> int
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	struct _
	{
		~_() { google::protobuf::ShutdownProtobufLibrary(); }
	} on_exit;
	if(argc != 3)
	{
		std::fprintf(stderr, "You need to pass a ydk file as 2nd arg.\n");
		std::fprintf(stderr, "You need to pass a script file as 3rd arg.\n");
		return 1;
	}
	boost::asio::io_context io_context;
	std::optional<Client> client;
	try
	{
		auto const d = [&]()
		{
			auto f = std::ifstream{argv[1]};
			return parse_ydk(f);
		}();
		auto const host = std::string_view("localhost");
		auto const port = std::string_view("7911");
		boost::asio::ip::tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(host, port);
		boost::asio::ip::tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);
		client.emplace(
			std::move(socket),
			Client::Options{d.data(), d.size(), std::string_view(argv[2])});
	}
	catch(std::exception& e)
	{
		std::fprintf(stderr, "Error while initializing client: %s\n", e.what());
		return 1;
	}
	io_context.run();
	return 0;
}
