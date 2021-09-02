/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "load_script.hpp"

#include <fstream>

auto load_script(void*, std::string_view name) noexcept -> std::string
{
	constexpr auto read_size = std::size_t{4096};
	auto stream = std::ifstream{name.data()};
	stream.exceptions(std::ios_base::badbit);
	auto out = std::string{""};
	auto buf = std::string(read_size, '\0');
	while(stream.read(&buf[0], read_size))
		out.append(buf, 0, stream.gcount());
	out.append(buf, 0, stream.gcount());
	return out;
}
