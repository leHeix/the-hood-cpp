#pragma once

class CPlayer;

namespace server
{
    // Some forwards
    class TextDraw;
    class PlayerTextDraw;
    class BaseTextDraw;
    struct stTextDrawData;

    cell DestroyPlayerTextDraws(std::uint16_t playerid, std::uint8_t reason);

    namespace textdraw 
    {
        cell OnPlayerClickTextDraw(std::uint16_t playerid, std::uint16_t clickedid);
    }

    class TextDrawIndexManager
    {
        friend cell server::textdraw::OnPlayerClickTextDraw(std::uint16_t playerid, std::uint16_t clickedid);

        std::bitset<2304> _slots;
        robin_hood::unordered_map<const BaseTextDraw*, std::uint16_t> _ids;

    public:
        inline std::uint16_t ClaimFreeId(const BaseTextDraw* td)
        {
            for (uint16_t bit = 0u; bit < _slots.size(); ++bit)
            {
                if (!_slots[bit])
                {
                    _slots.flip(bit);
                    _ids[td] = bit;
                    return bit;
                }
            }
            return 0xFFFF;
        }

        inline void FreeId(std::uint16_t id)
        {
            _slots.set(id, false);
        }

        inline void FreeId(const BaseTextDraw* td)
        {
            if (Shown(td))
            {
                _slots.set((*this)[td], false);
                _ids.erase(td);
            }
        }

        std::uint16_t operator[](const BaseTextDraw* td)
        {
            return (!_ids.contains(td) ? (_ids[td] = ClaimFreeId(td)) : _ids[td]);
        }

        inline bool Shown(const BaseTextDraw* td) const
        {
            return _ids.contains(td);
        }
    };

    class TextDrawList
    {
        friend class TextDrawManager;
        friend cell server::DestroyPlayerTextDraws(std::uint16_t playerid, std::uint8_t reason);

        std::vector<std::unique_ptr<TextDraw>> _textdraws;
        std::array<std::vector<std::unique_ptr<PlayerTextDraw>>, MAX_PLAYERS> _player_textdraws;
        std::vector<stTextDrawData> _ptd_data; // Ordered

        void CreateForPlayer(CPlayer* player);
        void DestroyForPlayer(std::uint16_t playerid);
    public:
        TextDrawList(const std::string_view file);

        void Show(CPlayer* player);
        void Show(CPlayer* player, unsigned short first, unsigned short last);
        void Show(CPlayer* player, unsigned short global_first, unsigned short global_last, unsigned short player_first, unsigned short player_last);
        void Hide(CPlayer* player);
        void Hide(CPlayer* player, unsigned short first, unsigned short last);

        inline std::vector<stTextDrawData>& PlayerTextData() { return _ptd_data; }
        inline std::vector<std::unique_ptr<TextDraw>>& GetGlobalTextDraws() { return _textdraws; }
        std::vector<std::unique_ptr<PlayerTextDraw>>& GetPlayerTextDraws(CPlayer* player);
    };

    class TextDrawManager
    {
        friend class TextDrawList;
        friend cell server::DestroyPlayerTextDraws(std::uint16_t playerid, std::uint8_t reason);

        std::unordered_map<std::string, std::unique_ptr<TextDrawList>> _td_lists;

    public:
        TextDrawManager() = default;
        ~TextDrawManager() = default;

        TextDrawList* LoadFile(const std::string_view file, const std::string& id);
        TextDrawList* operator[](const std::string& name)
        {
            return _td_lists.contains(name) ? _td_lists[name].get() : nullptr;
        }
    };
};

extern server::TextDrawManager textdraw_manager;