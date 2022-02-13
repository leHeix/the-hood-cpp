#pragma once

class CVehicle;

namespace vehicles
{
	struct stModelData
	{
		int max_speed;
		float max_fuel;
		int price;
	};

	struct stModel
	{
		std::uint16_t id;
		std::string_view name;
	};

	constexpr inline const std::array<const std::string_view, 212> names{ {
		"Landstalker",
		"Bravura",
		"Buffalo",
		"Linerunner",
		"Pereniel",
		"Sentinel",
		"Dumper",
		"Firetruck",
		"Trashmaster",
		"Stretch",
		"Manana",
		"Infernus",
		"Voodoo",
		"Pony",
		"Mule",
		"Cheetah",
		"Ambulance",
		"Leviathan",
		"Moonbeam",
		"Esperanto",
		"Taxi",
		"Washington",
		"Bobcat",
		"Mr Whoopee",
		"BF Injection",
		"Hunter",
		"Premier",
		"Enforcer",
		"Securicar",
		"Banshee",
		"Predator",
		"Bus",
		"Rhino",
		"Barracks",
		"Hotknife",
		"Trailer",
		"Previon",
		"Coach",
		"Cabbie",
		"Stallion",
		"Rumpo",
		"RC Bandit",
		"Romero",
		"Packer",
		"Monster",
		"Admiral",
		"Squalo",
		"Seasparrow",
		"Pizzaboy",
		"Tram",
		"Trailer",
		"Turismo",
		"Speeder",
		"Reefer",
		"Tropic",
		"Flatbed",
		"Yankee",
		"Caddy",
		"Solair",
		"Berkley's RC Van",
		"Skimmer",
		"PCJ-600",
		"Faggio",
		"Freeway",
		"RC Baron",
		"RC Raider",
		"Glendale",
		"Oceanic",
		"Sanchez",
		"Sparrow",
		"Patriot",
		"Quad",
		"Coastguard",
		"Dinghy",
		"Hermes",
		"Sabre",
		"Rustler",
		"ZR-350",
		"Walton",
		"Regina",
		"Comet",
		"BMX",
		"Burrito",
		"Camper",
		"Marquis",
		"Baggage",
		"Dozer",
		"Maverick",
		"News Chopper",
		"Rancher",
		"FBI Rancher",
		"Virgo",
		"Greenwood",
		"Jetmax",
		"Hotring",
		"Sandking",
		"Blista",
		"Police Maverick",
		"Boxville",
		"Benson",
		"Mesa",
		"RC Goblin",
		"Hotring-Racer",
		"Hotring-Racer",
		"Bloodring-Banger",
		"Rancher",
		"Super-GT",
		"Elegant",
		"Journey",
		"Bike",
		"Mountain Bike",
		"Beagle",
		"Cropdust",
		"Stunt",
		"Tanker",
		"RoadTrain",
		"Nebula",
		"Majestic",
		"Buccaneer",
		"Shamal",
		"Hydra",
		"FCR-900",
		"NRG-500",
		"HPV1000",
		"Cement Truck",
		"Tow Truck",
		"Fortune",
		"Cadrona",
		"FBI Truck",
		"Willard",
		"Forklift",
		"Tractor",
		"Combine",
		"Feltzer",
		"Remington",
		"Slamvan",
		"Blade",
		"Freight",
		"Streak",
		"Vortex",
		"Vincent",
		"Bullet",
		"Clover",
		"Sadler",
		"Firetruck",
		"Hustler",
		"Intruder",
		"Primo",
		"Cargobob",
		"Tampa",
		"Sunrise",
		"Merit",
		"Utility",
		"Nevada",
		"Yosemite",
		"Windsor",
		"Monster Truck A",
		"Monster Truck B",
		"Uranus",
		"Jester",
		"Sultan",
		"Stratum",
		"Elegy",
		"Raindance",
		"RC Tiger",
		"Flash",
		"Tahoma",
		"Savanna",
		"Bandito",
		"Freight",
		"Trailer",
		"Kart",
		"Mower",
		"Duneride",
		"Sweeper",
		"Broadway",
		"Tornado",
		"AT-400",
		"DFT-30",
		"Huntley",
		"Stafford",
		"BF-400",
		"Newsvan",
		"Tug",
		"Trailer",
		"Emperor",
		"Wayfarer",
		"Euros",
		"Hotdog",
		"Club",
		"Trailer",
		"Trailer",
		"Andromada",
		"Dodo",
		"RC Cam",
		"Launch",
		"Police Car",
		"Police Car",
		"Police Car",
		"Police Ranger",
		"Picador",
		"S.W.A.T. Van",
		"Alpha",
		"Phoenix",
		"Glendale",
		"Sadler",
		"Luggage Trailer",
		"Luggage Trailer",
		"Stair Trailer",
		"Boxville",
		"Farm Plow",
		"Utility Trailer"
	}};

	constexpr inline std::array<stModelData, 212> model_data{{
		{ 100.0, 0 },           // 400
		{ 50.0, 0 },            // 401 - Bravura
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 55.0, 0 },            // 410 - Manana
		{ 100.0, 0 },
		{ 45.0, 0 },            // 412 - Voodoo
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 75.0, 0 },            // 418 - Moonbeam
		{ 50.0, 0 },            // 419 - Esperanto
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 50.0, 0 },            // 426 - Premier
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },           // 436 - Previon
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 60.0, 0 },            // 458 - Solair
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 20.0, 0 },            // 461 - PCJ-600
		{ 25.0, 0 },            // 462 - Faggio
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 80.0, 0 },            // 482 - Burrito
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 45.0, 0 },            // 491 - Virgo
		{ 50.0, 0 },            // 492 - Greenwood
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 55.0, 0 },            // 507 - Elegant
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 65.0, 0 },            // 516 - Nebula
		{ 50.0, 0 },            // 517 - Majestic
		{ 52.5, 0 },            // 518 - Buccaneer
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 27.5, 0 },            // 521 - FCR-900
		{ 30.0, 0 },            // 522 - NRG-500
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 57.5, 0 },            // 534 - Remington
		{ 65.0, 0 },            // 535 - Slamvan
		{ 50.0, 0 },            // 536  Blade
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 50.0, 0 },            // 547 - Primo
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 45.0, 0 },            // 567 - Savanna
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 42.5, 0 },            // 576 - Tornado
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 },
		{ 100.0, 0 }
	}};

	extern std::array<std::unique_ptr<CVehicle>, MAX_VEHICLES> vehicle_pool;

	enum params : std::uint8_t
	{
		engine = 0,
		lights,
		alarm,
		doors,
		bonnet,
		boot,
		objective
	};

	/*
	union vehicle_params
	{
		std::uint8_t all;
		struct
		{
			unsigned char engine : 1;
			unsigned char lights : 1;
			unsigned char alarm : 1;
			unsigned char doors : 1;
			unsigned char bonnet : 1;
			unsigned char boot : 1;
			unsigned char objective : 1;
		};
	};
	*/
}

class CVehicle
{
public:
	enum class engine_state : std::uint8_t
	{
		off = 0,
		on = 1,
		default_state = 2
	};

	enum gearbox : std::int8_t
	{
		R = -1,
		N = 0,
		first = 1,
		second = 2,
		third = 3,
		fourth = 4
	};

private:
	std::uint16_t _vehicleid{ INVALID_VEHICLE_ID };
	int _dbid{ 0 };
	CPlayer* _owner{ nullptr };

	std::uint16_t _modelid;
	glm::vec4 _position;
	int _world;
	int _interior;
	float _health{ 1000.F };

	std::pair<int, int> _color;
	std::uint8_t _paintjob{ 3 };
	float _fuel{ 0.F };
	bool _locked{ false };
	bool _alarm{ false };
	std::array<std::uint16_t, 14> _components;
	std::int8_t _gearbox{ gearbox::N };

	struct
	{
		timers::CTimer* update{ nullptr };
		timers::CTimer* toggle_engine{ nullptr };
	} _timers;

	void Update(timers::CTimer* timer);
	void StartUpdating();
	void StopUpdating();

	CVehicle(std::uint16_t modelid, glm::vec4 position, std::pair<int, int> color);

public:
	static inline constexpr float VEHICLE_FUEL_DIVISOR = 20000.F;

	~CVehicle();

	static CVehicle* create(std::uint16_t modelid, glm::vec4 position, std::pair<int, int> color);
	// This is so niggerish...
	static void destroy(CVehicle* vehicle);

	inline std::uint16_t ID() const { return _vehicleid; }
	inline bool Valid() const { return _vehicleid != INVALID_VEHICLE_ID; }
	inline float GetSpeed() const 
	{
		float x, y, z;
		GetVehicleVelocity(_vehicleid, &x, &y, &z);
		return VectorSize(x, y, z) * 180.F;
	}
	
	inline engine_state Engine() const
	{
		int engine, param;
		GetVehicleParamsEx(_vehicleid, &engine, &param, &param, &param, &param, &param, &param);
		return static_cast<engine_state>(engine);
	}

	void ToggleEngine(engine_state state = engine_state::default_state);
	void ToggleEngineByPlayer(CPlayer* player);
	CPlayer* GetDriver();

	IO_GETTER_SETTER(DbId, _dbid)
	IO_GETTER_SETTER(Owner, _owner)
	IO_GETTER_SETTER(Components, _components)

	void SetFuel(float fuel);
	void AddFuel(float fuel);
	float GetFuel() const;

	void SetHealth(float health);
	float GetHealth() const;
	void Repair();

	void SetPosition(glm::vec3 position);
	void SetPosition(glm::vec4 position);
	void SetZAngle(float angle);
	glm::vec4 GetPosition() const;

	void SetColor(std::pair<int, int> color);
	std::pair<int, int> GetColor() const;
	
	void SetPaintjob(std::uint8_t paintjob);
	std::uint8_t GetPaintjob() const;

	void SetDamageStatus(int panels, int doors, int lights, int tires);
	std::tuple<int, int, int, int> GetDamageStatus() const;

	void SetInterior(int id);
	int GetInterior() const;

	void SetVirtualWorld(int worldid);
	int GetVirtualWorld() const;

	std::tuple<int, int, int, int, int, int, int> GetParams() const;
	std::bitset<7> GetParamsBitset() const;

	std::uint16_t GetModel() const;

	void Lock();
	void Unlock();
	bool Locked() const;
};
