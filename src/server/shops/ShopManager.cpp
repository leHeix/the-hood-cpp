#include "../../main.hpp"

std::unique_ptr<shops::CShopManager> shop_manager = std::make_unique<shops::CShopManager>();

inline shops::CShopManager::player_shop_data& shops::CShopManager::PlayerData(CPlayer* player) 
{ 
	return _player_data[player->PlayerId()]; 
}

shops::CShop* shops::CShopManager::Create(const std::string& name, glm::vec3 position, int world, int interior, std::pair<glm::vec3, glm::vec3> camera)
{
	std::unique_ptr<CShop> shop = std::make_unique<CShop>();
	shop->name = name;
	shop->world = world;
	shop->interior = interior;
	shop->cam_pos = camera.first;
	shop->cam_look_at = camera.second;
	shop->position = position;
	shop->label = streamer::CreateDynamic3DTextLabel(fmt::format("{{ED2B2B}}{}\n{{DADADA}}Presiona {{ED2B2B}}Y {{DADADA}}para ver el inventario", name), 0xED2B2BFF, position.x, position.y, position.z, 10.f, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 1, world, interior);;
	shop->area = streamer::CreateDynamicCircle(position.x, position.y, 1.f, world, interior);
	
	cell info[2] = { 'SHOP', static_cast<cell>(_shops.size()) };
	streamer::data::SetArrayData(streamer::STREAMER_TYPE_AREA, shop->area, streamer::E_STREAMER_EXTRA_ID, info);

	_shops.push_back(std::move(shop));
	return _shops.back().get();
}

shops::stShopItem* shops::CShop::AddItem(const std::string& name, int model, int price, const glm::vec3& rotation)
{
	auto shopitem = std::make_unique<stShopItem>();
	shopitem->name = name;
	shopitem->model = model;
	shopitem->price = price;
	shopitem->rotation = rotation;

	_shop_items.push_back(std::move(shopitem));
	return _shop_items.back().get();
}

cell shops::OnPlayerObjectMoved(std::uint16_t playerid, std::uint16_t objectid)
{
	if (shop_manager->PlayerData(playerid).object == objectid)
	{
		auto* player = server::player_pool[playerid];
		auto* shop = player->CurrentShop();
		auto& shop_pdata = shop_manager->PlayerData(playerid);
		auto& item_data = *shop_pdata.selected_item;
		player->Flags().set(player::flags::can_use_shop_buttons, true);
		PlayerPlaySound(playerid, 1145, 0.f, 0.f, 0.f);

		glm::vec3 cur_pos;
		GetPlayerObjectPos(playerid, objectid, &cur_pos.x, &cur_pos.y, &cur_pos.z);
		if (cur_pos == player->CurrentShop()->object_pos.end)
		{
			DestroyPlayerObject(playerid, objectid);
			shop_pdata.object = CreatePlayerObject(playerid, item_data->model, shop->object_pos.start.x, shop->object_pos.start.y, shop->object_pos.start.z, item_data->rotation.x, item_data->rotation.y, item_data->rotation.z, 0.f);
			MovePlayerObject(playerid, shop_pdata.object, shop->object_pos.idle.x, shop->object_pos.idle.y, shop->object_pos.idle.z, 1.2f, -1000.f, -1000.f, -1000.f);
		
			textdraw_manager["shop"]->GetPlayerTextDraws(player)[1]->SetText(fmt::format("${}", item_data->price));
			textdraw_manager["shop"]->GetPlayerTextDraws(player)[2]->SetText(item_data->name);
		}

		return ~1;
	}

	return 1;
}

static CPublicHook<shops::OnPlayerObjectMoved> _s_opom("OnPlayerObjectMoved");

cell shops::OnPlayerKeyStateChange(std::uint16_t playerid, std::uint32_t newkeys, std::uint32_t oldkeys)
{
	if ((newkeys & KEY_YES) != 0)
	{
		auto areas = streamer::GetPlayerDynamicAreas(playerid);
		if (!areas.empty())
		{
			auto* player = server::player_pool[playerid];

			for (auto&& area : areas)
			{
				cell info[2]{};
				streamer::data::GetArrayData(streamer::STREAMER_TYPE_AREA, area, streamer::E_STREAMER_EXTRA_ID, info);
				if (info[0] == 'SHOP')
				{
					player->Flags().set(player::flags::can_use_shop_buttons, true);
					player->Flags().set(player::flags::using_shop, true);

					auto& shop = shop_manager->_shops[info[1]];
					player->CurrentShop() = shop.get();

					auto* textdraws = textdraw_manager["shop"];
					auto& shop_item = shop->_shop_items.front();

					textdraws->GetPlayerTextDraws(player)[0]->SetText(shop->name);
					textdraws->GetPlayerTextDraws(player)[1]->SetText(fmt::format("${}", shop_item->price));
					textdraws->GetPlayerTextDraws(player)[2]->SetText(shop_item->name);
					textdraws->Show(player);

					glm::vec3 cam_pos, cam_vec;
					GetPlayerCameraPos(playerid, &cam_pos.x, &cam_pos.y, &cam_pos.z);
					GetPlayerCameraFrontVector(playerid, &cam_vec.x, &cam_vec.y, &cam_vec.z);
					InterpolateCameraPos(playerid, cam_pos.x, cam_pos.y, cam_pos.z, shop->cam_pos.x, shop->cam_pos.y, shop->cam_pos.z, 1000, CAMERA_CUT);
					InterpolateCameraLookAt(playerid, cam_vec.x, cam_vec.y, cam_vec.z, shop->cam_look_at.x, shop->cam_look_at.y, shop->cam_look_at.z, 1000, CAMERA_CUT);
				
					SelectTextDraw(playerid, 0xD2B567FF);

					PlayerPlaySound(playerid, 1145, 0.0, 0.0, 0.0);

					shop_manager->_player_data[playerid].selected_item = shop->_shop_items.begin();
					shop_manager->_player_data[playerid].object = CreatePlayerObject(playerid, shop_item->model, shop->object_pos.start.x, shop->object_pos.start.y, shop->object_pos.start.z, shop_item->rotation.x, shop_item->rotation.y, shop_item->rotation.z, 0.f);
					MovePlayerObject(playerid, shop_manager->_player_data[playerid].object, shop->object_pos.idle.x, shop->object_pos.idle.y, shop->object_pos.idle.z, 1.2, -1000.0, -1000.0, -1000.0);
				}
			}
		}
	}

	return 1;
}

static CPublicHook<shops::OnPlayerKeyStateChange> _s_opksc("OnPlayerKeyStateChange");

static cell RegisterShopCallbacks()
{
	auto* textdraws = textdraw_manager.LoadFile("shop.toml", "shop");
	// Left button
	textdraws->GetGlobalTextDraws()[6]->SetCallback([](CPlayer* player) {
		if (!player->Flags().test(player::flags::can_use_shop_buttons))
			return;

		if (shop_manager->PlayerData(player).selected_item == player->CurrentShop()->Items().begin())
			return;

		auto* shop = player->CurrentShop();
		player->Flags().set(player::flags::can_use_shop_buttons, false);
		shop_manager->PlayerData(player).selected_item--;
		MovePlayerObject(*player, shop_manager->PlayerData(player).object, shop->ObjectPos().end.x, shop->ObjectPos().end.y, shop->ObjectPos().end.z, 1.2f, -1000.f, -1000.f, -1000.f);
	});
	// Right button
	textdraws->GetGlobalTextDraws()[7]->SetCallback([](CPlayer* player) {
		if (!player->Flags().test(player::flags::can_use_shop_buttons))
			return;

		if (shop_manager->PlayerData(player).selected_item + 1 == player->CurrentShop()->Items().end())
			return;

		auto* shop = player->CurrentShop();
		player->Flags().set(player::flags::can_use_shop_buttons, false);
		shop_manager->PlayerData(player).selected_item++;
		MovePlayerObject(*player, shop_manager->PlayerData(player).object, shop->ObjectPos().end.x, shop->ObjectPos().end.y, shop->ObjectPos().end.z, 1.2f, -1000.f, -1000.f, -1000.f);
	});
	// Buy button
	textdraws->GetGlobalTextDraws()[8]->SetCallback([](CPlayer* player) {
		if (!player->Flags().test(player::flags::can_use_shop_buttons))
			return;

		if (player->CurrentShop()->TriggerCallback(player, shop_manager->PlayerData(player).selected_item->get()))
		{
			player->GiveMoney(-((*shop_manager->PlayerData(player).selected_item)->price));
			PlayerPlaySound(*player, 1054, 0.0, 0.0, 0.0);
		}
		else
		{
			PlayerPlaySound(*player, 1055, 0.0, 0.0, 0.0);
		}
	});

	return 1;
}

static CPublicHook<RegisterShopCallbacks> _s_ogmi("OnGameModeInit");

static cell CancelPlayerShopAction(std::uint16_t playerid)
{
	auto* player = server::player_pool[playerid];
	if (player->Flags().test(player::flags::using_shop))
	{
		player->StopShopping();
		return ~1;
	}

	return 1;
}

static CPublicHook<CancelPlayerShopAction> _s_opctds("OnPlayerCancelTextDrawSelection");