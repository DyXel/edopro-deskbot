/*
 * Copyright (c) 2021, Finn <finnjuh@gmail.com>
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef PARSE_YDK_HPP
#define PARSE_YDK_HPP
#include <string>
#include <vector>

// NOTE: Doesn't parse side deck yet.
template<typename Stream>
inline auto parse_ydk(Stream& stream) noexcept -> std::vector<uint32_t>
{
	std::vector<uint32_t> deck;
	std::string line;
	while(std::getline(stream, line))
	{
		if(line.size() == 0U)
			continue;
		if(line[0U] == '!') // If side deck, end parse.
			break;
		if(std::isdigit(line[0U]))
			deck.push_back(static_cast<uint32_t>(std::stoull(line)));
	}
	return deck;
}

#endif // PARSE_YDK_HPP
