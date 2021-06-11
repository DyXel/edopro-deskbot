/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_FIREBOT_COMMON_MSG_HPP
#define EDOPRO_FIREBOT_COMMON_MSG_HPP
#include <cstdint> // uint8_t, uint16_t, ...

namespace YGOPro
{

struct ClientVersion
{
	struct
	{
		uint8_t major;
		uint8_t minor;
	} client, core;
};

struct HostInfo
{
	uint32_t banlist_hash;
	uint8_t allowed;
	uint8_t mode;      // NOTE: UNUSED
	uint8_t duel_rule; // NOTE: UNUSED
	uint8_t dont_check_deck;
	uint8_t dont_shuffle_deck;
	uint32_t starting_lp;
	uint8_t starting_draw_count;
	uint8_t draw_count_per_turn;
	uint16_t time_limit_in_seconds;
	uint32_t duel_flags_high;
	uint32_t handshake;
	ClientVersion version;
	int32_t t0_count;
	int32_t t1_count;
	int32_t best_of;
	uint32_t duel_flags_low;
	int32_t forbidden_types;
	uint16_t extra_rules;
};

} // namespace YGOPro

#endif // EDOPRO_FIREBOT_COMMON_MSG_HPP
