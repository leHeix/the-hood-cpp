#include "../main.hpp"

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

void player::CNeedsManager::SetHunger(float hunger)
{
	_hunger = (hunger < 0.f ? 0.f : (hunger > 100.f ? 100.f : hunger));
	if (_bars_shown)
		UpdateTextDraws();
}

void player::CNeedsManager::SetThirst(float thirst)
{
	_thirst = (thirst < 0.f ? 0.f : (thirst > 100.f ? 100.f : thirst));
	if (_bars_shown)
		UpdateTextDraws();
}