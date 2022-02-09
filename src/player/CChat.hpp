#pragma once

namespace chat
{
	cell OnPlayerText(std::uint16_t playerid, std::string text);
}

class CChat
{
	friend cell chat::OnPlayerText(std::uint16_t playerid, std::string text);

	struct chat_message
	{
		std::uint32_t color;
		std::string message;
	};

	CPlayer* _player;
	std::deque<chat_message> _chatbuffer;
	bool _register_messages{ false };

	std::chrono::steady_clock::time_point _last_message;

	void PushMessage(std::uint32_t color, const std::string& message);
	std::vector<std::string> SplitChatMessage(const std::string& text, std::uint8_t max_line_length);

public:
	constexpr static std::size_t chatbuffer_size = 200U;
	constexpr static auto message_cooldown = std::chrono::milliseconds{ 500 };

	explicit CChat(CPlayer* player) : _player(player), _last_message(std::chrono::steady_clock::now())
	{
	}

	IO_GETTER_SETTER(LogMessages, _register_messages)

	void Send(std::uint32_t color, const std::string& message);

	template<class... Args>
	void Send(std::uint32_t color, fmt::format_string<Args...> message, Args&&... args)
	{
		std::string formatted = fmt::format(message, std::forward<Args>(args)...);

		BitStream bs;
		bs.Write<std::uint32_t>(color);
		bs.Write<std::uint32_t>(formatted.length());
		bs.Write(formatted.c_str(), formatted.length());
		net::RakServer->SendRPC(&bs, net::raknet::RPC_ClientMessage, _player->PlayerId());

		if (_register_messages)
		{
			PushMessage(color, std::move(formatted));
		}
	}

	void Resend();
	void Flush();
	void Clear();

	void SendRangedMessage(std::uint32_t color, float range, const std::string& text);
	void SendRangedMessage(std::uint32_t color, float range, const std::vector<std::string>& messages);
	void SendPlayerMessage(const std::string& text);
	void SendAction(const std::string& action);
	void SendEnvironment(const std::string& env);
	void SendOOC(const std::string& text);
};
