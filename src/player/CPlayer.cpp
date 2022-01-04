#include "../main.hpp"

CPlayer::CPlayer(std::uint16_t playerid)
	:	_playerid(playerid),
		_fadescreen(std::make_unique<CFadeScreen>(_playerid)),
		_notifications(std::make_unique<player::CNotificationManager>(this))
{
	_ip_address.resize(16);
	GetPlayerIp(playerid, _ip_address.data(), 16);
	_ip_address.erase(_ip_address.find('\0'));

	_name.resize(24);
	GetPlayerName(playerid, _name.data(), 24);
	_name.erase(_name.find('\0'));
}

void CPlayer::RegisterConnection()
{
	auto stmt = server::database->Prepare(
		"INSERT INTO `CONNECTION_LOGS` "
			"(ACCOUNT_ID, IP_ADDRESS) "
		"VALUES " 
			"(?, ?);"
	);

	stmt->Bind<1>(_account_id);
	stmt->Bind<2>(_ip_address);

	stmt->Step();
}

void CPlayer::ClearChat(unsigned char lines)
{
	while (lines--)
	{
		SendClientMessage(_playerid, 0, " ");
	}
}

void CPlayer::ToggleWidescreen()
{
	_widescreen = !_widescreen;

	BitStream bs;
	bs.Write<bool>(_widescreen);
	net::RakServer->SendRPC(&bs, net::raknet::RPC_Widescreen, _playerid);
}

void CPlayer::ToggleWidescreen(bool set)
{
	_widescreen = set;
	BitStream bs;
	bs.Write<bool>(_widescreen);
	net::RakServer->SendRPC(&bs, net::raknet::RPC_Widescreen, _playerid);
}

void CPlayer::CancelTextDrawSelection()
{
	_cancel_td_tick = std::chrono::steady_clock::now();
	CancelSelectTextDraw(_playerid);
}

void CPlayer::ResetMoney()
{
	_money = 0;
	ResetPlayerMoney(_playerid);
}

void CPlayer::GiveMoney(int money, bool give, bool update)
{
	_money += money;
	if (give)
	{
		GivePlayerMoney(_playerid, money);
	}

	if (update)
	{
		uv_work_t* work = new uv_work_t;
		work->data = this;
		uv_queue_work(uv_default_loop(), work, [](uv_work_t* handle) {
			CPlayer* player = static_cast<CPlayer*>(handle->data);

			auto stmt = server::database->Prepare("UPDATE `PLAYERS` SET `MONEY` = ? WHERE `ID` = ? LIMIT 1;");
			stmt->Bind<1>(player->GetMoney());
			stmt->Bind<2>(player->AccountId());
			stmt->Step();
		}, 
		[](uv_work_t* h, int status) {
			delete h;
		});
	}
}

void CPlayer::SetMoney(int money, bool give, bool update)
{
	_money = money;
	if (give)
	{
		GivePlayerMoney(_playerid, money);
	}

	if (update)
	{
		uv_work_t* work = new uv_work_t;
		work->data = this;
		uv_queue_work(uv_default_loop(), work, [](uv_work_t* handle) {
			CPlayer* player = static_cast<CPlayer*>(handle->data);

			auto stmt = server::database->Prepare("UPDATE `PLAYERS` SET `MONEY` = ? WHERE `ID` = ? LIMIT 1;");
			stmt->Bind<1>(player->GetMoney());
			stmt->Bind<2>(player->AccountId());
			stmt->Step();
		},
		[](uv_work_t* h, int status) {
			delete h;
		});
	}
}

void CPlayer::ShowDialog(unsigned char style, const std::string_view caption, const std::string_view info, const std::string_view button1, const std::string_view button2, std::optional<dialog_callback_t> callback)
{
	_dialog_shown = true;
	_dialog_callback = callback;
	ShowPlayerDialog(_playerid, 0x1A6, style, caption.data(), info.data(), button1.data(), button2.data());
}

cell PlayerDialog_OnDialogResponse(std::uint16_t playerid, short dialogid, bool response, int listitem, std::string inputtext)
{
	if (dialogid != 0x1A6)
		return 0;

	auto* player = server::player_pool[playerid];
	if (!player->DialogVisible())
		return ~0;

	std::replace(inputtext.begin(), inputtext.end(), '%', '#');

	// friend function
	auto cb = player->_dialog_callback.value_or(nullptr);
	player->_dialog_callback = std::nullopt;
	player->_dialog_shown = false;

	if(cb)
		cb(player, response, listitem, std::move(inputtext));

	return 1;
}

static CPublicHook<PlayerDialog_OnDialogResponse> _dialogs_odr("OnDialogResponse");

static cell PlayerPool_OnPlayerDisconnect(std::uint16_t playerid, unsigned char reason)
{
	server::player_pool.Remove(playerid);
	return 1;
}

static CPublicHook<PlayerPool_OnPlayerDisconnect> _pp_opd("OnPlayerDisconnect");