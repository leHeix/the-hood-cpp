#include "main.hpp"

extern void* pAMXFunctions;
void** server::plugin_data;
std::unique_ptr<sqlite::Database> server::database;

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	std::srand(std::time(0));

	server::plugin_data = ppData;
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];

	return sampgdk::Load(ppData);
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	if (uv_loop_close(uv_default_loop()) == UV_EBUSY)
	{
		uv_walk(uv_default_loop(), [](uv_handle_t* h, void* arg) {
			uv_close(h, [](uv_handle_t* h) {
				if (!h || !h->loop || uv_is_closing(h))
					return;

				sampgdk::logprintf("[evloop] Closed handle %x", reinterpret_cast<int>(h));
			});
		}, nullptr);
		uv_run(uv_default_loop(), UV_RUN_DEFAULT);
		[[maybe_unused]] int err = uv_loop_close(uv_default_loop());
		assert(err == 0);
	}

	sampgdk::Unload();
}

PLUGIN_EXPORT unsigned PLUGIN_CALL Supports()
{
	return sampgdk::Supports() | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	// sampgdk::ProcessTick();
	uv_run(uv_default_loop(), UV_RUN_NOWAIT);
}

// -

PLUGIN_EXPORT bool PLUGIN_CALL OnGameModeInit()
{
	sampgdk::logprintf("   /////////////////////////////////////////");
	sampgdk::logprintf("   ///                                   ///");
	sampgdk::logprintf("   ///               The Hood            ///");
	sampgdk::logprintf("   ///                                   ///");
	sampgdk::logprintf("   /////////////////////////////////////////");
	
	try
	{
		server::database = std::make_unique<sqlite::Database>("scriptfiles/the_hood.db");
		sampgdk::logprintf("~ Database file opened.");

		sampgdk::logprintf("~ Enabling database optimizations...");
		server::database->Exec(
			"PRAGMA TEMP_STORE = FILE; "
			"PRAGMA JOURNAL_MODE = TRUNCATE; "
			"PRAGMA SYNCHRONOUS = NORMAL; "
			"PRAGMA LOCKING_MODE = NORMAL;"
		);

		sampgdk::logprintf("~ Setting up database...");
		std::ifstream struct_file{ "./scriptfiles/struct.sql" };
		if (!struct_file.good())
		{
			sampgdk::logprintf("[!] Failed to initialize database:");
			sampgdk::logprintf("[!]   Couldn't find database structure file.");
			std::exit(1);
		}

		std::stringstream queries;
		queries << struct_file.rdbuf();
		struct_file.close();

		server::database->Exec(queries.str());
	}
	catch (const std::exception& e)
	{
		sampgdk::logprintf("[!] Failed to open or initialize database:");
		sampgdk::logprintf("[!]   %s", e.what());
		std::exit(1);
	}

	sampgdk::logprintf("~ Database setup done.");

	net::RakServer = std::make_unique<net::CRakServer>(server::plugin_data);
	server::console = std::make_unique<CConsole>();

	SetNameTagDrawDistance(20.f);
	SendRconCommand("hostname 	  .��   The Hood (RPG en Espa�ol)   ��.");
	SendRconCommand("language Espa�ol");
	SendRconCommand("gamemodetext Roleplay / RPG");

	// no one cares about this !
	server::console->ModifyVariableFlags("weather", CON_VARFLAG_READONLY);
	server::console->ModifyVariableFlags("worldtime", CON_VARFLAG_READONLY);
	server::console->ModifyVariableFlags("version", CON_VARFLAG_READONLY);
	server::console->ModifyVariableFlags("mapname", CON_VARFLAG_READONLY);
	
	// provide actually useful information
	server::console->SetStringVariable("weburl", "rakmong.com");
	server::console->AddStringVariable("versi�n de sa-mp", CON_VARFLAG_RULE, "0.3.7", nullptr);
	server::console->AddStringVariable("discord", CON_VARFLAG_RULE, "rakmong.com/servers/the-hood", nullptr);
	server::console->AddStringVariable("�ltima actualizaci�n", CON_VARFLAG_RULE, __DATE__, nullptr);

	UsePlayerPedAnims();
	DisableInteriorEnterExits();
	EnableStuntBonusForAll(false);
	ManualVehicleEngineAndLights();

	return true;
}