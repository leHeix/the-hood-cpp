#pragma once

#ifdef __cplusplus

#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <fstream>
#include <functional>
#include <thread>
#include <type_traits>
#include <algorithm>
#include <bitset>
#include <chrono>
#include <memory>
#include <array>
#include <any>
#include <optional>
#include <regex>
#include <stack>
#include <atomic>
#include <cstring>
#include <cstdint>
#include <queue>
#include <bit>
#include <numbers>
#include <syncstream>

#else

#include <string.h>
#include <stdint.h>

#endif

#ifdef _WIN32
	#include <Windows.h>
	#include <WinSock2.h>
#elif defined __linux__
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif