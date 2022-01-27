#pragma once

namespace auth 
{
	constexpr auto INTRO_PROP_OBJECT_INDEX = 0;
	constexpr auto PLAYER_STARTING_MONEY = 250;

	constexpr unsigned short skins[] = {
		(unsigned short)-1,

		/* Edad: 18 - 34 */
		19, 4, 7, 20, 21,			// Hombres
		12, 65, 76, 139,	 207,		// Mujeres

		/* Edad: 35 - 60 */
		6, 14, 15, 17, 24,          // Hombres
		11, 13, 63, 69, 148,        // Mujeres

		/* Edad: 61 - 100 */
		220, 221, 222, 262, 296,    // Hombres
		218, 10, 39, 129, 131       // Mujeres
	};

	extern std::array<unsigned char, MAX_PLAYERS> selected_skins;

	cell OnGameModeInit();
	cell OnPlayerConnect(std::uint16_t playerid);
}