#include "../../main.hpp"

std::uint8_t server::textdraw::GetTextDrawCharacterWidth(std::uint8_t character, std::uint8_t font, bool proportional)
{
	if ((font < 0 || font > 3) || (character < 0 || character >= 176)) 
	{
		return 0;
	}

	int width;

	if (!proportional || character >= TDCharacterWidth[0].size()) 
	{
		width = TDCharacterDefaultWidth[font];
	}
	else 
	{
		width = TDCharacterWidth[font][character];
	}

	return width;
}

int server::textdraw::GetTextDrawStringWidth(const std::string& string, std::uint8_t font, std::uint8_t outline, bool proportional)
{
	size_t other, result, width;
	other = result = width = 0;

	for (size_t i, length = string.length(); i < length; ++i) {
		if (string[i] == '~') {
			if ((other = string.find('~', i + 1)) == std::string::npos) {
				return GetTextDrawLineWidth("Error: unmatched tilde", font, outline, proportional);
			}

			if (other == i + 2 && string[i + 1] == 'n') {
				if (result < width) {
					result = width;
				}

				width = 0;
			}

			i = other + 1;
		}
		else {
			if (font == 3 && (string[i] > 0 && string[i] < 32) && i != length - 1 && string.find("~n~", i + 1) != i + 1) {
				width += TDFont3CharacterInlineWidth[string[i]];
			}
			else {
				width += GetTextDrawCharacterWidth(string[i], font, proportional);
			}
		}
	}

	if (result < width) {
		result = width;
	}

	return result + (outline * 2);
}

int server::textdraw::GetTextDrawLineWidth(const std::string& string, std::uint8_t font, std::uint8_t outline, bool proportional, int start, int end)
{
	int other, width;
	other = width = 0;

	if (end == -1) 
	{
		end = string.length();
	}

	for (; start < end; start++) 
	{
		if (string[start] == '~') 
		{
			if ((other = string.find('~', start + 1)) == -1) 
			{
				return GetTextDrawLineWidth("Error: unmatched tilde", font, outline, proportional);
			}

			start = other + 1;
		}
		else 
		{
			if (font == 3 && (string[start] > 0 && string[start] < 32) && start != end - 1) 
			{
				width += TDFont3CharacterInlineWidth[string[start]];
			}
			else 
			{
				width += GetTextDrawCharacterWidth(string[start], font, proportional);
			}
		}
	}

	return width + (outline * 2);
}

int server::textdraw::GetTextDrawLineCount(const std::string& string)
{
	int count = 1;
	size_t i = 0;

	while ((i = string.find("~n~", i)) != std::string::npos)
	{
		i += 3;
		++count;
	}

	return count;
}

bool server::textdraw::SplitTextDrawString(std::string& string, float max_width, float letter_size, std::uint8_t font, std::uint8_t outline, bool proportional)
{
	std::uint16_t previous_space = 0u;
	size_t i = -1;

	while ((i = string.find(' ', ++i)) != std::string::npos)
	{
		if (letter_size * (float)GetTextDrawLineWidth(string, font, outline, proportional, previous_space, i) <= max_width)
			continue;

		string.erase(i, 1);
		string.insert(i, "~n~");

		previous_space = i + 3;
		i += 2;
	}

	/*
	if (GetTextDrawLineCount(string) == 1)
	{
		int last_break = 0;
		i = 0u;

		for (; i < string.length(); ++i)
		{
			if (GetTextDrawLineWidth(string, font, outline, proportional, last_break, i) < max_width)
				continue;

			string.insert(i, "~n~");
			i += 3;
			last_break = i;
		}
	}
	*/

	return true;
}

cell server::textdraw::OnPlayerClickTextDraw(std::uint16_t playerid, std::uint16_t clickedid)
{
	sampgdk::logprintf("OnPlayerClickTextDraw(%i, %i)", playerid, clickedid);

	auto* player = server::player_pool[playerid];
	
	if (player->TextDraws()._slots.test(clickedid))
	{
		auto it = std::find_if(player->TextDraws()._ids.begin(), player->TextDraws()._ids.end(), [clickedid](const auto& p) {
			return p.second == clickedid;
		});

		if (it != player->TextDraws()._ids.end())
		{
			if (it->first->_data.callback)
			{
				it->first->_data.callback(player);
			}
		}
	}

	return 1;
}

static CPublicHook<server::textdraw::OnPlayerClickTextDraw> _td_cb_opctd("OnPlayerClickTextDraw");

server::TextDraw::~TextDraw()
{
	Hide();
}

void server::TextDraw::PushState()
{
	_states.push(_data);
}

void server::TextDraw::PopState()
{
	if (!_states.empty())
		return;

	_data = std::move(_states.top());
	_states.pop();
}

void server::TextDraw::Show(CPlayer* player)
{
	auto id = player->TextDraws()[this];
	if (id != 0xFFFF)
	{
		_shown_for.set(player->PlayerId(), true);
		Update(player);
	}
}

void server::TextDraw::Show()
{
	for (auto&& [id, player] : server::player_pool)
	{
		Show(player.get());
	}
}

void server::TextDraw::Hide(CPlayer* player)
{
	if (_shown_for.test(player->PlayerId()))
	{
		BitStream bs;
		bs.Write<std::uint16_t>(player->TextDraws()[this]);
		net::RakServer->SendRPC(&bs, net::raknet::RPC_TextDrawHideForPlayer, player->PlayerId(), HIGH_PRIORITY, RELIABLE);
		player->TextDraws().FreeId(this);

		_shown_for.set(player->PlayerId(), false);
	}
}

void server::TextDraw::Hide()
{
	for (std::uint16_t bit = 0U; bit < _shown_for.size(); ++bit)
	{
		if (_shown_for[bit])
		{
			BitStream bs;
			bs.Write<std::uint16_t>(server::player_pool[bit]->TextDraws()[this]);
			net::RakServer->SendRPC(&bs, net::raknet::RPC_TextDrawHideForPlayer, server::player_pool[bit]->PlayerId(), HIGH_PRIORITY, RELIABLE);
			server::player_pool[bit]->TextDraws().FreeId(this);
		}
	}

	_shown_for.reset();
}

server::TextDraw* server::TextDraw::SetText(std::string text)
{
	_data.text = text;
	std::for_each(_data.text.begin(), _data.text.end(), [](char& c) {
		switch (c)
		{
			case 'à': c = (char)151; break;
			case 'á': c = (char)152; break;
			case 'â': c = (char)153; break;
			case 'ä': c = (char)154; break;
			case 'À': c = (char)128; break;
			case 'Á': c = (char)129; break;
			case 'Â': c = (char)130; break;
			case 'Ä': c = (char)131; break;
			case 'è': c = (char)157; break;
			case 'é': c = (char)158; break;
			case 'ê': c = (char)159; break;
			case 'ë': c = (char)160; break;
			case 'È': c = (char)134; break;
			case 'É': c = (char)135; break;
			case 'Ê': c = (char)136; break;
			case 'Ë': c = (char)137; break;
			case 'ì': c = (char)161; break;
			case 'í': c = (char)162; break;
			case 'î': c = (char)163; break;
			case 'ï': c = (char)164; break;
			case 'Ì': c = (char)138; break;
			case 'Í': c = (char)139; break;
			case 'Î': c = (char)140; break;
			case 'Ï': c = (char)141; break;
			case 'ò': c = (char)165; break;
			case 'ó': c = (char)166; break;
			case 'ô': c = (char)167; break;
			case 'ö': c = (char)168; break;
			case 'Ò': c = (char)142; break;
			case 'Ó': c = (char)143; break;
			case 'Ô': c = (char)144; break;
			case 'Ö': c = (char)145; break;
			case 'ù': c = (char)169; break;
			case 'ú': c = (char)170; break;
			case 'û': c = (char)171; break;
			case 'ü': c = (char)172; break;
			case 'Ù': c = (char)146; break;
			case 'Ú': c = (char)147; break;
			case 'Û': c = (char)148; break;
			case 'Ü': c = (char)149; break;
			case 'ñ': c = (char)174; break;
			case 'Ñ': c = (char)173; break;
			case '¡': c = (char)64; break;
			case '¿': c = (char)175; break;
			case '`': c = (char)177; break;
		}
	});

	if (_shown_for.any())
	{
		BitStream bs;
		bs.Write<uint16_t>(0U);
		bs.Write<uint16_t>(_data.text.size());
		bs.Write(_data.text.c_str(), _data.text.size());

		for (uint16_t bit = 0U; bit < _shown_for.size(); ++bit)
		{
			if (_shown_for[bit])
			{
				bs.SetWriteOffset(0);
				bs.Write<uint16_t>(server::player_pool[bit]->TextDraws()[this]);
				net::RakServer->SendRPC(&bs, net::raknet::RPC_TextDrawSetString, bit, HIGH_PRIORITY, RELIABLE, 0);
			}
		}
	}

	return this;
}

void server::TextDraw::Update()
{
	if (_shown_for.any())
	{
		std::uint8_t flags = _data.box;

		if (_data.proportional)
		{
			flags |= (1 << 4);
		}

		switch (_data.alignment)
		{
			case 1:
			{
				flags |= (1 << 1);
				break;
			}
			default:
			case 2:
			{
				flags |= (1 << 3);
				break;
			}
			case 3:
			{
				flags |= (1 << 2);
				break;
			}
		}

		BitStream bs;
		bs.Write<uint16_t>(0);
		bs.Write<uint8_t>(flags);
		bs.Write<float>(_data.letter_size.first);
		bs.Write<float>(_data.letter_size.second);
		bs.Write<uint32_t>((((_data.letter_color << 16) | _data.letter_color & 0xFF00) << 8) | (((_data.letter_color >> 16) | _data.letter_color & 0xFF0000) >> 8));
		bs.Write<float>(_data.line_size.first);
		bs.Write<float>(_data.line_size.second);
		bs.Write<uint32_t>((((_data.box_color << 16) | _data.box_color & 0xFF00) << 8) | (((_data.box_color >> 16) | _data.box_color & 0xFF0000) >> 8));
		bs.Write<uint8_t>(_data.shadow);
		bs.Write<uint8_t>(_data.outline);
		bs.Write<uint32_t>((((_data.background_color << 16) | _data.background_color & 0xFF00) << 8) | (((_data.background_color >> 16) | _data.background_color & 0xFF0000) >> 8));
		bs.Write<uint8_t>(_data.style);
		bs.Write<uint8_t>(_data.selectable);
		bs.Write<float>(_data.position.first);
		bs.Write<float>(_data.position.second);
		bs.Write<uint16_t>(_data.modelid);
		bs.Write<float>(_data.rotation.x);
		bs.Write<float>(_data.rotation.y);
		bs.Write<float>(_data.rotation.z);
		bs.Write<float>(_data.zoom);
		bs.Write<uint16_t>(_data.preview_colors.first);
		bs.Write<uint16_t>(_data.preview_colors.second);
		bs.Write<uint16_t>(_data.text.size());
		bs.Write(_data.text.c_str(), _data.text.size());

		for (uint16_t bit = 0U; bit < _shown_for.size(); ++bit)
		{
			if (_shown_for[bit])
			{
				bs.SetWriteOffset(0);
				bs.Write<uint16_t>(server::player_pool[bit]->TextDraws()[this]);
				net::RakServer->SendRPC(&bs, net::raknet::RPC_ShowTextDraw, bit, HIGH_PRIORITY, RELIABLE, 0);
			}
		}
	}
}

void server::TextDraw::Update(CPlayer* player)
{
	if (_shown_for.test(player->PlayerId()))
	{
		std::uint8_t flags = _data.box;

		if (_data.proportional)
		{
			flags |= (1 << 4);
		}

		switch (_data.alignment)
		{
			case 1:
			{
				flags |= (1 << 1);
				break;
			}
			default:
			case 2:
			{
				flags |= (1 << 3);
				break;
			}
			case 3:
			{
				flags |= (1 << 2);
				break;
			}
		}

		BitStream bs;
		bs.Write<uint16_t>(player->TextDraws()[this]);
		bs.Write<uint8_t>(flags);
		bs.Write<float>(_data.letter_size.first);
		bs.Write<float>(_data.letter_size.second);
		bs.Write<uint32_t>((((_data.letter_color << 16) | _data.letter_color & 0xFF00) << 8) | (((_data.letter_color >> 16) | _data.letter_color & 0xFF0000) >> 8));
		bs.Write<float>(_data.line_size.first);
		bs.Write<float>(_data.line_size.second);
		bs.Write<uint32_t>((((_data.box_color << 16) | _data.box_color & 0xFF00) << 8) | (((_data.box_color >> 16) | _data.box_color & 0xFF0000) >> 8));
		bs.Write<uint8_t>(_data.shadow);
		bs.Write<uint8_t>(_data.outline);
		bs.Write<uint32_t>((((_data.background_color << 16) | _data.background_color & 0xFF00) << 8) | (((_data.background_color >> 16) | _data.background_color & 0xFF0000) >> 8));
		bs.Write<uint8_t>(_data.style);
		bs.Write<uint8_t>(_data.selectable);
		bs.Write<float>(_data.position.first);
		bs.Write<float>(_data.position.second);
		bs.Write<uint16_t>(_data.modelid);
		bs.Write<float>(_data.rotation.x);
		bs.Write<float>(_data.rotation.y);
		bs.Write<float>(_data.rotation.z);
		bs.Write<float>(_data.zoom);
		bs.Write<uint16_t>(_data.preview_colors.first);
		bs.Write<uint16_t>(_data.preview_colors.second);
		bs.Write<uint16_t>(_data.text.size());
		bs.Write(_data.text.c_str(), _data.text.size());
		net::RakServer->SendRPC(&bs, net::raknet::RPC_ShowTextDraw, player->PlayerId(), HIGH_PRIORITY, RELIABLE, 0);
	}
}

inline bool server::TextDraw::ShownFor(CPlayer* player) const
{
	return _shown_for.test(player->PlayerId());
}

server::PlayerTextDraw::~PlayerTextDraw()
{
	Hide();
}

void server::PlayerTextDraw::Show()
{
	if (!Shown())
	{
		_id = server::player_pool[_playerid]->TextDraws().ClaimFreeId(this);
	}
	
	Update();
}

void server::PlayerTextDraw::Hide()
{
	if (Shown())
	{
		BitStream bs;
		bs.Write<std::uint16_t>(_id);
		net::RakServer->SendRPC(&bs, net::raknet::RPC_TextDrawHideForPlayer, _playerid, HIGH_PRIORITY, RELIABLE);
		server::player_pool[_playerid]->TextDraws().FreeId(this);
		_id = 0xFFFF;
	}
}

void server::PlayerTextDraw::Update()
{
	if (Shown())
	{
		std::uint8_t flags = _data.box;

		if (_data.proportional)
		{
			flags |= (1 << 4);
		}

		switch (_data.alignment)
		{
			case 1:
			{
				flags |= (1 << 1);
				break;
			}
			default:
			case 2:
			{
				flags |= (1 << 3);
				break;
			}
			case 3:
			{
				flags |= (1 << 2);
				break;
			}
		}

		BitStream bs;
		bs.Write<uint16_t>(_id);
		bs.Write<uint8_t>(flags);
		bs.Write<float>(_data.letter_size.first);
		bs.Write<float>(_data.letter_size.second);
		bs.Write<uint32_t>((((_data.letter_color << 16) | _data.letter_color & 0xFF00) << 8) | (((_data.letter_color >> 16) | _data.letter_color & 0xFF0000) >> 8));
		bs.Write<float>(_data.line_size.first);
		bs.Write<float>(_data.line_size.second);
		bs.Write<uint32_t>((((_data.box_color << 16) | _data.box_color & 0xFF00) << 8) | (((_data.box_color >> 16) | _data.box_color & 0xFF0000) >> 8));
		bs.Write<uint8_t>(_data.shadow);
		bs.Write<uint8_t>(_data.outline);
		bs.Write<uint32_t>((((_data.background_color << 16) | _data.background_color & 0xFF00) << 8) | (((_data.background_color >> 16) | _data.background_color & 0xFF0000) >> 8));
		bs.Write<uint8_t>(_data.style);
		bs.Write<uint8_t>(_data.selectable);
		bs.Write<float>(_data.position.first);
		bs.Write<float>(_data.position.second);
		bs.Write<uint16_t>(_data.modelid);
		bs.Write<float>(_data.rotation.x);
		bs.Write<float>(_data.rotation.y);
		bs.Write<float>(_data.rotation.z);
		bs.Write<float>(_data.zoom);
		bs.Write<uint16_t>(_data.preview_colors.first);
		bs.Write<uint16_t>(_data.preview_colors.second);
		bs.Write<uint16_t>(_data.text.size());
		bs.Write(_data.text.c_str(), _data.text.size());
		net::RakServer->SendRPC(&bs, net::raknet::RPC_ShowTextDraw, _playerid, HIGH_PRIORITY, RELIABLE, 0);
	}
}

server::PlayerTextDraw* server::PlayerTextDraw::SetText(std::string text)
{
	_data.text = text;
	std::for_each(_data.text.begin(), _data.text.end(), [](char& c) {
		switch (c)
		{
			case 'à': c = (char)151; break;
			case 'á': c = (char)152; break;
			case 'â': c = (char)153; break;
			case 'ä': c = (char)154; break;
			case 'À': c = (char)128; break;
			case 'Á': c = (char)129; break;
			case 'Â': c = (char)130; break;
			case 'Ä': c = (char)131; break;
			case 'è': c = (char)157; break;
			case 'é': c = (char)158; break;
			case 'ê': c = (char)159; break;
			case 'ë': c = (char)160; break;
			case 'È': c = (char)134; break;
			case 'É': c = (char)135; break;
			case 'Ê': c = (char)136; break;
			case 'Ë': c = (char)137; break;
			case 'ì': c = (char)161; break;
			case 'í': c = (char)162; break;
			case 'î': c = (char)163; break;
			case 'ï': c = (char)164; break;
			case 'Ì': c = (char)138; break;
			case 'Í': c = (char)139; break;
			case 'Î': c = (char)140; break;
			case 'Ï': c = (char)141; break;
			case 'ò': c = (char)165; break;
			case 'ó': c = (char)166; break;
			case 'ô': c = (char)167; break;
			case 'ö': c = (char)168; break;
			case 'Ò': c = (char)142; break;
			case 'Ó': c = (char)143; break;
			case 'Ô': c = (char)144; break;
			case 'Ö': c = (char)145; break;
			case 'ù': c = (char)169; break;
			case 'ú': c = (char)170; break;
			case 'û': c = (char)171; break;
			case 'ü': c = (char)172; break;
			case 'Ù': c = (char)146; break;
			case 'Ú': c = (char)147; break;
			case 'Û': c = (char)148; break;
			case 'Ü': c = (char)149; break;
			case 'ñ': c = (char)174; break;
			case 'Ñ': c = (char)173; break;
			case '¡': c = (char)64; break;
			case '¿': c = (char)175; break;
			case '`': c = (char)177; break;
			case '&': c = (char)38; break;
		}
	});

	if (Shown())
	{
		BitStream bs;
		bs.Write<uint16_t>(_id);
		bs.Write<uint16_t>(_data.text.size());
		bs.Write(_data.text.c_str(), _data.text.size());
		net::RakServer->SendRPC(&bs, net::raknet::RPC_TextDrawSetString, _playerid, HIGH_PRIORITY, RELIABLE, 0);
	}

	return this;
}