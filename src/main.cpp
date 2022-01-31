#include "main.hpp"

extern void* pAMXFunctions;
void** server::plugin_data;
std::unique_ptr<sqlite::Database> server::database;
static std::chrono::steady_clock::time_point load_timestamp;

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	load_timestamp = std::chrono::steady_clock::now();

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
	sampgdk::logprintf("\n\n\n");
	
	sampgdk::logprintf("   /////////////////////////////////////////");
	sampgdk::logprintf("   ///                                   ///");
	sampgdk::logprintf(fmt::format(FMT_COMPILE("   ///{: ^35}///"), "The Hood").c_str());
	sampgdk::logprintf("   ///                                   ///");
	sampgdk::logprintf("   /////////////////////////////////////////");
	sampgdk::logprintf("   ~ Applying patches...");

	{
		unsigned char* const wrong_pid_branch = reinterpret_cast<unsigned char*>(_WIN32 ? 0x004591FC : 0x080752FC);
		constexpr auto size_wrong_pid_branch = (_WIN32 ? 82 : 114);
		utils::unlocked_scope lk(wrong_pid_branch, size_wrong_pid_branch);
		std::memset(wrong_pid_branch, 0x90, size_wrong_pid_branch);
	}

	try
	{
		server::database = std::make_unique<sqlite::Database>("scriptfiles/the_hood.db");
		sampgdk::logprintf("   ~ Database file opened.");

		sampgdk::logprintf("   ~ Enabling database optimizations...");
		server::database->Exec(
			"PRAGMA TEMP_STORE = FILE; "
			"PRAGMA JOURNAL_MODE = TRUNCATE; "
			"PRAGMA SYNCHRONOUS = NORMAL; "
			"PRAGMA LOCKING_MODE = NORMAL;"
		);

		sampgdk::logprintf("   ~ Setting up database...");
		std::ifstream struct_file{ "./scriptfiles/struct.sql" };
		if (!struct_file.good())
		{
			sampgdk::logprintf("   [!] Failed to initialize database:");
			sampgdk::logprintf("   [!]   Couldn't find database structure file.");
			std::exit(1);
		}

		std::stringstream queries;
		queries << struct_file.rdbuf();
		struct_file.close();

		server::database->Exec(queries.str());
	}
	catch (const std::exception& e)
	{
		sampgdk::logprintf("   [!] Failed to open or initialize database:");
		sampgdk::logprintf("   [!]   %s", e.what());
		std::exit(1);
	}

	sampgdk::logprintf("   ~ Database setup done.");

	net::RakServer = std::make_unique<net::CRakServer>(server::plugin_data);
	server::console = std::make_unique<CConsole>();

	SetNameTagDrawDistance(20.f);
	SendRconCommand("hostname 	  .•°   The Hood (RPG en Español)   °•.");
	SendRconCommand("language Español");
	SendRconCommand("gamemodetext Roleplay / RPG");

	// no one cares about this !
	server::console->ModifyVariableFlags("weather", CON_VARFLAG_READONLY);
	server::console->ModifyVariableFlags("worldtime", CON_VARFLAG_READONLY);
	server::console->ModifyVariableFlags("version", CON_VARFLAG_READONLY);
	server::console->ModifyVariableFlags("mapname", CON_VARFLAG_READONLY);
	
	// provide actually useful information
	server::console->SetStringVariable("weburl", "rakmong.com");
	server::console->AddStringVariable("versión de sa-mp", CON_VARFLAG_RULE, "0.3.7", nullptr);
	server::console->AddStringVariable("discord", CON_VARFLAG_RULE, "rakmong.com/servers/the-hood", nullptr);
	server::console->AddStringVariable("última actualización", CON_VARFLAG_RULE, __DATE__, nullptr);

	UsePlayerPedAnims();
	DisableInteriorEnterExits();
	EnableStuntBonusForAll(false);
	ManualVehicleEngineAndLights();
	ShowPlayerMarkers(PLAYER_MARKERS_MODE_GLOBAL);
	SetNameTagDrawDistance(25.f);

	sampgdk::logprintf("\n\n\n");

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - load_timestamp);
	sampgdk::logprintf("Took %i milliseconds to load the gamemode.", ms.count());

	return true;
}
