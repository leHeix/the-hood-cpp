#pragma once

namespace player
{
	constexpr auto MAX_NOTIFICATIONS = 3;
	constexpr auto NOTIFICATION_DELTA = 1;
	constexpr auto NOT_LAST_POS_X = 108.0f;
	constexpr auto NOT_INITIAL_POS_X = -100.0f;
	constexpr auto NOT_SUB_VAL = 208.0f;
	constexpr auto NOT_DISTANCE = 46.0f;

	static_assert(MAX_NOTIFICATIONS <= 8);

	struct notification_data
	{
		std::string message;
		std::uint16_t time;
	};

	class CNotificationManager
	{
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
	};
}