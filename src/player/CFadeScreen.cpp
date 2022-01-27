#include "../main.hpp"

CFadeScreen::CFadeScreen(std::uint16_t playerid)
	:	_player_id(playerid),
		_textdraw(std::make_unique<server::PlayerTextDraw>(_player_id))
{
	_textdraw->SetPosition({ 317.000000, -56.000000 })
		->SetFont(1)
		->SetAlignment(2)
		->SetLetterSize({ 0.600000, 124.850006 })
		->SetLineSize({ 298.500000, 1236.000000 })
		->SetLetterColor(-1)
		->SetBackgroundColor(255)
		->SetBoxColor(195)
		->ToggleBox(true);
}

CFadeScreen::~CFadeScreen()
{
	Stop();
}

void CFadeScreen::Fade(unsigned char callback_alpha, std::function<void()> callback)
{
	std::scoped_lock lk(_mtx);

	if (_textdraw->Shown())
	{
		Stop();
	}

	_in = true;
	_textdraw->SetBoxColor(0);
	_textdraw->Show();

	_timer = timers::timer_manager->Repeat(20, 20, [=,this](timers::CTimer* timer) {
		std::uint8_t alpha = _textdraw->GetBoxColor();
		
		if (alpha == callback_alpha)
		{
			callback();
		}

		if (alpha == 255)
			_in = false;
		else if (alpha == 0 && !_in)
		{
			Stop();
			return;
		}

		_textdraw->SetBoxColor((_in ? alpha + 5 : alpha - 5));
	});
}

void CFadeScreen::Stop()
{
	if (_timer)
	{
		_timer->Killed() = true;
		timers::timer_manager->Delete(_timer->ID());
	}

	_textdraw->Hide();
}

void CFadeScreen::Pause()
{
	std::scoped_lock lk(_mtx);
	_timer->Pause();
}

void CFadeScreen::Resume()
{
	std::scoped_lock lk(_mtx);
	_timer->Resume();
}