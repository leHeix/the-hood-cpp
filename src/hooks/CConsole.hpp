#pragma once

enum CON_VARTYPE 
{ 
	CON_VARTYPE_FLOAT, 
	CON_VARTYPE_INT, 
	CON_VARTYPE_BOOL, 
	CON_VARTYPE_STRING 
};

constexpr auto CON_VARFLAG_DEBUG = 1;
constexpr auto CON_VARFLAG_READONLY = 2;
constexpr auto CON_VARFLAG_RULE = 4;

struct ConsoleVariable_s
{
	CON_VARTYPE VarType;
	std::uint32_t VarFlags;
	void* VarPtr;
	void(*VARCHANGEFUNC)();
};

class CConsole
{
private:
	urmem::address_t _console;
	urmem::address_t _AddStringVariable_fun;
	urmem::address_t _GetStringVariable_fun;
	urmem::address_t _SetStringVariable_fun;
	urmem::address_t _GetIntVariable_fun;
	urmem::address_t _SetIntVariable_fun;
	urmem::address_t _GetBoolVariable_fun;
	urmem::address_t _ModifyVariableFlags_fun;
	urmem::address_t _FindVariable_fun;
	urmem::address_t _SendRules_fun;
	urmem::address_t _Execute_fun;

public:
	CConsole();

	void AddStringVariable(const char* szRule, int flags, const char* szString, void* changefunc);
	char* GetStringVariable(const char* szRule);
	void SetStringVariable(const char* szRule, const char* szString);
	int GetIntVariable(const char* szRule);
	void SetIntVariable(const char* szRule, int value);
	bool GetBoolVariable(const char* szRule);
	void ModifyVariableFlags(const char* szRule, int value);
	ConsoleVariable_s* FindVariable(const char* szRule);
	void SendRules(SOCKET s, const char* data, const sockaddr_in* to, int tolen);
	void Execute(const char* pExecLine);
};

namespace server
{
	extern std::unique_ptr<CConsole> console;
}