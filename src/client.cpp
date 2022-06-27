/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "client.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>
#include <deskbot/api.hpp>
#include <google/protobuf/arena.h>
#include <ygopen/codec/edo9300_ocgcore_decode.hpp>
#include <ygopen/codec/edo9300_ocgcore_encode.hpp>
#include <ygopen/proto/duel/answer.hpp>

#include "encode_context.hpp"
#include "load_script.hpp"

constexpr size_t ANSWER_BUFFER_RESERVE = 1U << 8U;
constexpr uint32_t HANDSHAKE = 4043399681U;
constexpr auto CLIENT_VERSION = YGOPro::ClientVersion{{39U, 3U}, {9U, 1U}};

auto log_cb(void*, Deskbot::LogType lt, std::string_view str) noexcept -> void
{
	std::fprintf(stderr, "[%i] %s\n", static_cast<int>(lt), str.data());
}

Client::Client(boost::asio::ip::tcp::socket socket, Options const& options)
	: socket_(std::move(socket))
	, deck_(options.deck_ptr, options.deck_ptr + options.deck_size)
	, hosting_(true)
	, t0_count_(0)
	, team_(0U)
	, duelist_(0)
	, script_(options.script)
{
	answer_buffer_.reserve(ANSWER_BUFFER_RESERVE);
	{
		auto player_info = YGOPro::CTOSMsg::PlayerInfo{};
		player_info.name[0U] = L'虚';
		send_msg_(YGOPro::CTOSMsg::make_fixed(player_info));
	}
	if(hosting_)
	{
		static constexpr uint64_t DUEL_FLAGS = 4295157760U;
		auto create_game = YGOPro::CTOSMsg::CreateGame{};
		auto& hi = create_game.host_info;
		hi.banlist_hash = 186804830U; // "2022.5 TCG"
		hi.allowed = 0x3U;            // "OCG/TCG"
		hi.starting_draw_count = 5U;
		hi.draw_count_per_turn = 1U;
		hi.duel_flags_high = DUEL_FLAGS >> 32U;
		hi.handshake = HANDSHAKE;
		hi.version = CLIENT_VERSION;
		hi.duel_flags_low = DUEL_FLAGS & 0xFFFFFFFFU;
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

Client::~Client() = default;

auto Client::send_msg_(YGOPro::CTOSMsg msg) noexcept -> void
{
	// std::is_trivial seems to be bugged in Visual Studio and that carries on
	// to all the functions relying on it. Disable this check when building with
	// Visual Studio.
#ifndef _MSC_VER
	// No need for std::move as long as type is trivially copyable.
	static_assert(std::is_trivially_copyable_v<YGOPro::CTOSMsg>);
#endif
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
				std::fprintf(stderr, "do_write_: %s.\n", ec.message().data());
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
				std::fprintf(stderr, "do_read_header_: %s.\n",
			                 ec.message().data());
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
				std::fprintf(stderr, "do_read_body_: %s.\n",
			                 ec.message().data());
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
	case STOCMsg::IdType::GAME_MSG:
	{
		analyze_(incoming_.body_data(), incoming_.body_size());
		return true;
	}
	case STOCMsg::IdType::ERROR:
	{
		if(incoming_.body_size() == sizeof(STOCMsg::Error))
		{
			auto const error = incoming_.as_fixed<STOCMsg::Error>();
			std::fprintf(stderr, "Server reported error 0x%X and code %u.\n",
			             error.msg, error.code);
		}
		else // incoming_.body_size() == sizeof(STOCMsg::DeckError)
		{
			auto const deck_error = incoming_.as_fixed<STOCMsg::DeckError>();
			std::fprintf(stderr, "Deck error 0x%X with code %u.\n",
			             deck_error.msg, deck_error.code);
		}
		return false;
	}
	case STOCMsg::IdType::CHOOSE_RPS:
	{
		send_msg_(CTOSMsg::make_fixed(CTOSMsg::RPSChoice{1U}));
		return true;
	}
	case STOCMsg::IdType::CHOOSE_ORDER:
	{
		auto turn_choice = CTOSMsg::TurnChoice{0U};
		if(auto const first = core_->wants_first_turn(); first.has_value())
		{
			turn_choice.value = static_cast<uint8_t>(*first);
		}
		else
		{
			// Indifferent. Randomly decide.
			// TODO.
		}
		send_msg_(CTOSMsg::make_fixed(turn_choice));
		return true;
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
			std::fprintf(stderr, "Room is full. Bailing out.\n");
			return false;
		}
		team_ = static_cast<uint8_t>(index > t0_count_ - 1U);
		duelist_ = (index > t0_count_ - 1U) ? index - t0_count_ : index;
		{
			auto msg = CTOSMsg::make_dynamic(CTOSMsg::IdType::UPDATE_DECK);
			msg.write(static_cast<uint32_t>(deck_.size()));
			msg.write<uint32_t>(0U); // No sidedeck for now.
			for(auto card_code : deck_)
				msg.write<uint32_t>(card_code);
			send_msg_(msg);
		}
		send_msg_(CTOSMsg::make_fixed(CTOSMsg::Ready{}));
		return true;
	}
	case STOCMsg::IdType::DUEL_START:
	{
		core_ = std::make_unique<Deskbot::Core>(
			Deskbot::Core::Options{log_cb, nullptr, load_script, nullptr});
		core_->process_script(script_, load_script(nullptr, script_));
		core_->call_initialize();
		ctx_ = std::make_unique<EncodeContext>();
		return true;
	}
	case STOCMsg::IdType::DUEL_END:
	{
		std::printf("All duels ended. Good Bye!\n");
		return false;
	}
	case STOCMsg::IdType::PLAYER_CHANGE:
	{
		auto const player_change = incoming_.as_fixed<STOCMsg::PlayerChange>();
		bool const ready = (player_change.value & 0xFU) == 0x9U; // NOLINT
		if(ready && hosting_)
			send_msg_(CTOSMsg::make_fixed(CTOSMsg::TryStart{}));
		return true;
	}
	case STOCMsg::IdType::REMATCH:
	{
		send_msg_(CTOSMsg::make_fixed(CTOSMsg::Rematch{1U}));
		return true;
	}
	default:
	{
		std::printf("Unknown message 0x%X, with size %i.\n",
		            static_cast<unsigned int>(incoming_.type()),
		            incoming_.body_size());
		return true;
	}
	}
}

auto Client::analyze_(uint8_t const* buffer, size_t size) noexcept -> void
{
	auto analyze_and_answer = [&](YGOpen::Proto::Duel::Msg const& msg)
	{
		ctx_->parse(msg);
		core_->analyze(msg);
		if(msg.t_case() != YGOpen::Proto::Duel::Msg::kRequest)
			return;
		auto const& req = msg.request();
		auto const answer = core_->answer(req);
		using namespace YGOpen::Codec;
		Edo9300::OCGCore::decode_one_answer(req, answer, answer_buffer_);
		assert(!answer_buffer_.empty());
		auto ctosmsg = YGOPro::CTOSMsg::make_dynamic(YGOPro::CTOSMsg::RESPONSE);
		ctosmsg.write(answer_buffer_.data(), answer_buffer_.size());
		send_msg_(ctosmsg);
	};
	// NOTE: Assuming the server is sending one game message at the time.
	google::protobuf::Arena arena;
	using namespace YGOpen::Codec;
	uint8_t const core_msg = *buffer;
	assert(core_msg != 1U); // NOLINT: MSG_RETRY
	if(core_msg == 3U)      // NOLINT: MSG_WAITING
		return;
	auto const r = Edo9300::OCGCore::encode_one(arena, *ctx_, buffer);
	switch(r.state)
	{
	case EncodeOneResult::State::OK:
	{
		analyze_and_answer(*r.msg);
		break;
	}
	case EncodeOneResult::State::UNKNOWN:
	{
		std::fprintf(stderr, "Regular encoding failed: %i.\n", core_msg);
		return;
	}
	default:
		break;
	}
	assert(r.bytes_read == size);
}
