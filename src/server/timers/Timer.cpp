#include "../../main.hpp"

std::unique_ptr<timers::CTimerManager> timers::timer_manager = std::make_unique<timers::CTimerManager>();

timers::CTimer::CTimer(uv_loop_t* loop, unsigned int time, const std::optional<unsigned int>& repeat_time, std::function<void(CTimer*)> callback)
	: _handle(new uv_timer_t), _time(time), _callback(std::move(callback)), _repeat(repeat_time)
{
	uv_timer_init(loop, _handle);
	_handle->data = static_cast<void*>(this);
	
	uv_update_time(loop);
}

timers::CTimer::~CTimer()
{
	sampgdk::logprintf("[Timers] Killed timer %p", static_cast<void*>(this));

	Stop();

	if (_calling)
		_handle->data = nullptr;
	else
		delete _handle;

	if (timer_manager->Exists(_id))
	{
		_killed = true;
		timer_manager->Delete(_id);
	}
}

void timers::CTimer::Callback(uv_timer_t* handle)
{
	auto* timer = static_cast<CTimer*>(handle->data);

	timer->_calling = true;

	timer->_callback(timer);

	if (handle->data == nullptr)
	{
		delete handle;
		return;
	}

	timer->_calling = false;

	if (!timer->_repeat)
	{
		timer->_killed = true;
		timer_manager->Delete(timer->ID());
	}
}

void timers::CTimer::Start()
{
	uv_timer_start(_handle, Callback, _time, (_repeat ? *_repeat : 0));
}

void timers::CTimer::Pause()
{
	_paused_time = uv_timer_get_due_in(_handle);
	uv_timer_stop(_handle);
}

void timers::CTimer::Resume()
{
	if (!_paused_time)
		return;

	uv_timer_start(_handle, Callback, _paused_time, (_repeat ? *_repeat : 0));
}

void timers::CTimer::Stop()
{
	uv_timer_stop(_handle);
}

timers::CTimer* timers::CTimerManager::Once(unsigned delay, std::function<void(CTimer*)> callback)
{
	std::scoped_lock<std::mutex> lk(_mutex);

	auto timer = std::make_unique<CTimer>(uv_default_loop(), delay, std::nullopt, std::move(callback));
	timer->ID() = ++_max_id;
	timer->Start();

	_timers[_max_id] = std::move(timer);

	return _timers[_max_id].get();
}

timers::CTimer* timers::CTimerManager::Repeat(unsigned delay_once, unsigned delay_repeat, std::function<void(CTimer*)> callback)
{
	std::scoped_lock<std::mutex> lk(_mutex);

	auto timer = std::make_unique<CTimer>(uv_default_loop(), delay_once, delay_repeat, std::move(callback));
	timer->ID() = ++_max_id;
	timer->Start();

	_timers[_max_id] = std::move(timer);

	return _timers[_max_id].get();
}

timers::CTimer* timers::CTimerManager::Once(CPlayer* player, unsigned delay, std::function<void(CTimer*, CPlayer*)> callback)
{
	std::scoped_lock<std::mutex> lk(_mutex);

	std::function<void(CTimer*)> fun = std::bind(callback, std::placeholders::_1, player);

	auto timer = std::make_unique<CTimer>(uv_default_loop(), delay, std::nullopt, std::move(fun));
	timer->ID() = ++_max_id;
	timer->Start();

	_timers[_max_id] = std::move(timer);
	_player_timers_map.insert({ player, _max_id });

	return _timers[_max_id].get();
}

timers::CTimer* timers::CTimerManager::Repeat(CPlayer* player, unsigned delay_once, unsigned delay_repeat, std::function<void(CTimer*, CPlayer*)> callback)
{
	std::scoped_lock<std::mutex> lk(_mutex);

	std::function<void(CTimer*)> fun = std::bind(callback, std::placeholders::_1, player);

	auto timer = std::make_unique<CTimer>(uv_default_loop(), delay_once, delay_repeat, std::move(fun));
	timer->ID() = ++_max_id;
	timer->Start();

	_timers[_max_id] = std::move(timer);
	_player_timers_map.insert({ player, _max_id });

	return _timers[_max_id].get();
}

/*
timers::CTimer* timers::CTimerManager::Repeat(CPlayer* player, unsigned delay_once, unsigned delay_repeat, std::function<void(CTimer*, CPlayer*)> callback)
{
	std::scoped_lock<std::mutex> lk(_mutex);

	std::function<void(CTimer*)> fun = std::bind(callback, std::placeholders::_1, player);

	auto timer = std::make_unique<CTimer>(uv_default_loop(), delay_once, delay_repeat, std::move(fun));
	timer->ID() = ++_max_id;
	timer->Start();

	_timers[_max_id] = std::move(timer);
	_player_timers_map.insert({ player, _max_id });

	return _timers[_max_id].get();
}
*/

void timers::CTimerManager::Delete(unsigned id)
{
	std::scoped_lock<std::mutex> lk(_mutex);
	_timers.erase(id);

	// what the FUCK
	// black excellence as some may say
	if (_player_timers_reverse_map.contains(id))
	{
		auto rng = _player_timers_map.equal_range(_player_timers_reverse_map[id]);
		for (auto it = rng.first; it != rng.second; ++it)
		{
			if (it->second == id)
			{
				_player_timers_map.erase(it);
				break;
			}
		}
	}
}

void timers::CTimerManager::Delete(CTimer* timer)
{
	Delete(timer->ID());
}

cell timers::OnPlayerDisconnect(std::uint16_t playerid, std::uint8_t reason)
{
	auto rg = timer_manager->_player_timers_map.equal_range(server::player_pool[playerid]);
	for (auto it = rg.first; it != rg.second; ++it)
	{
		timer_manager->Delete(it->second);
	}

	return 1;
}
