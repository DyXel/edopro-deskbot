/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef EDOPRO_DESKBOT_ENCODE_CONTEXT_HPP
#define EDOPRO_DESKBOT_ENCODE_CONTEXT_HPP
#include <google/protobuf/arena.h>
#include <map>
#include <vector>
#include <ygopen/client/board.hpp>
#include <ygopen/client/card.hpp>
#include <ygopen/client/frame.hpp>
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
	auto xyz_left(Place const& left,
	              Place const& from) noexcept -> void override;

	auto parse(YGOpen::Proto::Duel::Msg const& msg) noexcept -> void;

private:
	struct CardTraits
	{
		using OwnerType = YGOpen::Duel::Controller;
		using IsPublicType = bool;
		using IsHiddenType = bool;
		using PositionType = YGOpen::Duel::Position;
		using StatusType = YGOpen::Duel::Status;
		using CodeType = uint32_t;
		using TypeType = YGOpen::Duel::Type;
		using LevelType = uint32_t;
		using XyzRankType = uint32_t;
		using AttributeType = YGOpen::Duel::Attribute;
		using RaceType = YGOpen::Duel::Race;
		using AtkDefType = int32_t;
		using PendScalesType = uint32_t;
		using LinkRateType = uint32_t;
		using LinkArrowType = YGOpen::Duel::LinkArrow;
		using CountersType = std::vector<YGOpen::Proto::Duel::Counter>;
		using EquippedType = YGOpen::Proto::Duel::Place;
		using RelationsType = std::vector<YGOpen::Proto::Duel::Place>;
	};

	using CardType = YGOpen::Client::BasicCard<CardTraits>;

	struct BoardTraits
	{
		using BlockedZonesType = std::vector<YGOpen::Proto::Duel::Place>;
		using ChainStackType = std::vector<YGOpen::Proto::Duel::Chain>;
		using FrameType = YGOpen::Client::BasicFrame<CardType>;
		using LPType = uint32_t;
		using PhaseType = YGOpen::Duel::Phase;
		using TurnControllerType = YGOpen::Duel::Controller;
		using TurnType = uint32_t;
	};

	using BoardType = YGOpen::Client::BasicBoard<BoardTraits>;

	BoardType board_;

	// Encoder context data.
	uint32_t match_win_reason_;
	std::map<Place, Place, YGOpen::Proto::Duel::PlaceLess> left_;
	std::vector<Place> deferred_;
};

#endif // EDOPRO_DESKBOT_ENCODE_CONTEXT_HPP
