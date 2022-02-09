#pragma once

namespace player
{
	class CNotificationManager
	{
		static constexpr auto MAX_NOTIFICATIONS = 3;
		static constexpr auto NOTIFICATION_DELTA = 1;
		static constexpr auto NOT_LAST_POS_X = 108.0f;
		static constexpr auto NOT_INITIAL_POS_X = -100.0f;
		static constexpr auto NOT_SUB_VAL = 208.0f;
		static constexpr auto NOT_DISTANCE = 46.0f;

		static_assert(MAX_NOTIFICATIONS <= 8);

		struct notification_data
		{
			std::string message;
			std::uint16_t time;
		};

		struct notification
		{
			std::uint16_t time;
			std::uint16_t delta;
		};

		mutable std::mutex _mtx;
		CPlayer* _player;
		std::queue<notification_data> _pending;
		std::bitset<MAX_NOTIFICATIONS> _shown;
		std::array<notification, MAX_NOTIFICATIONS> _notifications;

		timers::CTimer* _beating_text_timer{ nullptr };
		uint8_t _beating_text_data{ 0u };
		std::chrono::steady_clock::time_point _beating_text_tick;

		static void ProcessBeatingText(timers::CTimer* timer, CPlayer* player, std::pair<uint8_t, uint8_t> alpha, std::uint16_t time);

		static void MoveRight(timers::CTimer* timer, CPlayer* player, std::uint8_t idx);
		static void MoveLeft(timers::CTimer* timer, CPlayer* player, std::uint8_t idx);

		static float EaseInOutBack(float x);
	public:
		explicit CNotificationManager(CPlayer* player)
			: _player(player)
		{
			for (size_t i = 0; i < MAX_NOTIFICATIONS; ++i)
			{
				textdraw_manager.LoadFile("notification", fmt::format("notification_{}", i));
			}
		}

		bool Show(const std::string& message, std::uint16_t time_ms);
		void ShowBeatingText(std::uint16_t time, std::uint32_t color, std::pair<std::uint8_t, std::uint8_t> alpha, const std::string& text);
	};
}
