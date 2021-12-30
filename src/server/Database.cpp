#include "../main.hpp"

sqlite::Statement::Statement(Database* db, const std::string& query, std::optional<std::reference_wrapper<std::atomic<bool>>> safe_stmt)
	: _handle(db), _safe_stmt_done(safe_stmt)
{
	int error = sqlite3_prepare_v2(db->Handle(), query.c_str(), query.length(), &_statement, nullptr);
	if (error == SQLITE_ERROR)
	{
		throw std::runtime_error{ fmt::format("(Error {}): {}", error, sqlite3_errmsg(_handle->Handle())) };
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
		sqlite3_clear_bindings(_statement);
		sqlite3_reset(_statement);
		_finished = true;
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
		_columns.insert({ sqlite3_column_name(_stmt->_statement, col), sqlite3_value_dup(sqlite3_column_value(_stmt->_statement, col)) });
	}
}

sqlite::Row::~Row()
{
	for (auto&& [column, value] : _columns)
	{
		sqlite3_value_free(value);
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