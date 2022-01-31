#include "../main.hpp"

CPlayer::CPlayer(std::uint16_t playerid)
	:	_playerid(playerid),
		_fadescreen(std::make_unique<CFadeScreen>(_playerid)),
		_notifications(std::make_unique<player::CNotificationManager>(this)),
		_needs(std::make_unique<player::CNeedsManager>(this)),
		_chat(std::make_unique<CChat>(this))
{
	_ip_address.resize(16);
	GetPlayerIp(playerid, _ip_address.data(), 16);
	_ip_address.erase(_ip_address.find('\0'));

	_name.resize(24);
	GetPlayerName(playerid, _name.data(), 24);
	_name.erase(_name.find('\0'));

	_last_command = std::chrono::steady_clock::now();
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

bool CPlayer::Spawned() const
{
	switch (GetPlayerState(_playerid))
	{
		case PLAYER_STATE_ONFOOT:
		case PLAYER_STATE_DRIVER:
		case PLAYER_STATE_PASSENGER:
		case PLAYER_STATE_SPAWNED:
			return true;
	}

	return false;
}

void CPlayer::SetPosition(const glm::vec3& pos)
{
	_position.x = pos.x;
	_position.y = pos.y;
	_position.z = pos.z;
	SetPlayerPos(_playerid, pos.x, pos.y, pos.z);
}

void CPlayer::SetPosition(const glm::vec4& pos)
{
	_position = pos;
	SetPlayerPos(_playerid, pos.x, pos.y, pos.z);
	SetPlayerFacingAngle(_playerid, pos.w);
}

void CPlayer::SetFacingAngle(float angle)
{
	_position.w = angle;
	SetPlayerFacingAngle(_playerid, angle);
}

void CPlayer::StopShopping()
{
	_flags.set(player::flags::using_shop, false);
	_flags.set(player::flags::can_use_shop_buttons, false);
	
	SetCameraBehindPlayer(_playerid);
	TogglePlayerControllable(_playerid, true);
	textdraw_manager["shop"]->Hide(this);
	CancelTextDrawSelection();
	DestroyPlayerObject(_playerid, shop_manager->PlayerData(_playerid).object);

	shop_manager->PlayerData(_playerid) = {};
	_shop = nullptr;
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

			auto stmt = server::database->Prepare("UPDATE `PLAYERS` SET `MONEY` = ? WHERE `ID` = ?;");
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

			auto stmt = server::database->Prepare("UPDATE `PLAYERS` SET `MONEY` = ? WHERE `ID` = ?;");
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

static cell Account_OnPlayerDisconnect(std::uint16_t playerid, std::uint8_t reason)
{
 	auto* player = server::player_pool[playerid];
	if (!player->Flags().test(player::flags::registered))
		return 1;

	if (!player->Flags().test(player::flags::in_game))
		return 1;

	if (player->Spawned())
	{
		GetPlayerPos(player->PlayerId(), &player->Position().x, &player->Position().z, &player->Position().z);
		GetPlayerFacingAngle(player->PlayerId(), &player->Position().w);
		player->VirtualWorld() = GetPlayerVirtualWorld(player->PlayerId());
		player->Interior() = GetPlayerInterior(player->PlayerId());
	}

	try
	{
		auto stmt = server::database->Prepare(
			"UPDATE `PLAYERS` SET "
				"`PLAYED_TIME` = (`PLAYED_TIME` + (strftime('%s', 'now') - `CURRENT_CONNECTION`)) - ?, "
				"`POS_X` = ?, "
				"`POS_Y` = ?, "
				"`POS_Z` = ?, "
				"`ANGLE` = ?, "
				"`VW` = ?, "
				"`INTERIOR` = ?, "
				"`HUNGER` = ?, "
				"`THIRST` = ?, "
				"`SKIN` = ?, "
				"`CURRENT_CONNECTION` = 0 "
			"WHERE `ID` = ?;"
		);

		stmt->Bind<1>(player->PausedTime()); // PLAYED_TIME
		stmt->Bind<2>(player->Position().x); // POS_X
		stmt->Bind<3>(player->Position().y); // POS_Y
		stmt->Bind<4>(player->Position().z); // POS_Z
		stmt->Bind<5>(player->Position().w); // ANGLE
		stmt->Bind<6>(player->VirtualWorld()); // VW
		stmt->Bind<7>(player->Interior()); // INTERIOR
		stmt->Bind<8>(player->Needs()->Hunger()); // HUNGER
		stmt->Bind<9>(player->Needs()->Thirst()); // THIRST
		stmt->Bind<10>(player->Skin()); // SKIN
		stmt->Bind<11>(player->AccountId()); // where ID

		stmt->Step();
	}
	catch (const std::runtime_error& e)
	{
		sampgdk::logprintf("[Account] Couldn't save player %i data: %s", playerid, e.what());
	}

	return 1;
}

static CPublicHook<Account_OnPlayerDisconnect> _acc_opd("OnPlayerDisconnect");