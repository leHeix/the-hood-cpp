#include "../../main.hpp"

namespace auth
{
	static constexpr unsigned short AdjustSkinToRange(unsigned char age, unsigned char skin)
	{
		if (35 <= age && age <= 60)
		{
			skin += 10;
		}
		else if (61 <= age && age <= 100)
		{
			skin += 20;
		}

		return skins[skin];
	}

	cell OnPlayerCancelTextDrawSelection(std::uint16_t playerid)
	{
		if (server::player_pool[playerid]->Flags().test(player::flags::authenticating) && !server::player_pool[playerid]->Flags().test(player::flags::customizing_player))
		{
			timers::timer_manager->Once(200, [=](timers::CTimer* timer) {
				SelectTextDraw(playerid, 0xD2B567FF);
			});

			return ~1;
		}
		else if (server::player_pool[playerid]->Flags().test(player::flags::customizing_player))
		{
			server::player_pool[playerid]->Flags().set(player::flags::customizing_player, false);
			textdraw_manager["player_customization"]->Hide(server::player_pool[playerid]);

			const auto screen_blacked = [=] {
				auto* player = server::player_pool[playerid];
				
				std::thread([](std::uint16_t playerid) {
					CPlayer* player = server::player_pool[playerid];

					{
						std::string password = *player->GetData<std::string>("auth:password");
						player->RemoveData("auth:password");
						player->Password() = Botan::argon2_generate_pwhash(password.c_str(), password.size(), Botan::system_rng(), 1, 1024, 1, 2);

						try
						{
							{
								auto stmt = server::database->PrepareLock(
									"INSERT INTO `PLAYERS` "
										"(NAME, PASSWORD, SEX, AGE, POS_X, POS_Y, POS_Z, ANGLE, VW, INTERIOR, SKIN, CURRENT_CONNECTION, MONEY) "
									"VALUES "
										"(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, strftime('%s', 'now'), ?); "
								);

								stmt->Bind<1>(player->Name());
								stmt->Bind<2>(player->Password());
								stmt->Bind<3>(player->Sex());
								stmt->Bind<4>(player->Age());
								stmt->Bind<5>(2110.2029f);
								stmt->Bind<6>(-1784.2820f);
								stmt->Bind<7>(13.3874f);
								stmt->Bind<8>(350.1182f);
								stmt->Bind<9>(0);
								stmt->Bind<10>(0);
								stmt->Bind<11>(player->Skin());
								stmt->Bind<12>(PLAYER_STARTING_MONEY);

								stmt->Step();

								player->AccountId() = stmt->LastInsertId();
							} // Database should be unlocked after the statement gets destroyed

							player->RegisterConnection();
							
						}
						catch (const std::runtime_error& e)
						{
							sampgdk::logprintf("[Auth] Failed to register player %i: %s", playerid, e.what());
							return;
						}
					}

					player->Flags().set(player::flags::registered, true);
					player->Flags().set(player::flags::authenticating, false);

					SetPlayerPos(playerid, 2109.1204, -1790.6901, 13.5547);
					SetPlayerFacingAngle(playerid, 350.1182);
					SetPlayerInterior(playerid, 0);
					SetPlayerCameraPos(playerid, 2096.242675, -1779.497558, 15.979070);
					SetPlayerCameraLookAt(playerid, 2103.439697, -1783.191162, 14.913400, CAMERA_CUT);
					RemovePlayerAttachedObject(playerid, INTRO_PROP_OBJECT_INDEX);
					SetPlayerSpecialAction(playerid, SPECIAL_ACTION_SMOKE_CIGGY);
					ApplyAnimation(playerid, "SMOKING", "null", 4.1, false, false, false, false, 0, false);
					ApplyAnimation(playerid, "SMOKING", "M_SMKLEAN_LOOP", 4.1, false, false, false, true, 0, false);

					const auto timer_due = [=](timers::CTimer*) {
						SetPlayerSpecialAction(playerid, SPECIAL_ACTION_NONE);
						ApplyAnimation(playerid, "PED", "WALK_CIVI", 4.1, true, true, true, true, 0, false);
						InterpolateCameraPos(playerid, 2100.242675, -1779.497558, 15.979070, 2109.331542, -1790.645874, 14.679038, 4000, CAMERA_CUT);
						InterpolateCameraLookAt(playerid, 2103.439697, -1783.191162, 14.913400, 2109.276855, -1785.655639, 14.370956, 4000, CAMERA_CUT);

						const auto give_controls_timer = [=](timers::CTimer*) {
							PlayerPlaySound(playerid, 5205, 0.0, 0.0, 0.0);
							ClearAnimations(playerid, false);
							server::player_pool[playerid]->ToggleWidescreen(false);
							server::player_pool[playerid]->ClearChat();
							SetCameraBehindPlayer(playerid);
							TogglePlayerControllable(playerid, true);
							SetPlayerVirtualWorld(playerid, 0);
							player->Needs()->StartUpdating();
							player->Needs()->ShowBars();

							server::player_pool[playerid]->Flags().set(player::flags::in_game, true);
						};
						timers::timer_manager->Once(4000, give_controls_timer);
					};
					timers::timer_manager->Once(7500, timer_due);
				}, playerid).detach();
			};
			server::player_pool[playerid]->FadeScreen()->Fade(255, screen_blacked);

			return ~1;
		}

		return 1;
	}

	static CPublicHook<OnPlayerCancelTextDrawSelection> _auth_opctds("OnPlayerCancelTextDrawSelection");

	cell OnGameModeInit()
	{
		// Set-up the textdraws

		auto* login_textdraws = textdraw_manager.LoadFile("auth.toml", "auth");
		auto* pcustom_textdraws = textdraw_manager.LoadFile("player_customization.toml", "player_customization");

		// Login password input
		login_textdraws->PlayerTextData()[2].callback = [](CPlayer* player) -> void {
			static const CPlayer::dialog_callback_t dialog_callback = [](CPlayer* player, bool response, std::uint8_t listitem, std::string inputtext) {
				if (!inputtext.empty())
				{
					if (inputtext.size() >= 32)
					{
						player->ShowDialog(DIALOG_STYLE_PASSWORD, "Introduce tu {D2B567}contraseña", "{FFFFFF}Introduce tu contraseña. Debe medir {D2B567}menos de 32 caracteres{FFFFFF}.", "Listo", "", dialog_callback);
						return;
					}

					player->SetData("auth:password", inputtext);
					if (player->GetData<bool>("auth:password_shown").value_or(false))
					{
						textdraw_manager["auth"]->GetPlayerTextDraws(player)[2]->SetText(inputtext);
					}
					else
					{
						textdraw_manager["auth"]->GetPlayerTextDraws(player)[2]->SetText(std::string(inputtext.size(), 'X'));
					}
				}
			};

			player->ShowDialog(DIALOG_STYLE_PASSWORD, "Introduce tu {D2B567}contraseña", "{FFFFFF}Introduce tu contraseña. Debe medir {D2B567}menos de 32 caracteres{FFFFFF}.", "Listo", "", dialog_callback);
		};

		// Login continue button
		login_textdraws->GetGlobalTextDraws()[19]->SetCallback([](CPlayer* player) -> void {
			player->CancelTextDrawSelection();

			if (!player->HasData("auth:password"))
			{
				SelectTextDraw(player->PlayerId(), 0xD2B567FF);
				player->ShowDialog(DIALOG_STYLE_MSGBOX, "{D2B567}Error", "{3E3D53}- {FFFFFF}Introduce una {D2B567}contraseña válida {FFFFFF}para continuar.", "Entendido", "");
				return;
			}

			if (player->Flags().test(player::flags::registered))
			{
				std::thread([](CPlayer* player) {
					std::string password = player->GetData<std::string>("auth:password").value_or("");
					bool result = Botan::argon2_check_pwhash(password.c_str(), password.length(), player->Password());
					if (!result)
					{
						SelectTextDraw(player->PlayerId(), 0xD2B567FF);
						player->ShowDialog(DIALOG_STYLE_MSGBOX, "{D2B567}Error", "{3E3D53}- {FFFFFF}La {D2B567}contraseña {FFFFFF}es incorrecta.", "Entendido", "");
						return;
					}

					player->Flags().set(player::flags::authenticating, false);
					player->RemoveData("auth:password");

					const auto screen_blacked = [player] {
						textdraw_manager["auth"]->Hide(player);

						float x, y, z, angle;
						x = player->Position().x;
						y = player->Position().y;
						z = player->Position().z;
						angle = player->Position().w;

						auto playerid = player->PlayerId();
						SetSpawnInfo(playerid, NO_TEAM, player->Skin(), x, y, z, angle, 0, 0, 0, 0, 0, 0);
						TogglePlayerSpectating(playerid, false);
						player->ToggleWidescreen(false);
						player->ClearChat();

						SetPlayerVirtualWorld(playerid, player->VirtualWorld());
						SetPlayerInterior(playerid, player->Interior());
						SetPlayerHealth(playerid, player->Health());
						SetPlayerArmour(playerid, player->Armor());
						GivePlayerMoney(playerid, player->GetMoney());
						SetCameraBehindPlayer(playerid);
						player->RegisterConnection();

						player->Notifications()->Show(fmt::format("Bienvenido a The Hood, {}. Tu última conexión fue el ~y~{}~w~.", player->Name(), player->LastConnection()), 5000);
						player->Needs()->StartUpdating();
						player->Needs()->ShowBars();

						player->Flags().set(player::flags::in_game, true);
					};
					player->FadeScreen()->Fade(255, screen_blacked);
				}, player).detach();
			}
			else
			{
				player->Flags().set(player::flags::customizing_player, true);

				const auto screen_blacked = [=] {
					player->FadeScreen()->Pause();
					textdraw_manager["auth"]->Hide(player);

					// highly unnecesary!
					player->Sex() = Random::get<bool>();
					player->Age() = Random::get(18, 100);
					unsigned char skin = Random::get(1, 5) + (5 * player->Sex());
					player->Skin() = AdjustSkinToRange(player->Age(), skin);
					selected_skins[player->PlayerId()] = skin;

					auto playerid = player->PlayerId();
					SetSpawnInfo(playerid, NO_TEAM, player->Skin(), 448.8462f, 508.5697f, 1001.4195f, 284.2451f, 0, 0, 0, 0, 0, 0);
					TogglePlayerSpectating(playerid, false);
					player->ToggleWidescreen(false);
					player->ClearChat();

					SetPlayerInterior(playerid, 12);
					SetPlayerVirtualWorld(playerid, 1 + playerid);
					SetPlayerCameraPos(playerid, 449.177429f, 510.692901f, 1001.518493f);
					SetPlayerCameraLookAt(playerid, 447.455413f, 506.018188f, 1001.092041f, CAMERA_CUT);
					ApplyAnimation(playerid, "CRIB", "null", 0.f, false, false, false, false, 0, false);
					ApplyAnimation(playerid, "CRIB", "PED_CONSOLE_LOOP", 4.1f, true, false, false, 0, false, false);
					SetPlayerSpecialAction(playerid, SPECIAL_ACTION_SMOKE_CIGGY);
					SetPlayerAttachedObject(playerid, INTRO_PROP_OBJECT_INDEX, 18875, 6, 0.15f, 0.15f, 0.0f, 0.0f, 0.0f, -110.59f, 1.0f, 1.0f, 1.0f, -1, -1);

					player->Flags().set(player::flags::customizing_player, true);
					textdraw_manager["player_customization"]->GetPlayerTextDraws(player)[0]->SetText(std::to_string(player->Age()));
					textdraw_manager["player_customization"]->Show(player);
					SelectTextDraw(playerid, 0xD2B567FF);

					player->FadeScreen()->Resume();
				};
				player->FadeScreen()->Fade(255, screen_blacked);
			}
		});

		// Player customization age input
		pcustom_textdraws->PlayerTextData()[0].callback = [](CPlayer* player) -> void {
			static const CPlayer::dialog_callback_t dialog_callback = [](CPlayer* player, bool response, std::uint8_t listitem, std::string inputtext) {
				if (inputtext.empty())
				{
					player->ShowDialog(DIALOG_STYLE_INPUT, "Introduce tu edad", "{FFFFFF}Introduce tu edad. Debe ser {D2B567}mayor a 18{FFFFFF} y {D2B567}menor a 100{FFFFFF}.", "Listo", "", dialog_callback);
					return;
				}

				int age = std::stoi(inputtext);
				if (age < 18 || age > 100)
				{
					player->ShowDialog(DIALOG_STYLE_INPUT, "Introduce tu edad", "{FFFFFF}Introduce tu edad. Debe ser {D2B567}mayor a 18{FFFFFF} y {D2B567}menor a 100{FFFFFF}.", "Listo", "", dialog_callback);
					return;
				}

				player->Age() = age;
				player->Skin() = AdjustSkinToRange(age, selected_skins[player->PlayerId()]);
				SetPlayerSkin(player->PlayerId(), player->Skin());
				ApplyAnimation(player->PlayerId(), "CRIB", "PED_CONSOLE_LOOP", 4.1, true, false, false, false, 0, false);

				textdraw_manager["player_customization"]->GetPlayerTextDraws(player)[0]->SetText(std::to_string(age));
			};
			player->ShowDialog(DIALOG_STYLE_INPUT, "Introduce tu edad", "{FFFFFF}Introduce tu edad. Debe ser {D2B567}mayor a 18{FFFFFF} y {D2B567}menor a 100{FFFFFF}.", "Listo", "", dialog_callback);
		};

		// Player customization male button
		pcustom_textdraws->GetGlobalTextDraws()[11]->SetCallback([](CPlayer* player) {
			if (player->Sex() == player::sex::male)
				return;

			player->Sex() = player::sex::male;
			selected_skins[player->PlayerId()] = selected_skins[player->PlayerId()] - 5;
			player->Skin() = AdjustSkinToRange(player->Age(), selected_skins[player->PlayerId()]);
			SetPlayerSkin(player->PlayerId(), player->Skin());
			ApplyAnimation(player->PlayerId(), "CRIB", "PED_CONSOLE_LOOP", 4.1, true, false, false, false, 0, false);
		});

		// Player customization female button
		pcustom_textdraws->GetGlobalTextDraws()[12]->SetCallback([](CPlayer* player) {
			if (player->Sex() == player::sex::female)
				return;

			player->Sex() = player::sex::female;
			selected_skins[player->PlayerId()] = selected_skins[player->PlayerId()] + 5;
			player->Skin() = AdjustSkinToRange(player->Age(), selected_skins[player->PlayerId()]);
			SetPlayerSkin(player->PlayerId(), player->Skin());
			ApplyAnimation(player->PlayerId(), "CRIB", "PED_CONSOLE_LOOP", 4.1, true, false, false, false, 0, false);
		});

		// Player customization left button
		pcustom_textdraws->GetGlobalTextDraws()[18]->SetCallback([](CPlayer* player) {
			unsigned char skin = selected_skins[player->PlayerId()] - (5 * player->Sex());

			if (skin > 1 && skin <= 5)
			{
				player->Skin() = AdjustSkinToRange(player->Age(), --selected_skins[player->PlayerId()]);
				SetPlayerSkin(player->PlayerId(), player->Skin());
				ApplyAnimation(player->PlayerId(), "CRIB", "PED_CONSOLE_LOOP", 4.1, true, false, false, false, 0, false);
			}
		});

		// Player customization right button
		pcustom_textdraws->GetGlobalTextDraws()[19]->SetCallback([](CPlayer* player) {
			unsigned char skin = selected_skins[player->PlayerId()] - (5 * player->Sex());
			
			if (skin >= 1 && skin < 5)
			{
				player->Skin() = AdjustSkinToRange(player->Age(), ++selected_skins[player->PlayerId()]);
				SetPlayerSkin(player->PlayerId(), player->Skin());
				ApplyAnimation(player->PlayerId(), "CRIB", "PED_CONSOLE_LOOP", 4.1, true, false, false, false, 0, false);
			}
		});

		auto& global = login_textdraws->GetGlobalTextDraws();
		global[16]->SetText("Código de referido");
		global[6]->SetText("Contraseña");

		return 1;
	}

	static CPublicHook<OnGameModeInit> _auth_ogmi("OnGameModeInit");

	cell OnPlayerConnect(std::uint16_t playerid)
	{
		TogglePlayerSpectating(playerid, true);
		server::player_pool[playerid]->ToggleWidescreen(true);
		server::player_pool[playerid]->ClearChat();
		server::player_pool[playerid]->Flags().set(player::flags::authenticating, true);

		const auto screen_blacked = [=]() {
			server::player_pool[playerid]->FadeScreen()->Pause();
			
			static const std::regex name_regex(".*");

			if (!std::regex_match(server::player_pool[playerid]->Name(), name_regex))
			{
				server::player_pool[playerid]->ShowDialog(DIALOG_STYLE_MSGBOX,
					"{DADADA}Nombre {ED2B2B}inválido",
					"{DADADA}Tu cuenta no puede ser registrada con un nombre inválido. Para entrar al servidor, tu nombre debe seguir el siguiente patrón:\n\n\t\"Nombre_Apellido\"",
					"Entendido", ""
				);

				timers::timer_manager->Once(150, [=](timers::CTimer* timer) {
					Kick(playerid);
				});

				return;
			}

			auto* textdraws = textdraw_manager.LoadFile("auth.toml", "auth");
			auto& global = textdraws->GetGlobalTextDraws();

			auto stmt = server::database->Prepare(
				"SELECT "
					"`PLAYERS`.*, `CONNECTION_LOGS`.`DATE` AS `LAST_CONNECTION` " 
				"FROM `PLAYERS`, `CONNECTION_LOGS` "
					"WHERE "
						"`PLAYERS`.`NAME` = ? "
						"AND `CONNECTION_LOGS`.`ACCOUNT_ID` = `PLAYERS`.`ID` "
				"ORDER BY `CONNECTION_LOGS`.`DATE` DESC "
				"LIMIT 1;"
			);

			stmt->Bind<1>(server::player_pool[playerid]->Name());

			stmt->Step();

			if (stmt->HasRow())
			{
				server::player_pool[playerid]->Flags().set(player::flags::registered, true);

				auto row = stmt->Row();
				server::player_pool[playerid]->AccountId() = *row->Get<int>("ID");
				server::player_pool[playerid]->Password() = *row->Get<std::string>("PASSWORD");
				server::player_pool[playerid]->Sex() = *row->Get<bool>("SEX");
				server::player_pool[playerid]->Age() = *row->Get<unsigned char>("AGE");
				server::player_pool[playerid]->SetMoney(*row->Get<int>("MONEY"), false, false);
				server::player_pool[playerid]->Health() = *row->Get<float>("HEALTH");
				server::player_pool[playerid]->Armor() = *row->Get<float>("ARMOUR");
				server::player_pool[playerid]->Position() = glm::vec4{
					*row->Get<float>("POS_X"),
					*row->Get<float>("POS_Y"),
					*row->Get<float>("POS_Z"),
					*row->Get<float>("ANGLE")
				};
				server::player_pool[playerid]->VirtualWorld() = *row->Get<int>("VW");
				server::player_pool[playerid]->Interior() = *row->Get<int>("INTERIOR");
				server::player_pool[playerid]->LastConnection() = *row->Get<std::string>("LAST_CONNECTION");
				server::player_pool[playerid]->Skin() = *row->Get<int>("SKIN");
				server::player_pool[playerid]->Needs()->SetHunger(*row->Get<float>("HUNGER"));
				server::player_pool[playerid]->Needs()->SetThirst(*row->Get<float>("THIRST"));
				server::player_pool[playerid]->Rank() = static_cast<player::rank>(*row->Get<unsigned char>("ADMIN"));
				server::player_pool[playerid]->PlayedTime() = *row->Get<int>("PLAYED_TIME");
				server::player_pool[playerid]->PhoneNumber() = *row->Get<int>("PHONE_NUMBER");

				// load weapon slots
				
				textdraws->GetPlayerTextDraws(server::player_pool[playerid])[1]->SetText(server::player_pool[playerid]->Name());
				textdraws->GetPlayerTextDraws(server::player_pool[playerid])[2]->SetText("Tu contraseña");
				textdraws->GetPlayerTextDraws(server::player_pool[playerid])[3]->SetText("Mostrar contraseña");

				global[7]->PushState();
				global[19]->PushState();

				global[7]->SetText("Cuenta registrada");

				for (size_t i = 0; i <= 13; ++i)
				{
					global[i]->Show();
				}

				global[19]->SetText("Iniciar sesión");

				for (size_t i = 18; i < global.size(); ++i)
				{
					global[i]->Show();
				}

				global[7]->PopState();
				global[19]->PopState();

				textdraws->GetPlayerTextDraws(server::player_pool[playerid])[0]->SetText(fmt::format("Último inicio de sesión: ~y~{}", server::player_pool[playerid]->LastConnection()));
				textdraws->GetPlayerTextDraws(server::player_pool[playerid])[1]->SetText(server::player_pool[playerid]->Name());

				for (size_t i = 0, count = textdraws->GetPlayerTextDraws(server::player_pool[playerid]).size() - 2; i <= count; ++i)
					textdraws->GetPlayerTextDraws(server::player_pool[playerid])[i]->Show();
			}
			else
			{
				SetPlayerCameraPos(playerid, 1585.296142, -2566.993652, 13.769470);
				SetPlayerCameraLookAt(playerid, 1580.729736, -2568.970458, 14.259890, CAMERA_CUT);

				textdraws->GetPlayerTextDraws(server::player_pool[playerid])[1]->SetText(server::player_pool[playerid]->Name());
				textdraws->GetPlayerTextDraws(server::player_pool[playerid])[2]->SetText("Tu contraseña");
				textdraws->GetPlayerTextDraws(server::player_pool[playerid])[3]->SetText("Mostrar contraseña");

				textdraws->Show(server::player_pool[playerid], 0, -1, 1, -1);
			}

			SelectTextDraw(playerid, 0xD2B567FF);
			server::player_pool[playerid]->FadeScreen()->Resume();
		};

		server::player_pool[playerid]->FadeScreen()->Fade(255, screen_blacked);

		return 1;
	}

	static CPublicHook<OnPlayerConnect> _auth_opc("OnPlayerConnect");
}