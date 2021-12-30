#pragma once

namespace sqlite
{
	class Database;
	class Statement;
	class Row;

	class Row
	{
		Statement* _stmt;
		std::unordered_map<std::string, sqlite3_value*> _columns;

		friend class Statement;
	public:
		explicit Row(Statement* stmt);
		~Row();

		template<class T>
		std::optional<T> Get(const std::string& column)
		{
			if (!_columns.contains(column))
				return std::nullopt;

			int type = sqlite3_value_type(_columns[column]);

			if (type == SQLITE_NULL)
			{
				return std::nullopt;
			}

			if constexpr (std::is_integral_v<T>)
			{
				if (type != SQLITE_INTEGER)
					return std::nullopt;

				return sqlite3_value_int(_columns[column]);
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				if (type != SQLITE_FLOAT)
					return std::nullopt;

				return sqlite3_value_double(_columns[column]);
			}
			else if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>)
			{
				if (type != SQLITE_TEXT)
					return std::nullopt;

				return std::string(reinterpret_cast<const char*>(sqlite3_value_text(_columns[column])));
			}
		}
	};

	class Database
	{
	private:
		sqlite3* _handle;

	public:
		explicit Database(const std::string_view path);
		~Database();

		inline sqlite3* Handle() { return _handle; }
		inline const sqlite3* Handle() const { return _handle; }

		void Exec(const std::string_view query);
		std::unique_ptr<Statement> Prepare(const std::string& query);
	};

	class Statement
	{
		Database* _handle;
		sqlite3_stmt* _statement;
		std::atomic<bool> _has_row{ false };
		std::atomic<bool> _finished{ false };
		std::shared_ptr<sqlite::Row> _current_row;

		friend class Row;
	public:
		Statement(Database* db, const std::string& query);
		~Statement();

		template<size_t Index, class T>
		void Bind(const T& value)
		{
			int error = SQLITE_OK;
			if constexpr (std::is_integral_v<T>)
			{
				error = sqlite3_bind_int(_statement, Index, value);
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				error = sqlite3_bind_double(_statement, Index, value);
			}
			else if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>)
			{
				error = sqlite3_bind_text(_statement, Index, value.c_str(), value.size(), SQLITE_STATIC);
			}
			else if constexpr (std::is_same_v<std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<T>>>, char*>)
			{
				error = sqlite3_bind_text(_statement, Index, value, strlen(value), SQLITE_STATIC);
			}

			if (error != SQLITE_OK)
			{
				throw std::runtime_error{ std::string(sqlite3_errmsg(_handle->Handle())) };
			}
		}

		void Step();
		bool Finished();
		bool HasRow();
		// Rows don't depend on the current state of the statement
		std::shared_ptr<sqlite::Row> Row();
	};
}