#include "encode_context.hpp"

EncodeContext::EncodeContext() noexcept : match_win_reason_(0)
{}

EncodeContext::~EncodeContext() noexcept = default;

auto EncodeContext::pile_size(Con con, Loc loc) const noexcept -> size_t
{
	return board_.frame().pile(con, loc).size();
}

auto EncodeContext::get_match_win_reason() const noexcept -> uint32_t
{
	return match_win_reason_;
}

auto EncodeContext::has_xyz_mat(Place const& p) const noexcept -> bool
{
	return !board_.frame().zone(p).materials.empty();
}

auto EncodeContext::get_xyz_left(Place const& left) const noexcept -> Place
{
	return left_.find(left)->second;
}

auto EncodeContext::match_win_reason(uint32_t reason) noexcept -> void
{
	match_win_reason_ = reason;
}

auto EncodeContext::xyz_mat_defer(Place const& place) noexcept -> void
{
	deferred_.emplace_back(place);
}

auto EncodeContext::take_deferred_xyz_mat() noexcept -> std::vector<Place>
{
	decltype(deferred_) taken{};
	std::swap(taken, deferred_);
	return taken;
}

auto EncodeContext::xyz_left(Place const& left, Place const& from) noexcept
	-> void
{
	left_[left] = from;
}

auto EncodeContext::parse(YGOpen::Proto::Duel::Msg const& msg) noexcept -> void
{
	if(msg.t_case() == YGOpen::Proto::Duel::Msg::kEvent)
		parse_event(board_, msg.event());
	for(auto const& query : msg.queries())
		parse_query(board_.frame(), query);
}
