#include "../main.hpp"
#include "Needs.hpp"

void player::CNeedsManager::UpdateThirst(timers::CTimer* timer, CPlayer* player)
{

}

void player::CNeedsManager::UpdateHunger(timers::CTimer* timer, CPlayer* player)
{

}

void player::CNeedsManager::UpdateTextDraws()
{
	auto* textdraws = textdraw_manager["needs"];

	constexpr float thirst_full = 596.500f;
	constexpr float thirst_empty = 505.f;
	constexpr float hunger_full = 510.500f;
	constexpr float hunger_empty = 608.f;

	float thirst_coords = thirst_empty + (_thirst * 0.915f);
	if (thirst_coords > thirst_full)
		thirst_coords = thirst_full;

	float hunger_coords = hunger_empty - (_hunger * 0.975f);
	if (hunger_coords < hunger_full)
		hunger_coords = hunger_full;

	auto& hunger_bar = textdraws->GetPlayerTextDraws(_player)[0];
	auto& thirst_bar = textdraws->GetPlayerTextDraws(_player)[1];

	hunger_bar->SetLineSize({ hunger_coords, 0.f });
	thirst_bar->SetLineSize({ thirst_coords, 0.f });
}

void player::CNeedsManager::StartUpdating()
{
	_timers[needs_timers::thirst_update] = timers::timer_manager->Repeat(_player, 60000, 60000, &UpdateThirst);
	_timers[needs_timers::hunger_update] = timers::timer_manager->Repeat(_player, 120000, 120000, &UpdateHunger);
}

void player::CNeedsManager::StopUpdating()
{
	_timers[needs_timers::thirst_update]->Killed() = true;
	_timers[needs_timers::hunger_update]->Killed() = true;
	timers::timer_manager->Delete(_timers[needs_timers::thirst_update]->ID());
	timers::timer_manager->Delete(_timers[needs_timers::hunger_update]->ID());
}

void player::CNeedsManager::ShowBars()
{
	_bars_shown = true;
	UpdateTextDraws();
	
	auto* textdraws = textdraw_manager["needs"];
	auto& global = textdraws->GetGlobalTextDraws();

	global[0]->Show(_player);
	global[1]->Show(_player);
	global[2]->Show(_player);
	global[3]->Show(_player);

	textdraws->GetPlayerTextDraws(_player)[0]->Show();

	global[4]->Show(_player);
	global[5]->Show(_player);
	global[6]->Show(_player);
	global[7]->Show(_player);
	global[8]->Show(_player);

	textdraws->GetPlayerTextDraws(_player)[1]->Show();

	for (std::uint8_t i = 9; i < textdraws->GetGlobalTextDraws().size(); ++i)
		textdraws->GetGlobalTextDraws()[i]->Show(_player);
}

void player::CNeedsManager::HideBars()
{
	_bars_shown = false;
	textdraw_manager["needs"]->Hide(_player);
}

void player::CNeedsManager::Puke()
{
	_player->StopShopping();
	_eat_count = 0u;
	_last_puke_tick = std::chrono::steady_clock::now();

	_player->Flags().set(player::flags::is_puking, true);

	// Start player puke anim
	_player->SetFacingAngle(0.f);
	auto& pos = _player->Position();
	ApplyAnimation(*_player, "FOOD", "null", 4.1F, false, false, false, false, 0, false);
	ApplyAnimation(*_player, "FOOD", "EAT_VOMIT_P", 4.0F, false, false, false, true, 0, false);
	PlayerPlaySound(*_player, 1169, pos.x, pos.y, pos.z);

	timers::timer_manager->Once(_player, 4000, [](timers::CTimer*, CPlayer* player) {
		auto& pos = player->Position();
		auto object = CreateObject(18722, pos.x + 0.355F, pos.y - 0.116F, pos.z - 1.6F, 0.F, 0.F, 0.F, 0.F);

		timers::timer_manager->Once(player, 3500, [object](timers::CTimer*, CPlayer* player) {
			player->Flags().set(player::flags::is_puking, false);
			DestroyObject(object);
			ClearAnimations(*player, false);
			PlayerPlaySound(*player, 0, 0.F, 0.F, 0.F);
		});
	});
}

void player::CNeedsManager::SetHunger(float hunger)
{
	_hunger = std::clamp(hunger, 0.F, 100.F);
	if (_bars_shown)
		UpdateTextDraws();
}

void player::CNeedsManager::SetThirst(float thirst)
{
	_thirst = std::clamp(thirst, 0.F, 100.F);
	if (_bars_shown)
		UpdateTextDraws();
}

void player::CNeedsManager::GiveHunger(float hunger)
{
	_hunger += std::clamp(hunger, 0.F, 100.F);
	if (_bars_shown)
		UpdateTextDraws();
}

void player::CNeedsManager::GiveThirst(float thirst)
{
	_thirst += std::clamp(thirst, 0.F, 100.F);
	if (_bars_shown)
		UpdateTextDraws();
}
