/*


.......................................................................
.   o   \ o /  _ o        __|    \ /     |__         o _  \ o /   o   .
.  /|\    |     /\   __\o   \o    |    o/     o/__   /\     |    /|\  .
.  / \   / \   | \  /) |    ( \  /o\  / )    |   (\  / |   / \   / \  .
.       .......................................................       .
. \ o / .                                                     . \ o / .
.   |   .                                                     .   |   .
.  / \  .     **********THE BEST RAKSAMP EVER************     .  / \  .
.       .                                                     .       .
.  _ o  .     		   __    _____ _____ _____                .  _ o  .
.   /\  .     		  |  |  |     |  _  |     |               .   /\  .
.  | \  .     		  |  |__| | | |     |  |  |               .  | \  .
.       .     		  |_____|_|_|_|__|__|_____|               .       .
.       .      _____ __ __ _____ _____ _____ _____ _____      .       .
.  __\o .     |   __|  |  |_   _| __  |   __|     |   __|     .  __\o .
. /) |  .     |   __|-   -| | | |    -|   __| | | |   __|     . /) |  .
.       .     |_____|__|__| |_| |__|__|_____|_|_|_|_____|     .       .
. __|   .                                                     . __|   .
.   \o  .     >>>>>>>>>>>>>>>-- CREDITS --<<<<<<<<<<<<<<<     .    \o .
.   ( \ .                                                     .   ( \ .
.       .      .P3TI.     .TYT.      .FYP.        .OPCODE.    .       .
.  \ /  .               .HANDZ.       .BRAINZ.                .  \ /  .
.   |   .                                                     .   |   .
.  /o\  .     .-.            .-.                              .  /o\  .
.       .   .-| |-.        .-| |-.          _---~~(~~-_.      .       .
.   |__ .   | | | |  THX   | | | |        _{  KK    )   )     .   |__ .
. o/    . .-| | | |        | | | |-.    ,   ) -~~- ( ,-' )_   . o/    .
./ )    . | | | | |        | | | | |   (  `-,_..`., )-- '_,)  ./ )    .
.       . | | | | |-.    .-| | |*| |  (_-  _  ~_-~~~~`,  ,' ) .       .
.       . | '     | |    | |     ` |    `~ -^(    __;-,((())) .       .
. o/__  . |       | |    | |       |          ~~~~ {_ -_(())  . o/__  .
.  | (\ . |         |    |         |     THX        `\  }     . |  (\ .
.       . \         /    \         /                 `\  }    .       .
.  o _  .  |       |     |       |                            .  o _  .
.  /\   .  |       |     |       |                            .  /\   .
.  / |  .      +----------------------------------+           .  / |  .
.       .      |     MANY THANKS TO OUR HANDS     |           .       .
. \ o / .      | AND BRAINS TO MAKE THIS POSSIBLE |           . \ o / .
.   |   .      +----------------------------------+           .   |   .
.  / \  .                                                     .  / \  .
.       .......................................................       .
.   o   \ o /  _ o        __|    \ /     |__         o _  \ o /   o   .
.  /|\    |     /\   __\o   \o    |    o/     o/__   /\     |    /|\  .
.  / \   / \   | \  /) |    ( \  /o\  / )    |   (\  / |   / \   / \  .
.......................................................................

*/

#pragma once

//
// STL
#include "pch.h"

//
// Libraries
#include <uv.h>
#if defined __GNUC__ || defined __clang__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wattributes"
	#include <samp-gdk/sampgdk.h>
	#pragma GCC diagnostic pop
#else
	#include <samp-gdk/sampgdk.h>
#endif
#include <sqlite3.h>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <urmem/urmem.hpp>
#if defined __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunsequenced"
	#include <raknet/RakNet.h>
	#include <raknet/BitStream.h>
	#pragma clang diagnostic pop
#else
	#include <raknet/RakNet.h>
	#include <raknet/BitStream.h>
#endif
#include <robin_hood.h>
#include <toml++/toml.h>
#include <effolkronium/random.hpp>
#include <botan/hash.h>
#include <botan/hex.h>
#include <botan/argon2fmt.h>
#include <botan/system_rng.h>
#include <ringbuffer/ringbuffer.hpp>

#undef MAX_PLAYERS
#define MAX_PLAYERS 150
using Random = effolkronium::random_static;
#ifdef _WIN32
	#ifndef FASTCALL
		#define FASTCALL __fastcall
	#endif
#else
	using SOCKET = uintptr_t;

	#ifndef FASTCALL
		#define FASTCALL __attribute__((fastcall))
	#endif
#endif


//
// Source

// Native replacements
#include "Natives.hpp"

#include "Utils.hpp"

class CPlayer;
class CVehicle;

#include "hooks/RakUtil.hpp"
#include "hooks/CRakServer.hpp"
#include "hooks/CConsole.hpp"
#include "hooks/Publics.hpp"
#include "hooks/Server.hpp"

#include "server/natives/streamer/Natives.hpp"
#include "server/natives/colandreas/Natives.hpp"
#include "server/Database.hpp"
#include "server/commands/ArgumentStore.hpp"
#include "server/commands/Commands.hpp"
#include "server/timers/Timer.hpp"
#include "server/textdraws/TextDrawManager.hpp"
#include "server/textdraws/TextDraw.hpp"
#include "server/EnterExitManager.hpp"
#include "server/shops/ShopManager.hpp"
#include "server/vehicles/CVehicle.hpp"
#include "server/vehicles/CPlayerVehicleManager.hpp"

#include "player/CFadeScreen.hpp"
#include "player/CChat.hpp"
#include "player/Notifications.hpp"
#include "player/Needs.hpp"
#include "player/KeyGame.hpp"
#include "player/auth/Auth.hpp"
#include "player/CSpeedometer.hpp"
#include "player/CPlayer.hpp"
#include "player/CPlayerPool.hpp"
#include "player/jobs/Jobs.hpp"

namespace server
{
	extern std::unique_ptr<sqlite::Database> database;
	extern void** plugin_data;
}
