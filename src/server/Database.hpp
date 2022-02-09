#pragma once

namespace sqlite
{
	class Database;
	class Statement;
	class Row;

	class Row
	{

		Statement* _stmt;
		std::unordered_map<std::string, std::unique_ptr<sqlite3_value, decltype(&sqlite3_value_free)>> _columns;

		friend class Statement;
	public:
		explicit Row(Statement* stmt);
		~Row() = default;

		template<class T>
		std::optional<T> Get(const std::string& column)
		{
			if (!_columns.contains(column))
				return std::nullopt;

			auto* value_ptr = _columns.at(column).get();
			int type = sqlite3_value_type(value_ptr);

			if (type == SQLITE_NULL)
			{
				return std::nullopt;
			}

			if constexpr (std::is_integral_v<T>)
			{
				if (type != SQLITE_INTEGER)
					return std::nullopt;

				return sqlite3_value_int(value_ptr);
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				if (type != SQLITE_FLOAT)
					return std::nullopt;

				return sqlite3_value_double(value_ptr);
			}
			else if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::string>)
			{
				if (type != SQLITE_TEXT)
					return std::nullopt;

				return std::string(reinterpret_cast<const char*>(sqlite3_value_text(value_ptr)));
			}
		}
	};

	class Database
	{
	private:
		friend class Statement;

		mutable std::mutex _mtx;
		std::condition_variable _cv;
		sqlite3* _handle;
		std::atomic<bool> _safe_stmt_done{ true };

	public:
		explicit Database(const std::string_view path);
		~Database();

		inline sqlite3* Handle() { return _handle; }
		inline const sqlite3* Handle() const { return _handle; }

		void Exec(const std::string_view query);
		std::unique_ptr<Statement> Prepare(const std::string& query);
		std::unique_ptr<Statement> PrepareLock(const std::string& query);
	};

	class Statement
	{
		Database* _handle;
		sqlite3_stmt* _statement;
		std::atomic<bool> _has_row{ false };
		std::atomic<bool> _finished{ false };
		std::shared_ptr<sqlite::Row> _current_row;
		std::optional<std::reference_wrapper<std::atomic<bool>>> _safe_stmt_done;
		std::string _query;
		const char* _remaining{ nullptr };

		friend class Row;
	public:
		Statement(Database* db, const std::string& query, std::optional<std::reference_wrapper<std::atomic<bool>>> safe_stmt = std::nullopt);
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
				error = sqlite3_bind_text(_statement, Index, value.c_str(), -1, SQLITE_STATIC);
			}
			else if constexpr (std::is_same_v<std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<T>>>, char*>)
			{
				error = sqlite3_bind_text(_statement, Index, value, -1, SQLITE_STATIC);
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
		inline int LastInsertId() const { return sqlite3_last_insert_rowid(_handle->_handle); }
	};
}
