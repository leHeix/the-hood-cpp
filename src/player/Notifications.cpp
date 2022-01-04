#include "../main.hpp"

// https://easings.net/#easeInOutBack
float player::CNotificationManager::EaseInOutBack(float x)
{
	constexpr auto c1 = 1.70158f;
	constexpr auto c2 = c1 * 1.525f;

	return x < 0.5
	  ? (std::pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
	  : (std::pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
}

void player::CNotificationManager::MoveRight(timers::CTimer* timer, CPlayer* player, std::uint8_t idx)
{
	auto& notification = player->Notifications()->_notifications[idx];

	notification.delta += NOTIFICATION_DELTA;
	float t = (static_cast<float>(notification.delta) / 150.f);
	float x = std::lerp(0.f, 208.f, EaseInOutBack(t));


	auto* textdraws = textdraw_manager[fmt::format("notification_{}", idx)];
	auto& ptds = textdraws->GetPlayerTextDraws(player);
	ptds[0]->SetPosition({ (108.f - NOT_SUB_VAL) + x, 290.f - (46.f * idx) });
	ptds[1]->SetPosition({ (17.f - NOT_SUB_VAL) + x, 293.f - (46.f * idx) });
	ptds[2]->SetPosition({ (20.50f - NOT_SUB_VAL) + x, 293.f - (46.f * idx) });
	ptds[3]->SetPosition({ (29.60f - NOT_SUB_VAL) + x, 299.f - (46.f * idx) });
	ptds[4]->SetPosition({ (50.f - NOT_SUB_VAL) + x, 299.f - (46.f * idx) });

	textdraws->Show(player);

	if (t >= 1.0)
	{
		timer->Killed() = true;
		timers::timer_manager->Delete(timer->ID());

		timers::timer_manager->Repeat(player, notification.time, 8, &MoveLeft, idx);
	}
}

void player::CNotificationManager::MoveLeft(timers::CTimer* timer, CPlayer* player, std::uint8_t idx)
{
	auto* manager = player->Notifications();
	auto& notification = manager->_notifications[idx];

	notification.delta -= NOTIFICATION_DELTA;
	float t = (static_cast<float>(notification.delta) / 150.f);
	float x = std::lerp(208.f, 0.f, EaseInOutBack(t));

	auto* textdraws = textdraw_manager[fmt::format("notification_{}", idx)];

	if (t <= 0.0)
	{
		manager->_shown.flip(idx);
		textdraws->Hide(player);
		timer->Killed() = true;
		timers::timer_manager->Delete(timer->ID());

		return;
	}

	auto& ptds = textdraws->GetPlayerTextDraws(player);
	ptds[0]->SetPosition({ 108.f - x, 290.f - (46.f * idx) });
	ptds[1]->SetPosition({ 17.f - x, 293.f - (46.f * idx) });
	ptds[2]->SetPosition({ 20.50f - x, 293.f - (46.f * idx) });
	ptds[3]->SetPosition({ 29.60f - x, 299.f - (46.f * idx) });
	ptds[4]->SetPosition({ 50.f - x, 299.f - (46.f * idx) });

	while (!manager->_shown.all() && !manager->_pending.empty())
	{
		auto& v = manager->_pending.front();
		manager->Show(std::move(v.message), v.time);
		manager->_pending.pop();
	}

	textdraws->Show(player);
}

bool player::CNotificationManager::Show(const std::string& message, std::uint16_t time_ms)
{
	uint8_t shown_notifications = _shown.to_ulong();
	uint8_t idx = std::countr_one(shown_notifications);

	if (idx == MAX_NOTIFICATIONS)
	{
		notification_data data;
		data.message = message;
		data.time = time_ms;

		_pending.push(std::move(data));
		return false;
	}

	_shown.flip(idx);
	_notifications[idx].delta = 0;
	_notifications[idx].time = time_ms;

	server::TextDrawList* textdraws = textdraw_manager[fmt::format("notification_{}", idx)];
	
	float size = 0.208333f;
	for (size_t i = 0, j = message.length(); i < j; i += 45)
		size -= 0.015f;

	std::string split_message = message;
	server::textdraw::SplitTextDrawString(split_message, 122.5f, size, 1, 1, true);

	for (auto&& td : textdraws->GetPlayerTextDraws(_player))
	{
		auto pos = td->GetPosition();
		td->SetPosition({ pos.first - NOT_SUB_VAL, pos.second + (46.f * idx) });
	}

	textdraws->GetPlayerTextDraws(_player)[4]
		->SetLetterSize({ size, 1.f })
		->SetText(split_message);

	textdraws->Show(_player);

	timers::timer_manager->Repeat(_player, 8, 8, &MoveRight, idx);

	return true;
}
