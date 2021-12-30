#pragma once

#define DEFINE_GETTER_SETTER(getter,setter,variable) \
	inline auto getter() const { return variable; }\
	auto* setter(decltype(variable) value) { variable = value; Update(); return this; }

class CPlayer;

namespace server 
{
	namespace textdraw
	{
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
		glm::vec3 rotation{ 0.f,0.f,0.f };
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
		~TextDraw();

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
		~PlayerTextDraw();

		void Show();
		void Hide();
		inline bool Shown() const { return _id != 0xFFFF; }
	
		PlayerTextDraw* SetText(std::string text) override;
	};
}

#undef DEFINE_GETTER_SETTER