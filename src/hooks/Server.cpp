#include "../main.hpp"

static urmem::hook _ContainsInvalidChars_hook;
static bool ContainsInvalidChars(char* name)
{
	static const std::vector<char> valid_chars{
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'[', ']', '(', ')', '$', '@', '.', '_',
		'ï', 'ò', 'ù', 'ú', 'û', 'ü', 'ý', 'þ', 'ÿ', '÷', 'ø', 'ö',
		'Š', 'Œ', 'Ž', 'š', 'ž', 'Ÿ', 'õ', 'À', 'Á', 'Â', 'Ã', 'Ä',
		'Å', 'Æ', 'Ç', 'ñ', 'È', 'É', 'Ê', 'Ë', 'Ì', 'Í', 'Î', 'Ï',
		'Ð', 'Ñ', 'Ò', 'Ó', 'Ô', 'Õ', 'Ö', 'Ø', 'Ù', 'Ú', 'Û', 'Ü',
		'Ý', 'Þ', 'ß', 'à', 'á', 'â', 'ã', 'ä', 'å', 'î', 'ç', 'è',
		'é', 'ê', 'ë', 'ì', 'í', ' '
	};

	while (*name)
	{
		if (std::find(valid_chars.begin(), valid_chars.end(), *name++) == valid_chars.end())
			return true;
	}

	return false;
}

void server::hooks::Install()
{
	urmem::sig_scanner scanner;

	DWORD address = (DWORD)GetModuleHandle(NULL);
	if (!scanner.init(address))
	{
		sampgdk::logprintf("[server:hooks] Scanner initialization failed.");
		return;
	}

	urmem::address_t addr;
#ifdef _WIN32
	if (scanner.find("\x8B\x4C\x24\x04\x8A\x01\x84\xC0", "xxxxxxxx", addr))
	{
#else
	if(scanner.find("\x53\x8B\x5D\x00\x0F\xB6\x0B\x84\xC9\x74\x00\x66\x90", "xxx?xxxxxx?xx", addr))
	{
		addr -= 0x3;
#endif
		_ContainsInvalidChars_hook.install(addr, urmem::get_func_addr(&ContainsInvalidChars));
		sampgdk::logprintf("[server::hooks] Installed hook to ContainsInvalidChars");
	}
	else
	{
		sampgdk::logprintf("[server::hooks] Couldn't find ContainsInvalidChars.");
	}
}