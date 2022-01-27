#pragma once

namespace commands
{
	template<class T>
	concept Float = std::is_floating_point_v<T>;

	class argument_store
	{
		bool _final{ false };
		std::string _data;
		std::size_t _parsed{ 0u };
		std::uint16_t _parsed_arguments{ 0u };

		std::string GetNextChunk();
		struct final_t {};
	public:
		static inline const final_t final = {};

		explicit argument_store(const std::string& data)
			: _data(data), _parsed(data.empty() ? std::string::npos : 0u)
		{}

		inline bool empty() const noexcept { return _parsed == std::string::npos; }

		inline void set_arguments(const std::string& data)
		{
			_data = data;
			_parsed = (data.empty() ? std::string::npos : 0u);
		}

		argument_store& operator>>(CPlayer*& data);
		argument_store& operator>>(int& data);
		template<Float T>
		argument_store& operator>>(T& data)
		{
			std::string chunk = GetNextChunk();

			char* p;
			double val = std::strtod(chunk.c_str(), &p);
			if (*p != '\0')
				throw type_error{ "floating-point number", "not all data on chunk is a floating-point number" };

			data = static_cast<T>(val);
			++_parsed_arguments;
			return *this;
		}

		argument_store& operator>>(std::string& data);
		argument_store& operator>>(const final_t& f);
		template<class T>
		argument_store& operator>>(T& data)
		{
			static_assert(std::is_convertible_v<T, int>);
			return ((*this) >> static_cast<int&>(data));
		}

		class type_error : public std::runtime_error
		{
			std::string _expected;

		public:
			type_error(const std::string_view expected, const std::string& what = "") : std::runtime_error(what), _expected(expected) {}

			std::string expected() const { return _expected; }
		};
	};
}
