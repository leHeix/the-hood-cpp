#pragma once

namespace player
{
	// libtards get rekt !
	enum sex : std::uint8_t {
		male = 0,
		female = 1
	};

	enum flags : std::uint8_t {
		authenticating,
		registered,
		in_game,
		customizing_player,
		using_shop,
		can_use_shop_buttons,
		is_puking,
		on_auto_dealership,
		has_drink_on_hands,

		max_player_flags
	};

	enum rank : unsigned char {
		user,
		assistant,
		moderator,
		global_operator,
		admin
	};
}

class CPlayer
{
public:
	using dialog_callback_t = std::function<void(CPlayer*, bool, unsigned char, std::string)>;

private:
	unsigned short _playerid{ 0U };
	unsigned int _account_id{ 0U };
	int _money{ 0 };

	// Managers
	std::unique_ptr<CFadeScreen> _fadescreen;
	server::TextDrawIndexManager _td_indexer{};
	
	std::string _ip_address;
	std::string _name;
	std::string _password_hash;
	std::string _last_connection;
	std::bitset<player::flags::max_player_flags> _flags;

	// Character data
	unsigned char _age;
	bool _sex;
	float _health{ 100.f };
	float _armor{ 0.f };
	glm::vec4 _position;
	int _virtual_world{ 0 };
	unsigned char _interior{ 0 };
	int _skin{ 0 };
	float _hunger{ 0.f };
	float _thirst{ 0.f };
	player::rank _rank_level{ player::rank::user };
	int _phone_number{ 0 };
	int _played_time{ 0 };
	bool _widescreen{ false };

	// Dialogs
	std::optional<dialog_callback_t> _dialog_callback{ std::nullopt };
	bool _dialog_shown{ false };

	std::unordered_map<std::string, std::any> _player_data{};
	std::optional<std::chrono::steady_clock::time_point> _cancel_td_tick{ std::nullopt };

	friend cell PlayerDialog_OnDialogResponse(std::uint16_t playerid, short dialogid, bool response, int listitem, std::string inputtext);
	friend bool PLUGIN_CALL OnPublicCall(AMX* amx, const char* name, cell* params, cell* retval);
	friend cell auth::OnPlayerConnect(std::uint16_t playerid);
public:
	explicit CPlayer(unsigned short id);

	inline unsigned short PlayerId() const noexcept { return _playerid; }

	void ResetMoney();
	void GiveMoney(int money, bool give = true, bool update = true);
	void SetMoney(int money, bool give = true, bool update = true);
	inline int GetMoney() const { return _money; }

	// Player Functions
	void ClearChat(unsigned char lines = 20);
	void ToggleWidescreen();
	void ToggleWidescreen(bool set);
	inline bool WidescreenEnabled() const noexcept { return _widescreen; }
	void CancelTextDrawSelection();

	// Dialogs
	void ShowDialog(unsigned char style, const std::string_view caption, const std::string_view info, const std::string_view button1, const std::string_view button2, std::optional<dialog_callback_t> callback = std::nullopt);
	inline bool DialogVisible() const { return _dialog_shown; }

	inline auto* FadeScreen() noexcept { return _fadescreen.get(); }
	inline const auto* FadeScreen() const noexcept { return _fadescreen.get(); }

	IO_GETTER_SETTER(TextDraws, _td_indexer)
	IO_GETTER_SETTER(AccountId, _account_id)
	IO_GETTER_SETTER(Flags, _flags)
	IO_GETTER_SETTER(Name, _name)
	IO_GETTER_SETTER(IP, _ip_address)
	IO_GETTER_SETTER(Password, _password_hash)
	IO_GETTER_SETTER(Age, _age)
	IO_GETTER_SETTER(Sex, _sex)
	IO_GETTER_SETTER(Health, _health)
	IO_GETTER_SETTER(Armor, _armor)
	IO_GETTER_SETTER(Position, _position)
	IO_GETTER_SETTER(VirtualWorld, _virtual_world)
	IO_GETTER_SETTER(Interior, _interior)
	IO_GETTER_SETTER(LastConnection, _last_connection)
	IO_GETTER_SETTER(Skin, _skin)
	IO_GETTER_SETTER(Hunger, _hunger)
	IO_GETTER_SETTER(Thirst, _thirst)
	IO_GETTER_SETTER(Rank, _rank_level)
	IO_GETTER_SETTER(PlayedTime, _played_time)
	IO_GETTER_SETTER(PhoneNumber, _phone_number)

	// Custom data manipulation
	template<class T>
	void SetData(const std::string& key, const T& value) { _player_data[key] = std::any{ value }; }
	inline bool HasData(const std::string& key) { return _player_data.contains(key); }
	inline void RemoveData(const std::string& key) { _player_data.erase(key); }
	template<class T>
	std::optional<T> GetData(const std::string& key) 
	{ 
		auto it = _player_data.find(key);
		if (it == _player_data.end())
			return std::nullopt;

		try
		{
			return std::any_cast<T>(it->second);
		}
		catch (const std::bad_any_cast& e)
		{
			return std::nullopt;
		}
	}
};