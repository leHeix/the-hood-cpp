#pragma once

namespace shops
{
	cell OnPlayerKeyStateChange(std::uint16_t playerid, std::uint32_t newkeys, std::uint32_t oldkeys);
	cell OnPlayerObjectMoved(std::uint16_t playerid, std::uint16_t objectid);

	struct stShopItem
	{
		friend cell shops::OnPlayerKeyStateChange(std::uint16_t playerid, std::uint32_t newkeys, std::uint32_t oldkeys);
		friend cell shops::OnPlayerObjectMoved(std::uint16_t playerid, std::uint16_t objectid);
		friend class CShop;

		std::string name;
		int model;
		int price;

	private:
		glm::vec3 rotation;
	};

	class CShop
	{
	private:
		friend cell shops::OnPlayerKeyStateChange(std::uint16_t playerid, std::uint32_t newkeys, std::uint32_t oldkeys);
		friend cell shops::OnPlayerObjectMoved(std::uint16_t playerid, std::uint16_t objectid);
		friend class CShopManager;

		int label;
		int area;

		std::string name;
		glm::vec3 position;
		int world;
		int interior;

		glm::vec3 cam_pos;
		glm::vec3 cam_look_at;

		struct 
		{
			glm::vec3 start;
			glm::vec3 idle;
			glm::vec3 end;
		} object_pos;

		std::function<bool(CShop*, CPlayer*, stShopItem*)> _callback;
		std::vector<std::unique_ptr<stShopItem>> _shop_items;

	public:
		stShopItem* AddItem(const std::string& name, int model, int price, const glm::vec3& rotation);
		inline void SetObjectPositions(glm::vec3 start, glm::vec3 idle, glm::vec3 end)
		{
			object_pos.start = start;
			object_pos.idle = idle;
			object_pos.end = end;
		}

		inline void SetCallback(std::function<bool(CShop*, CPlayer*, stShopItem*)> callback)
		{
			_callback = callback;
		}

		inline auto& Items() { return _shop_items; }
		inline bool HasCallback() const noexcept { return !!_callback; }
		inline bool TriggerCallback(CPlayer* player, stShopItem* item) { return _callback(this, player, item); }
		IO_GETTER_SETTER(ObjectPos, object_pos)
	};

	class CShopManager
	{
		friend cell shops::OnPlayerKeyStateChange(std::uint16_t playerid, std::uint32_t newkeys, std::uint32_t oldkeys);
		std::vector<std::unique_ptr<CShop>> _shops;

		struct player_shop_data
		{
			decltype(CShop::_shop_items)::iterator selected_item;
			int object;
		};
		std::array<player_shop_data, MAX_PLAYERS> _player_data;

	public:
		CShopManager() = default;
		~CShopManager() = default;

		CShop* Create(const std::string& name, glm::vec3 position, int world, int interior, std::pair<glm::vec3, glm::vec3> camera);
		inline player_shop_data& PlayerData(std::uint16_t playerid) { return _player_data[playerid]; }
		inline player_shop_data& PlayerData(CPlayer* player);
	};
}

extern std::unique_ptr<shops::CShopManager> shop_manager;