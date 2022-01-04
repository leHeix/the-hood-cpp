#include "../../main.hpp"

server::TextDrawManager textdraw_manager{};

server::TextDrawList::TextDrawList(const std::string_view file)
{
	toml::table tbl = toml::parse_file(file);

	if (!tbl.contains("textdraws"))
		throw std::runtime_error{ "invalid textdraw file" };

	toml::array* tdarray = tbl["textdraws"].as_array();
	if (!tdarray)
		throw std::runtime_error{ "invalid textdraw file" };

	for (auto&& td_node : *tdarray)
	{
		auto* td_data_ptr = td_node.as_table();
		if (!td_data_ptr)
			continue;
		
		const auto& td = *td_data_ptr;

		stTextDrawData td_data{};
#define IF_EXISTS_INSERT(key, type, data) \
		if(auto value = td[key].value<type>()) \
			td_data.data = *value

#define IF_EXISTS_INSERT_PAIR(key, type, data, default_val) \
		if(auto* arr_ptr = td[key].as_array(); arr_ptr && arr_ptr->size() >= 2) \
		{ \
			auto& arr = *arr_ptr; \
			td_data.data = { arr[0].value_or<type>(default_val), arr[1].value_or<type>(default_val) }; \
		}

#define IF_EXISTS_INSERT_VEC3D(key, type, data, default_val) \
		if(auto* arr_ptr = td[key].as_array(); arr_ptr && arr_ptr->size() >= 3) \
		{ \
			auto& arr = *arr_ptr; \
			td_data.data = glm::vec3{ arr[0].value_or<type>(default_val), arr[1].value_or<type>(default_val), arr[2].value_or<type>(default_val) }; \
		}

		IF_EXISTS_INSERT_PAIR("position", float, position, 0.F);
		IF_EXISTS_INSERT("text", std::string, text);
		IF_EXISTS_INSERT("style", unsigned char, style);
		IF_EXISTS_INSERT_PAIR("letter_size", float, letter_size, 0.F);
		IF_EXISTS_INSERT_PAIR("line_size", float, line_size, 0.F);
		IF_EXISTS_INSERT("outline", unsigned char, outline);
		IF_EXISTS_INSERT("shadow", unsigned char, shadow);
		IF_EXISTS_INSERT("alignment", unsigned char, alignment);
		IF_EXISTS_INSERT("letter_color", int32_t, letter_color);
		IF_EXISTS_INSERT("bg_color", int32_t, background_color);
		IF_EXISTS_INSERT("box_color", int32_t, box_color);
		IF_EXISTS_INSERT("box", bool, box);
		IF_EXISTS_INSERT("proportional", bool, proportional);
		IF_EXISTS_INSERT("selectable", bool, selectable);
		IF_EXISTS_INSERT("modelid", unsigned short, modelid);
		IF_EXISTS_INSERT_VEC3D("rotation", float, rotation, 0.F);
		IF_EXISTS_INSERT("zoom", float, zoom);
		IF_EXISTS_INSERT_PAIR("model_colors", int16_t, preview_colors, -1);

#undef IF_EXISTS_INSERT_PAIR
#undef IF_EXISTS_INSERT
#undef IF_EXISTS_INSERT_VEC3D

		const bool player = td["player"].value_or<bool>(false);
		if (player)
		{
			_ptd_data.push_back(std::move(td_data));
		}
		else
		{
			auto td_ptr = std::make_unique<TextDraw>();
			td_ptr->CopyData(td_data);
			_textdraws.push_back(std::move(td_ptr));
		}
	}

	sampgdk::logprintf("[TextDraws] Loaded %i textdraws (%i public, %i per-player) from file %s.", _textdraws.size() + _ptd_data.size(), _textdraws.size(), _ptd_data.size(), file.data());
}

void server::TextDrawList::CreateForPlayer(CPlayer* player)
{
	auto playerid = player->PlayerId();
	_player_textdraws[playerid].clear();

	for (auto&& data : _ptd_data)
	{
		auto td = std::make_unique<PlayerTextDraw>(playerid);
		td->CopyData(data);
		_player_textdraws[playerid].push_back(std::move(td));
	}
}

void server::TextDrawList::DestroyForPlayer(std::uint16_t playerid)
{
	_player_textdraws[playerid].clear();
}

void server::TextDrawList::Show(CPlayer* player)
{
	if (_player_textdraws[player->PlayerId()].empty())
	{
		CreateForPlayer(player);
	}

	for (auto&& td : _textdraws)
	{
		td->Show(player);
	}

	for (auto&& td : _player_textdraws[player->PlayerId()])
	{
		td->Show();
	}
}

void server::TextDrawList::Show(CPlayer* player, unsigned short first, unsigned short last)
{
	if (_player_textdraws[player->PlayerId()].empty())
	{
		CreateForPlayer(player);
	}

	for (auto last_f = (last == (unsigned short)-1 ? _textdraws.size() : last); first < last_f; ++first)
	{
		_textdraws[first]->Show(player);
	}

	for (auto&& td : _player_textdraws[player->PlayerId()])
	{
		td->Show();
	}
}

void server::TextDrawList::Show(CPlayer* player, unsigned short global_first, unsigned short global_last, unsigned short player_first, unsigned short player_last)
{
	if (_player_textdraws[player->PlayerId()].empty())
	{
		CreateForPlayer(player);
	}

	if (global_first != (unsigned short)-1)
	{
		for (std::uint16_t last = (global_last == (unsigned short)-1 ? _textdraws.size() : global_last); global_first < last; ++global_first)
		{
			_textdraws[global_first]->Show(player);
		}
	}

	if (player_first != (unsigned short)-1)
	{
		auto playerid = player->PlayerId();
		for (std::uint16_t last = (player_last == (unsigned short)-1 ? _player_textdraws[playerid].size() : player_last); player_first < last; ++player_first)
		{
			_player_textdraws[playerid][player_first]->Show();
		}
	}

}

void server::TextDrawList::Hide(CPlayer* player)
{
	for (auto&& td : _textdraws)
	{
		td->Hide();
	}

	DestroyForPlayer(player->PlayerId());
}

server::TextDrawList* server::TextDrawManager::LoadFile(const std::string_view file, const std::string& id)
{
	std::filesystem::path filepath{ std::filesystem::current_path() / "scriptfiles" / "textdraws" / file };
	if (!filepath.has_extension())
		filepath = filepath.replace_extension(".toml");

	std::ifstream hfile{ filepath, std::ios::binary };
	if (!hfile)
	{
		sampgdk::logprintf("[TextDraw] Failed to parse file %s: couldn't open file", file.data());
		return nullptr;
	}

	auto hash_function = Botan::HashFunction::create("CRC32");
	std::stringstream file_content;
	file_content << hfile.rdbuf();
	std::string file_content_s = file_content.str();
	auto csum_bytes = hash_function->process(file_content_s);
	
	if (_td_lists.contains(id))
	{
		if (_td_lists[id].file_csum == csum_bytes)
			return _td_lists[id].list.get();

		_td_lists.erase(id);
	}

	try
	{
		_td_lists[id].list = std::make_unique<TextDrawList>(filepath.string());
		_td_lists[id].file_csum = std::move(csum_bytes);

		return _td_lists[id].list.get();
	}
	catch (const toml::parse_error& e)
	{
		sampgdk::logprintf("[TextDraw] Failed to parse file %s: %s", file.data(), e.what());
	}
	catch (const std::runtime_error& e)
	{

	}

	return nullptr;
}

std::vector<std::unique_ptr<server::PlayerTextDraw>>& server::TextDrawList::GetPlayerTextDraws(CPlayer* player)
{
	if (_player_textdraws[player->PlayerId()].empty())
		CreateForPlayer(player);

	return _player_textdraws[player->PlayerId()];
}

cell server::DestroyPlayerTextDraws(std::uint16_t playerid, std::uint8_t reason)
{
	for (auto&& [id, listfile] : textdraw_manager._td_lists)
	{
		listfile.list->_player_textdraws[playerid].clear();
	}

	return 1;
}

static CPublicHook<server::DestroyPlayerTextDraws> _tdm_opd("OnPlayerDisconnect");
