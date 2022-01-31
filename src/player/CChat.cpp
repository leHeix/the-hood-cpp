#include "../main.hpp"

void CChat::PushMessage(std::uint32_t color, const std::string& message)
{
	if (_chatbuffer.size() == chatbuffer_size)
	{
		_chatbuffer.pop_front();
	}

	_chatbuffer.push_back(chat_message{ color, message });
}

std::vector<std::string> CChat::SplitChatMessage(const std::string& text, std::uint8_t max_line_length)
{
	if (text.length() > max_line_length)
	{
		std::vector<std::string> messages;
		messages.reserve(2);

		auto space = text.rfind(' ', max_line_length);
		if (space == std::string::npos)
		{
			messages.push_back(text.substr(0, --max_line_length) + "…");
			messages.push_back("…" + text.substr(max_line_length));
		}
		else
		{
			messages.push_back(text.substr(0, space));
			messages.push_back("— " + text.substr(space));
		}

		return messages;
	}
	else
	{
		return { text };
	}
}

void CChat::Send(std::uint32_t color, const std::string& message)
{
	BitStream bs;
	bs.Write<std::uint32_t>(color);
	bs.Write<std::uint32_t>(message.length());
	bs.Write(message.c_str(), message.length());
	net::RakServer->SendRPC(&bs, net::raknet::RPC_ClientMessage, _player->PlayerId());

	if (_register_messages)
	{
		PushMessage(color, message);
	}
}

void CChat::Resend()
{
	for(auto&& msg : _chatbuffer)
	{
		BitStream bs;
		bs.Write<uint32_t>(msg.color);
		bs.Write<uint32_t>(msg.message.length());
		bs.Write(msg.message.c_str(), msg.message.length());
		net::RakServer->SendRPC(&bs, net::raknet::RPC_ClientMessage, _player->PlayerId());
	}
}

void CChat::Flush()
{
	_chatbuffer.clear();
}

void CChat::Clear()
{
	BitStream bs;
	bs.Write<uint32_t>(0);
	bs.Write<uint32_t>(1);
	bs.Write(" ", 1);

	_chatbuffer.assign(chatbuffer_size, chat_message{ 0, " " });

	for (size_t i = 0; i < chatbuffer_size; ++i)
	{
		net::RakServer->SendRPC(&bs, net::raknet::RPC_ClientMessage, _player->PlayerId());
	}
}

void CChat::SendRangedMessage(std::uint32_t color, float range, const std::string& text)
{
	auto& pos = _player->Position();

	for (auto&& [id, player] : server::player_pool)
	{
		float distance = GetPlayerDistanceFromPoint(id, pos.x, pos.y, pos.z);
		if (distance > range)
			continue;

		int alpha = (255 - (distance * 3.0));
#define RGBToHex(r,g,b) (0xFF | ((b) << 8) | ((g) << 16) | ((r) << 24))
#define Darken(col,alpha) ((col) & RGBToHex(alpha,alpha,alpha))

		player->Chat()->Send(Darken(color, alpha), text);

#undef RGBToHex
#undef Darken
	}
}

void CChat::SendRangedMessage(std::uint32_t color, float range, const std::vector<std::string>& messages)
{
	auto& pos = _player->Position();

	for (auto&& [id, player] : server::player_pool)
	{
		float distance = GetPlayerDistanceFromPoint(id, pos.x, pos.y, pos.z);
		if (distance > range)
			continue;

		int alpha = (255 - (distance * 3.0));
#define RGBToHex(r,g,b) (0xFF | ((b) << 8) | ((g) << 16) | ((r) << 24))
#define Darken(col,alpha) ((col) & RGBToHex(alpha,alpha,alpha))

		for (auto&& text : messages)
		{
			player->Chat()->Send(Darken(color, alpha), text);
		}

#undef RGBToHex
#undef Darken
	}
}

void CChat::SendPlayerMessage(const std::string& text)
{
	std::string final_msg = fmt::format("{{{:X}}}{}{{FFFFFF}}", static_cast<uint32_t>(GetPlayerColor(_player->PlayerId())) >> 8, _player->Name());
	if (GetPlayerDrunkLevel(_player->PlayerId()) > 2000)
		final_msg += " alcoholizado dice: ";
	else
		final_msg += " dice: ";

	auto max_length = 128 - final_msg.length();
	auto messages = SplitChatMessage(text, max_length);
	messages.front().insert(0, final_msg);

	SendRangedMessage(0xFFFFFFFF, 15.f, messages);

	std::string chatbubble(text);
	if (chatbubble.length() > 50)
	{
		chatbubble.insert(47, "...");
		chatbubble[50] = '\0';
	}

	SetPlayerChatBubble(*_player, chatbubble.c_str(), 0xFFFFFFFF, 15.f, 5000);
}

void CChat::SendAction(const std::string& action)
{
	std::string message_start = fmt::format("* {} ", _player->Name());
	auto messages = SplitChatMessage(action, 128 - message_start.length());
	messages[0].insert(0, message_start);
	SendRangedMessage(0xC157EBFF, 15.f, messages);

	std::string chatbubble("* " + action);
	if (chatbubble.length() > 50)
	{
		chatbubble.insert(47, "...");
		chatbubble[50] = '\0';
	}

	SetPlayerChatBubble(*_player, chatbubble.c_str(), 0xC157EBFF, 15.f, 5000);
}

void CChat::SendEnvironment(const std::string& env)
{
	auto messages = SplitChatMessage(env, 122 - _player->Name().length());
	messages.back() += fmt::format(" (( {} ))", _player->Name());
	SendRangedMessage(0x46C759FF, 15.f, messages);

	std::string chatbubble("(( " + env);
	if (chatbubble.length() > 50)
	{
		chatbubble.insert(47, " ))");
		chatbubble[50] = '\0';
	}

	SetPlayerChatBubble(*_player, chatbubble.c_str(), 0x46C759FF, 15.f, 5000);
}

void CChat::SendOOC(const std::string& text)
{
	std::string final_msg = fmt::format("{{{:X}}}{}{{FFFFFF}}: (( ", (uint32_t)GetPlayerColor(_player->PlayerId()) >> 8, _player->Name());

	auto max_length = 128 - final_msg.length() - 3;
	auto messages = SplitChatMessage(text, max_length);
	messages.front().insert(0, final_msg);

	SendRangedMessage(0xEEEEEEFF, 15.f, messages);

	std::string chatbubble("(( " + text);
	if (chatbubble.length() > 50)
	{
		chatbubble.insert(47, " ))");
		chatbubble[50] = '\0';
	}

	SetPlayerChatBubble(*_player, chatbubble.c_str(), 0xEEEEEEFF, 15.f, 5000);
}


cell chat::OnPlayerText(std::uint16_t playerid, std::string text)
{
	auto* player = server::player_pool[playerid];
	if (!player->Flags().test(player::flags::in_game))
		return 0;

	auto time_since = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player->Chat()->_last_message);
	if (time_since < CChat::message_cooldown)
	{
		constexpr auto messages_per_sec = (1000 / CChat::message_cooldown.count());
		auto msg = fmt::format(FMT_COMPILE("Solo puedes enviar {{ED2B2B}}{} mensaje{}{{DADADA}} cada segundo."), messages_per_sec, (messages_per_sec > 1 ? "s" : ""));
		player->Chat()->Send(0xDADADAFF, msg);
		return 0;
	}

	std::replace(text.begin(), text.end(), '%', '#');
	player->Chat()->SendPlayerMessage(text);

	return 0;
}

static CPublicHook<chat::OnPlayerText> _c_opt("OnPlayerText");

// - Chat Commands
using namespace std::string_view_literals;

command me_cmd("me", { "y"sv }, [](CPlayer* player, commands::argument_store args) {
	std::string text;

	try
	{
		args >> commands::argument_store::final >> text;
	}
	catch (const std::exception& e)
	{
		player->Chat()->Send(0xDADADAFF, "USO: {ED2B2B}/me{DADADA} <acción>");
		return;
	}

	player->Chat()->SendAction(text);
});

command do_cmd("do", { "p"sv }, [](CPlayer* player, commands::argument_store args) {
	std::string text;

	try
	{
		args >> commands::argument_store::final >> text;
	}
	catch (const std::exception& e)
	{
		player->Chat()->Send(0xDADADAFF, "USO: {ED2B2B}/do{DADADA} <entorno>");
		return;
	}

	player->Chat()->SendEnvironment(text);
});

command ooc_cmd("ooc"sv, { "b"sv }, [](CPlayer* player, commands::argument_store args) {
	std::string text;

	try
	{
		args >> commands::argument_store::final >> text;
	}
	catch (const std::exception& e)
	{
		player->Chat()->Send(0xDADADAFF, "USO: {ED2B2B}/ooc{DADADA} <texto>");
		return;
	}

	player->Chat()->SendOOC(text);
});
