#pragma once

class CKeyGame
{
	static constexpr float BAR_MAX_Y = 115.F;
	static constexpr float BAR_MIN_Y = 16.F;

	CPlayer* _player;
	std::function<void(CPlayer*, bool)> _callback;

	timers::CTimer* _decrease_bar_timer{ nullptr };
	timers::CTimer* _process_key_timer{ nullptr };
	int _current_key{ -1 };
	float _ppk{ 9.9f };
	std::chrono::steady_clock::time_point _last_key_appearance;
	float _decrease_sec{ 2.5f };
	float _current_size{ BAR_MIN_Y };
	bool _key_red{ false };

	void ProcessKey(timers::CTimer* timer, CPlayer* player);
	void DecreaseBar(timers::CTimer* timer, CPlayer* player);
public:
	static inline constexpr std::array<std::pair<std::string_view, int>, 5> random_keys = { {
		{ "~k~~CONVERSATION_YES~", KEY_YES },
		{ "~k~~CONVERSATION_NO~", KEY_NO },
		{ "~k~~GROUP_CONTROL_BWD~", KEY_CTRL_BACK },
		{ "~k~~PED_DUCK~", KEY_CROUCH },
		{ "~k~~GO_FORWARD~", KEY_UP }
	} };

	explicit CKeyGame(CPlayer* player) : _player(player)
	{
		textdraw_manager.LoadFile("keygame.toml", "keygame");
	}

	void Start(float key_percentage_up = 9.9F, float decrease_sec = 2.5F, std::function<void(CPlayer*, bool)> callback = nullptr);
	void Stop();
};
