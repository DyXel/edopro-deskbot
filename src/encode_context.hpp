/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_DESKBOT_ENCODE_CONTEXT_HPP
#define EDOPRO_DESKBOT_ENCODE_CONTEXT_HPP
#include <google/protobuf/arena.h>
#include <ygopen/client/board.hpp>
#include <ygopen/client/card.hpp>
#include <ygopen/client/frame.hpp>
#include <ygopen/client/parse_event.hpp>
#include <ygopen/client/parse_query.hpp>
#include <ygopen/client/value_wrappers.hpp>
#include <ygopen/codec/encode_common.hpp>

class EncodeContext final : public YGOpen::Codec::IEncodeContext
{
public:
	EncodeContext() noexcept;
	~EncodeContext() noexcept;

	auto pile_size(Con con, Loc loc) const noexcept -> size_t override;
	auto get_match_win_reason() const noexcept -> uint32_t override;
	auto has_xyz_mat(Place const& p) const noexcept -> bool override;
	auto get_xyz_left(Place const& left) const noexcept -> Place override;
	auto match_win_reason(uint32_t reason) noexcept -> void override;
	auto xyz_mat_defer(Place const& place) noexcept -> void override;
	auto take_deferred_xyz_mat() noexcept -> std::vector<Place> override;
	auto xyz_left(Place const& left, Place const& from) noexcept
		-> void override;

	auto parse(YGOpen::Proto::Duel::Msg const& msg) noexcept -> void;

private:
	template<typename T>
	using WrapperType = YGOpen::Client::Value<T>;
	using CardType = YGOpen::Client::BasicCard<WrapperType>;
	using FrameType = YGOpen::Client::BasicFrame<CardType>;
	using BoardType = YGOpen::Client::BasicBoard<FrameType, WrapperType>;
	BoardType board_;

	// Encoder context data.
	uint32_t match_win_reason_;
	std::map<Place, Place, YGOpen::Proto::Duel::PlaceLess> left_;
	std::vector<Place> deferred_;
};

#endif // EDOPRO_DESKBOT_ENCODE_CONTEXT_HPP
