#pragma once

class CFadeScreen
{
	mutable std::mutex _mtx;

	bool _in{ true };
	std::uint16_t _player_id;
	timers::CTimer* _timer;
	std::unique_ptr<server::PlayerTextDraw> _textdraw;
	std::function<void()> _callback;

public:
	explicit CFadeScreen(std::uint16_t playerid);
	~CFadeScreen();

	void Fade(unsigned char callback_alpha, std::function<void()> callback);
	void Stop();
	void Pause();
	void Resume();
};