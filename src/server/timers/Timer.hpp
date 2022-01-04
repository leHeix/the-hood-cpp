#pragma once

namespace timers
{
	class CTimer
	{
		uv_timer_t* _handle;
		std::function<void(CTimer*)> _callback;
		unsigned _time;
		std::optional<unsigned> _repeat;
		unsigned _id{ 0U };
		unsigned _paused_time{ 0U };
		bool _killed{ false };
		bool _calling{ false };

		static void Callback(uv_timer_t* handle);
	public:
		CTimer(uv_loop_t* loop, unsigned int time, const std::optional<unsigned int>& repeat_time, std::function<void(CTimer*)> callback);
		~CTimer();

		void Start();
		void Pause();
		void Resume();
		void Stop();

		IO_GETTER_SETTER(ID, _id)
		IO_GETTER_SETTER(Killed, _killed)
	};

	class CTimerManager
	{
		friend cell OnPlayerDisconnect(std::uint16_t playerid, std::uint8_t reason);

		mutable std::mutex _mutex;
		robin_hood::unordered_map<unsigned int, std::unique_ptr<CTimer>> _timers;
		std::unordered_multimap<const CPlayer*, unsigned int> _player_timers_map;
		std::unordered_map<unsigned int, const CPlayer*> _player_timers_reverse_map;
		unsigned int _max_id{ 0U };

	public:
		CTimer* Once(unsigned delay, std::function<void(CTimer*)> callback);
		CTimer* Repeat(unsigned delay_once, unsigned delay_repeat, std::function<void(CTimer*)> callback);
		
		CTimer* Once(CPlayer* player, unsigned delay, std::function<void(CTimer*, CPlayer*)> callback);

		template<class... Args>
		CTimer* Repeat(CPlayer* player, unsigned delay_once, unsigned delay_repeat, void(*callback)(CTimer*, CPlayer*, Args...), Args... args)
		{
			using namespace std::placeholders;

			std::scoped_lock<std::mutex> lk(_mutex);

			std::function<void(CTimer*)> fun = std::bind(callback, _1, player, args...);

			auto timer = std::make_unique<CTimer>(uv_default_loop(), delay_once, delay_repeat, std::move(fun));
			timer->ID() = ++_max_id;
			timer->Start();

			_timers[_max_id] = std::move(timer);
			_player_timers_map.insert({ player, _max_id });

			return _timers[_max_id].get();
		}

		void Delete(unsigned id);

		inline bool Exists(unsigned id) { return _timers.contains(id) && !_timers[id]->Killed(); }
	};

	extern std::unique_ptr<CTimerManager> timer_manager;
}