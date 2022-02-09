#include "../../main.hpp"

static public_hook _s_conv_ogmi("OnGameModeInit", +[]()
{
	sampgdk::logprintf("[stores:convenience] Registering...");

	enter_exits->Create(19902, "{ED2B2B}7-Eleven\n{DADADA}Presiona {ED2B2B}H {DADADA}para entrar", "{DADADA}Presiona {ED2B2B}H {DADADA}para salir", { 2001.8507, -1761.6123, 13.5391, 359.4877 }, 0, 0, { 6.0728, -31.3407, 1003.5494, 6.2127 }, 0, 10, nullptr);
	enter_exits->Create(19902, "{ED2B2B}7-Eleven\n{DADADA}Presiona {ED2B2B}H {DADADA}para entrar", "{DADADA}Presiona {ED2B2B}H {DADADA}para salir", { 1833.0375, -1842.4987, 13.5781, 82.9984 }, 0, 0, { 6.0728, -31.3407, 1003.5494, 6.2127 }, 1, 10, nullptr);

	// Actors
	streamer::CreateDynamicActor(229, 2.0491, -30.7007, 1004.5494, 358.3559, true, 100.0f, 0, 10);
	streamer::CreateDynamicActor(229, 2.0491, -30.7007, 1004.5494, 358.3559, true, 100.0f, 1, 10);

	// Map Icons
    streamer::CreateDynamicMapIcon(2001.8507, -1761.6123, 13.5391, 17, -1, 0, 0);
    streamer::CreateDynamicMapIcon(1833.0375, -1842.4987, 13.5781, 17, -1, 0, 0);

	const auto cb = [](shops::CShop* shop, CPlayer* player, shops::stShopItem* item)
	{
		player->Chat()->Send(-1, "jijo");
		return false;
	};

	auto* shop = shop_manager->Create("7-Eleven", { 2.1105, -29.0141, 1003.5494 }, 0, 10, { { 1.124887, -28.677103, 1004.111938 }, { 4.151215, -32.320850, 1002.510559 } });
	shop->SetCallback(cb);
	shop->SetObjectPositions({ 1.02, -29.88, 1003.47 }, { 2.0, -29.88, 1003.47 }, { 1.02, -29.88, 1003.47 });
	shop->AddItem("Teléfono celular", 19513, 500, { 0.0, 0.0, 0.0 });
	shop->AddItem("GPS", 18875, 250, { 0.0, 0.0, 0.0 });
	shop->AddItem("Walkie-Talkie", 330, 1500, { 90.10, 3.59, -90.09 });
	shop->AddItem("Vaso de café", 19835, 10, { 0.0, 0.0, 0.0 });

	return 1;
});
