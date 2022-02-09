#include "../../main.hpp"

static public_hook _s_pz_ogmi("OnGameModeInit", +[]()
{
	sampgdk::logprintf("[stores:pizza] Registering...");

	enter_exits->Create(19902, "{ED2B2B}Ugi's Pizza\n{DADADA}Presiona {ED2B2B}H {DADADA}para entrar", "{DADADA}Presiona {ED2B2B}H {DADADA}para salir", { 2105.0681, -1806.4565, 13.5547, 91.9755 }, 0, 0, { 372.4150, -133.3214, 1001.4922, 355.1316 }, 0, 5, nullptr);

	// Actors
	streamer::CreateDynamicActor(155, 373.7393, -117.2236, 1002.4995, 175.4680, true, 100.0f, 0, 5);

	// Map Icons
	streamer::CreateDynamicMapIcon(2105.0681, -1806.4565, 13.5547, 29, -1, 0, 0);

	const auto cb = [](shops::CShop* shop, CPlayer* player, shops::stShopItem* item)
	{
		if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - player->Needs()->LastEatTick()) > std::chrono::minutes{ 5 })
		{
			player->Needs()->EatCount() = 0u;
		}

		player->Needs()->EatCount()++;
		player->Needs()->LastEatTick() = std::chrono::steady_clock::now();

		if (player->Needs()->EatCount() >= 5u)
		{
			player->Needs()->Puke();
			return false;
		}

		switch (utils::hash(item->name))
		{
			case "Porción de pizza pepperoni"_hash:
			{
				player->Chat()->Send(0xDADADAFF, "Compraste una {ED2B2B}porción de pizza pepperoni{DADADA}.");
				player->Needs()->GiveHunger(-10.0);
				player->Needs()->GiveThirst(1.0);
				
				break;
			}
		}

		return true;
	};

	auto* shop = shop_manager->Create("Ugi's Pizza", { 373.7325, -119.4309, 1001.4922 }, 0, 5, { { 372.986755, -118.988250, 1002.399780 }, { 375.441986, -115.871269, 999.357360 } });
	shop->SetCallback(cb);
	shop->SetObjectPositions({ 373.21, -118.10, 1001.58 }, { 373.97, -118.07, 1001.58 }, { 375.06, -118.06, 1001.58 });
	shop->AddItem("Porción de pizza pepperoni", 2218, 25, { -25.29, 23.39, 74.69 });
	shop->AddItem("Pizza con papas fritas", 2220, 35, { -25.29, 23.39, 74.69 });
	shop->AddItem("Ensalada con pollo", 2355, 40, { -25.29, 23.39, 74.69 });
	shop->AddItem("Porción de pizza con ensalada", 2219, 50, { -25.29, 23.39, 74.69 });
	shop->AddItem("Pizza grande", 19580, 100, { -25.29, 23.39, 74.69 });

	return 1;
});
