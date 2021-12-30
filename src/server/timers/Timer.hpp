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
		mutable std::mutex _mutex;
		robin_hood::unordered_map<unsigned int, std::unique_ptr<CTimer>> _timers;
		unsigned int _max_id{ 0U };

	public:
		CTimer* Once(unsigned delay, std::function<void(CTimer*)> callback);
		CTimer* Repeat(unsigned delay_once, unsigned delay_repeat, std::function<void(CTimer*)> callback);
		void Delete(unsigned id);
		
		inline bool Exists(unsigned id) { return _timers.contains(id) && !_timers[id]->Killed(); }
	};

	extern std::unique_ptr<CTimerManager> timer_manager;
}