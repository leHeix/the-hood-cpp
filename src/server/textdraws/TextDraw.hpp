#pragma once

#define DEFINE_GETTER_SETTER(getter,setter,variable) \
	inline auto getter() const { return variable; }\
	auto* setter(decltype(variable) value) { variable = value; Update(); return this; }

class CPlayer;

namespace server 
{
	namespace textdraw
	{
		// Width arays and width methods taken from https://github.com/kristoisberg/samp-td-string-width/

		constexpr std::array<std::uint8_t, 4> TDCharacterDefaultWidth = { 27, 20, 27, 20 };
		constexpr std::array<std::array<std::uint8_t, 176>, 4> TDCharacterWidth = { {
			{
				 0,  12,  12,  12,  12,  12,  12,  12,
				12,  12,  12,  12,  12,  12,  12,  12,
				12,  12,  12,  12,  12,  12,  12,  12,
				12,  12,  12,  12,  12,  12,  12,  12,
				12,  13,  13,  28,  28,  28,  28,   8,
				17,  17,  30,  28,  28,  12,   9,  21,
				28,  14,  28,  28,  28,  28,  28,  28,
				28,  28,  13,  13,  30,  30,  30,  30,
				10,  25,  23,  21,  24,  22,  20,  24,
				24,  17,  20,  22,  20,  30,  27,  27,
				26,  26,  24,  23,  24,  31,  23,  31,
				24,  23,  21,  28,  33,  33,  14,  28,
				10,  11,  12,   9,  11,  10,  10,  12,
				12,   7,   7,  13,   5,  18,  12,  10,
				12,  11,  10,  12,   8,  13,  13,  18,
				17,  13,  12,  30,  30,  37,  35,  37,
				25,  25,  25,  25,  33,  21,  24,  24,
				24,  24,  17,  17,  17,  17,  27,  27,
				27,  27,  31,  31,  31,  31,  11,  11,
				11,  11,  11,  20,   9,  10,  10,  10,
				10,   7,   7,   7,   7,  10,  10,  10,
				10,  13,  13,  13,  13,  27,  12,  30
			}, {
				 0,  15,  15,  15,  15,  15,  15,  15,
				15,  15,  15,  15,  15,  15,  15,  15,
				15,  15,  15,  15,  15,  15,  15,  15,
				15,  15,  15,  15,  15,  15,  15,  15,
				15,   9,  17,  27,  20,  34,  23,  12,
				12,  12,  21,  20,  12,  14,  12,  15,
				23,  15,  21,  21,  21,  21,  21,  21,
				20,  21,  12,  12,  24,  24,  24,  19,
				10,  22,  19,  19,  22,  16,  19,  24,
				22,  11,  16,  21,  15,  28,  24,  27,
				20,  25,  19,  19,  18,  23,  23,  31,
				23,  19,  21,  21,  13,  35,  11,  21,
				10,  19,  20,  14,  20,  19,  13,  20,
				19,   9,   9,  19,   9,  29,  19,  21,
				19,  19,  15,  15,  14,  18,  19,  27,
				20,  20,  17,  21,  17,  20,  15,  15,
				22,  22,  22,  22,  29,  19,  16,  16,
				16,  16,  11,  11,  11,  11,  27,  27,
				27,  27,  23,  23,  23,  23,  20,  19,
				19,  19,  19,  30,  14,  19,  19,  19,
				19,   9,   9,   9,   9,  21,  21,  21,
				21,  18,  18,  18,  18,  24,  19,  19
			}, {
				 0,  15,  23,  15,  21,  21,  21,  21,
				21,  21,  20,  21,  12,  12,  24,  24,
				24,  19,  10,  22,  19,  19,  22,  16,
				19,  24,  22,  11,  16,  21,  15,  28,
				12,  13,  13,  28,  37,  28,  30,   8,
				17,  17,  30,  28,  28,  12,   9,  21,
				27,  16,  27,  27,  27,  27,  27,  27,
				27,  27,  18,  13,  30,  30,  30,  30,
				10,  29,  26,  25,  28,  26,  25,  27,
				28,  12,  24,  25,  24,  30,  27,  29,
				26,  26,  25,  26,  25,  26,  28,  32,
				27,  26,  26,  28,  33,  33,  10,  28,
				10,  29,  26,  25,  28,  26,  25,  27,
				28,  12,  24,  25,  24,  30,  27,  29,
				26,  26,  25,  26,  25,  26,  28,  32,
				27,  26,  26,  30,  30,  37,  35,  37,
				29,  29,  29,  29,  33,  25,  26,  26,
				26,  26,  14,  14,  14,  14,  29,  29,
				29,  29,  26,  26,  26,  26,  21,  29,
				29,  29,  29,  33,  25,  26,  26,  26,
				26,  14,  14,  14,  14,  29,  29,  29,
				29,  26,  26,  26,  26,  25,  25,  30
			}, {
				 0,   9,   9,  18,  18,  18,  18,  18,
				18,  18,  18,  19,  19,  19,   0,   9,
				 9,   9,   9,  18,  18,  18,  18,  18,
				18,  18,  18,  19,  19,  19,   0,   9,
				15,  10,  17,  27,  20,  34,  23,  10,
				15,  15,  21,  20,  12,  14,   9,  15,
				20,  18,  19,  19,  21,  19,  19,  19,
				19,  19,  16,  12,  24,  24,  24,  21,
				10,  19,  19,  19,  20,  19,  16,  19,
				19,   9,  19,  20,  14,  29,  19,  19,
				19,  19,  19,  19,  21,  19,  20,  32,
				21,  19,  19,  21,  13,  35,  10,  21,
				10,  19,  19,  19,  20,  19,  16,  19,
				19,   9,  19,  20,  14,  29,  19,  19,
				19,  19,  19,  19,  21,  19,  20,  32,
				21,  19,  19,  21,  17,  20,  15,  15,
				19,  19,  19,  19,  29,  19,  19,  19,
				19,  19,   9,   9,   9,   9,  19,  19,
				19,  19,  19,  19,  19,  19,  19,  19,
				19,  19,  19,  29,  19,  19,  19,  19,
				19,   9,   9,   9,   9,  19,  19,  19,
				19,  19,  19,  19,  19,  21,  21,  19
			}
		}};
		constexpr std::array<std::uint8_t, 32> TDFont3CharacterInlineWidth = {
			 0, 255,   0,   0, 128,  63, 147,  36,
			19,  64,   0,   0,   0,   0,   0,   0,
			32,  68,   0,   0,   0,   0,   0,   0,
			 0,   0,   0,   0,   0,   0,   0,   0
		};

		std::uint8_t GetTextDrawCharacterWidth(std::uint8_t character, std::uint8_t font, bool proportional = true);
		int GetTextDrawStringWidth(const std::string& string, std::uint8_t font, std::uint8_t outline = 0, bool proportional = true);
		int GetTextDrawLineWidth(const std::string& string, std::uint8_t font, std::uint8_t outline = 0, bool proportional = true, int start = 0, int end = -1);
		int GetTextDrawLineCount(const std::string& string);
		bool SplitTextDrawString(std::string& string, float max_width, float letter_size, std::uint8_t font, std::uint8_t outline = 0, bool proportional = true);
		
		cell OnPlayerClickTextDraw(std::uint16_t playerid, std::uint16_t clickedid);

		enum alignment : std::uint8_t
		{
			left = 1,
			right = 2,
			centered = 3
		};
	}

	struct stTextDrawData
	{
		bool box{ false };
		std::uint8_t alignment{ 3 };
		bool proportional{ false };
		std::pair<float, float> letter_size{ 0.f, 0.f };
		std::uint32_t letter_color{ 0 };
		std::pair<float, float> line_size{ 0.f, 0.f };
		std::uint32_t box_color{ 0 };
		std::uint8_t shadow{ 0 };
		std::uint8_t outline{ 1 };
		std::uint32_t background_color{ 0 };
		std::uint8_t style{ 1 };
		bool selectable{ false };
		std::pair<float, float> position{ 0.f, 0.f };

		std::uint16_t modelid{ 0 };
		glm::vec3 rotation{ 0.f, 0.f, 0.f };
		float zoom{ 0.f };
		std::pair<std::uint16_t, std::uint16_t> preview_colors{ 0,0 };
		std::string text{ "_" };
		std::function<void(CPlayer*)> callback;
	};

	class BaseTextDraw
	{
		friend cell server::textdraw::OnPlayerClickTextDraw(std::uint16_t playerid, std::uint16_t clickedid);

	protected:
		stTextDrawData _data{};

		virtual void Update() = 0;
	public:
		virtual ~BaseTextDraw() = default;
	
		DEFINE_GETTER_SETTER(UsingBox, ToggleBox, _data.box)
		DEFINE_GETTER_SETTER(GetAlignment, SetAlignment, _data.alignment)
		DEFINE_GETTER_SETTER(IsProportional, ToggleProportional, _data.proportional)
		DEFINE_GETTER_SETTER(GetLetterSize, SetLetterSize, _data.letter_size)
		DEFINE_GETTER_SETTER(GetLetterColor, SetLetterColor, _data.letter_color)
		DEFINE_GETTER_SETTER(GetLineSize, SetLineSize, _data.line_size)
		DEFINE_GETTER_SETTER(GetBoxColor, SetBoxColor, _data.box_color)
		DEFINE_GETTER_SETTER(GetShadowLevel, SetShadowLevel, _data.shadow)
		DEFINE_GETTER_SETTER(GetOutlineLevel, SetOutlineLevel, _data.outline)
		DEFINE_GETTER_SETTER(GetBackgroundColor, SetBackgroundColor, _data.background_color)
		DEFINE_GETTER_SETTER(GetFont, SetFont, _data.style)
		DEFINE_GETTER_SETTER(IsSelectable, ToggleSelectable, _data.selectable)
		DEFINE_GETTER_SETTER(GetPosition, SetPosition, _data.position)
		DEFINE_GETTER_SETTER(GetPreviewModelID, SetPreviewModelID, _data.modelid)
		DEFINE_GETTER_SETTER(GetPreviewModelRotation, SetPreviewModelRotation, _data.rotation)
		DEFINE_GETTER_SETTER(GetPreviewModelZoom, SetPreviewModelZoom, _data.zoom)
		DEFINE_GETTER_SETTER(GetPreviewModelColors, SetPreviewModelColors, _data.preview_colors)
		inline auto GetText() const { return _data.text; }
		// An implementation might choose to optimize this function to send a single TextDrawSetString RPC
		virtual inline BaseTextDraw* SetText(std::string text) { _data.text = text; Update(); return this; }

		inline BaseTextDraw* SetCallback(const std::function<void(CPlayer*)>& callback) { _data.callback = callback; return this; }

		inline void CopyData(const stTextDrawData& data) { _data = data; }
		inline stTextDrawData GetData() const { return _data; }
	};

	class TextDraw final : public BaseTextDraw
	{
		friend cell server::DestroyPlayerTextDraws(std::uint16_t playerid, std::uint8_t reason);
		std::bitset<MAX_PLAYERS> _shown_for;
		std::stack<stTextDrawData> _states;

		void Update() override;
		void Update(CPlayer* player);
	public:
		TextDraw() = default;
		~TextDraw() override;

		void PushState();
		void PopState();

		void Show();
		void Show(CPlayer* player);
		void Hide(CPlayer* player);
		void Hide();
		inline bool ShownFor(CPlayer* player) const;

		TextDraw* SetText(std::string text) override;
	};

	class PlayerTextDraw final : public BaseTextDraw
	{
		std::uint16_t _playerid;
		std::uint16_t _id{ 0xFFFF };

		void Update() override;
	public:
		explicit PlayerTextDraw(std::uint16_t player)
			: _playerid(player)
		{}
		~PlayerTextDraw() override;

		void Show();
		void Hide();
		inline bool Shown() const { return _id != 0xFFFF; }
	
		PlayerTextDraw* SetText(std::string text) override;
	};
}

#undef DEFINE_GETTER_SETTER
