#include "../main.hpp"

sqlite::Statement::Statement(Database* db, const std::string& query, std::optional<std::reference_wrapper<std::atomic<bool>>> safe_stmt)
	: _handle(db), _query(query), _safe_stmt_done(safe_stmt)
{
	int error = sqlite3_prepare_v2(_handle->Handle(), _query.c_str(), _query.length(), &_statement, &_remaining);
	if (error == SQLITE_ERROR)
	{
		std::string err = fmt::format("(Error {}): {}", error, sqlite3_errmsg(_handle->Handle()));
		sampgdk::logprintf("[stmt] Failed to create:");
		sampgdk::logprintf("[stmt]    %s", err.c_str());

		throw std::runtime_error{ err };
	}

	if (_remaining)
	{
		while (isspace(*_remaining))
			_remaining++;
	}
}

sqlite::Statement::~Statement()
{
	if (_safe_stmt_done)
	{
		std::scoped_lock lk(_handle->_mtx);
		_safe_stmt_done->get() = true;
		_handle->_cv.notify_one();
	}

	sqlite3_finalize(_statement);
}

void sqlite::Statement::Step()
{
	_current_row.reset();
	_has_row = false;

	int error = sqlite3_step(_statement);
	if (error != SQLITE_DONE && error != SQLITE_ROW)
	{
		throw std::runtime_error{ fmt::format("(Error {}): {}", error, sqlite3_errmsg(_handle->Handle())) };
	}
	
	if (error == SQLITE_DONE)
	{
		if (_remaining && _remaining[0])
		{
			sqlite3_finalize(_statement);
			_statement = nullptr;

			do
			{
				int error = sqlite3_prepare_v2(_handle->Handle(), _remaining, -1, &_statement, &_remaining);
				if (error == SQLITE_ERROR)
				{
					throw std::runtime_error{ fmt::format("(Error {}): {}", error, sqlite3_errmsg(_handle->Handle())) };
				}
			} while (!_statement);
		}
		else
		{
			_finished = true;
		}
	}
	else if (error == SQLITE_ROW)
		_has_row = true;
}

bool sqlite::Statement::HasRow()
{
	return _has_row;
}

bool sqlite::Statement::Finished()
{
	return _finished;
}

std::shared_ptr<sqlite::Row> sqlite::Statement::Row()
{
	if (!_has_row)
		return nullptr;

	// Save it instead of re-creating a row every time this function is called
	if (_current_row)
		return _current_row;

	_current_row = std::make_shared<sqlite::Row>(this);
	return _current_row;
}

sqlite::Row::Row(Statement* stmt)
	: _stmt(stmt)
{
	for (size_t col = 0, count = sqlite3_column_count(stmt->_statement); col < count; ++col)
	{
		auto val = sqlite3_value_dup(sqlite3_column_value(_stmt->_statement, col));
		std::unique_ptr<sqlite3_value, decltype(&sqlite3_value_free)> val_ptr{ val, sqlite3_value_free };

		_columns.insert({ sqlite3_column_name(_stmt->_statement, col), std::move(val_ptr) });
	}
}

sqlite::Database::Database(const std::string_view path)
{
	if (std::filesystem::exists(path))
	{
		if (!std::filesystem::exists(std::filesystem::current_path() / "scriptfiles" / "backups"))
		{
			std::filesystem::create_directories(std::filesystem::current_path() / "scriptfiles" / "backups");
		}

		std::filesystem::path p{ std::filesystem::current_path() / "scriptfiles" / "backups" / fmt::format("database-{:%d-%m-%Y--%H%M%S}.db", fmt::localtime(std::time(nullptr))) };
		std::filesystem::copy_file(path, p);
	}

	int error = sqlite3_open(path.data(), &_handle);
	if (error != SQLITE_OK)
	{
		throw std::runtime_error{ fmt::format("(Error {}): {}", error, sqlite3_errmsg(_handle)) };
	}
}

sqlite::Database::~Database()
{
	sqlite3_close_v2(_handle);
}

void sqlite::Database::Exec(const std::string_view query)
{
	char* error{ nullptr };
	if (sqlite3_exec(_handle, query.data(), nullptr, nullptr, &error) != SQLITE_OK)
	{
		std::string error_str(error);
		sqlite3_free(error);
		throw std::runtime_error{ error_str };
	}
}

std::unique_ptr<sqlite::Statement> sqlite::Database::Prepare(const std::string& query)
{
	std::unique_lock lk(_mtx);
	_cv.wait(lk, [&] { return _safe_stmt_done.load(); });

	return std::make_unique<sqlite::Statement>(this, query, std::nullopt);
}

std::unique_ptr<sqlite::Statement> sqlite::Database::PrepareLock(const std::string& query)
{
	std::unique_lock lk(_mtx);
	_cv.wait(lk, [&] { return _safe_stmt_done.load(); });

	_safe_stmt_done = false;
	return std::make_unique<sqlite::Statement>(this, query, std::ref(_safe_stmt_done));
}
