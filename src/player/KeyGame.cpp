#include "../main.hpp"

void CKeyGame::ProcessKey(timers::CTimer* timer, CPlayer* player)
{
	if (_decrease_bar_timer == nullptr)
	{
		if (_current_size > BAR_MIN_Y)
		{
			using namespace std::placeholders;
			std::function<void(timers::CTimer*, CPlayer*)> fun = std::bind(&CKeyGame::DecreaseBar, this, _1, _2);
			_decrease_bar_timer = timers::timer_manager->Repeat(_player, 1000, 1000, fun);
		}
	}

	int keys, ud, lr;
	GetPlayerKeys(*_player, &keys, &ud, &lr);

	auto* textdraws = textdraw_manager["keygame"];
	int current_keycode = random_keys[_current_key].second;
	if ((current_keycode & keys) != 0 || ud == current_keycode || lr == current_keycode)
	{
		_key_red = false;
		_current_size = std::clamp(_current_size + _ppk, BAR_MIN_Y, BAR_MAX_Y);
		textdraws->GetPlayerTextDraws(_player)[0]->SetLineSize({ 298.500, _current_size });
		
		if (_current_size == BAR_MAX_Y)
		{
			Stop();
			if (_callback)
			{
				_callback(_player, true);
				_callback = nullptr;
			}
		}
		else
		{
			_current_key = Random::get(0u, random_keys.size() - 1);
			auto& key = random_keys[_current_key];
			textdraws->GetPlayerTextDraws(_player)[1]->SetText(std::string{ key.first.begin(), key.first.end() });
			_last_key_appearance = std::chrono::steady_clock::now();
		}
	}
	else
	{
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _last_key_appearance);
		if (!_key_red && duration >= std::chrono::milliseconds{ 5000 })
		{
			_key_red = true;
			textdraws->GetPlayerTextDraws(_player)[1]->SetText("~r~" + std::string{ random_keys[_current_key].first });
		}
		else if (_key_red && duration >= std::chrono::milliseconds{ 10000 })
		{
			Stop();
			if (_callback)
			{
				_callback(_player, false);
				_callback = nullptr;
			}
		}
	}
}

void CKeyGame::DecreaseBar(timers::CTimer* timer, CPlayer* player)
{
	_current_size = std::clamp(_current_size - _decrease_sec, BAR_MIN_Y, BAR_MAX_Y);
	textdraw_manager["keygame"]->GetPlayerTextDraws(_player)[0]->SetLineSize({ 298.500, _current_size });
}

void CKeyGame::Start(float key_percentage_up, float decrease_sec, std::function<void(CPlayer*, bool)> callback)
{
	_current_key = Random::get(0u, random_keys.size() - 1);
	auto& key = random_keys[_current_key];

	auto* textdraws = textdraw_manager["keygame"];
	textdraws->GetPlayerTextDraws(_player)[0]->SetLineSize({ 298.500, CKeyGame::BAR_MIN_Y });
	textdraws->GetPlayerTextDraws(_player)[1]->SetText(std::string{ key.first.begin(), key.first.end() });
	textdraws->GetGlobalTextDraws()[0]->Show(_player);
	textdraws->GetPlayerTextDraws(_player)[0]->Show();
	textdraws->Show(_player, 1, -1, -1, -1);
	textdraws->GetPlayerTextDraws(_player)[1]->Show();

	_current_size = BAR_MIN_Y;
	_callback = callback;
	_last_key_appearance = std::chrono::steady_clock::now();
	_decrease_sec = decrease_sec;
	_ppk = key_percentage_up;
	_decrease_bar_timer = nullptr;

	using namespace std::placeholders;
	std::function<void(timers::CTimer*, CPlayer*)> fun = std::bind(&CKeyGame::ProcessKey, this, _1, _2);
	_process_key_timer = timers::timer_manager->Repeat(_player, 200, 200, fun);
}

void CKeyGame::Stop()
{
	if (_decrease_bar_timer)
	{
		_decrease_bar_timer->Killed() = true;
		timers::timer_manager->Delete(_decrease_bar_timer->ID());
		_decrease_bar_timer = nullptr;
	}

	_process_key_timer->Killed() = true;
	timers::timer_manager->Delete(_process_key_timer->ID());
	_process_key_timer = nullptr;

	textdraw_manager["keygame"]->Hide(_player);
}
