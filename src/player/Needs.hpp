#pragma once

namespace player
{
	class CNeedsManager
	{
		enum needs_timers : std::uint8_t {
			thirst_update,
			hunger_update,

			max_timers
		};

		CPlayer* _player;
		float _hunger{ 0.f };
		float _thirst{ 0.f };
		bool _bars_shown{ false };

		std::array<timers::CTimer*, needs_timers::max_timers> _timers;
	
		static void UpdateThirst(timers::CTimer* timer, CPlayer* player);
		static void UpdateHunger(timers::CTimer* timer, CPlayer* player);
		void UpdateTextDraws();
	public:
		explicit CNeedsManager(CPlayer* player)
			: _player(player)
		{
			textdraw_manager.LoadFile("needs", "needs");
		}

		void StartUpdating();
		void StopUpdating();
		void ShowBars();

		inline float Hunger() const noexcept { return _hunger; }
		inline float Thirst() const noexcept { return _thirst; }
		void SetHunger(float hunger);
		void SetThirst(float thirst);
	};
}