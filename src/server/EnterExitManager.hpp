#pragma once

class CEnterExitManager
{
	struct enter_exit
	{
		struct position_data {
			glm::vec4 position;
			int world;
			int interior;
			int pickup;
			int label;
			int area;
		};

		position_data enter;
		position_data exit;
		std::function<bool(CPlayer*, bool)> callback;
	};

	std::vector<enter_exit> _enter_exits;

public:
	void Create(int pickup_model, const std::string& enter_text, const std::string& exit_text, glm::vec4 enter_pos, int enter_world, int enter_interior, glm::vec4 exit_pos, int exit_world, int exit_interior, std::function<bool(CPlayer*, bool)> callback = nullptr);
	inline const enter_exit& Get(int id) const { return _enter_exits.at(id); }
};

extern std::unique_ptr<CEnterExitManager> enter_exits;
