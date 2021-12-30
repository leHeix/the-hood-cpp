#
##
## Libraries
##
#

#
## SQLite
#
add_library(SQLite3 STATIC
	"./sqlite3/sqlite3.c"
)
target_compile_definitions(SQLite3 PUBLIC
	SQLITE_USE_ALLOCA
	$<IF:$<PLATFORM_ID:Linux>,HAS_LOCALTIME_R,HAS_LOCALTIME_S>
	HAVE_ISNAN
	SQLITE_DQS=0
	SQLITE_ENABLE_MATH_FUNCTIONS
	SQLITE_DEFAULT_MEMSTATUS=0
	SQLITE_THREADSAFE=1
	SQLITE_MAX_EXPR_DEPTH=0
	SQLITE_OMIT_SHARED_CACHE
	SQLITE_OMIT_DEPRECATED
	SQLITE_OMIT_PROGRESS_CALLBACK
	SQLITE_ENABLE_UPDATE_DELETE_LIMIT
)
target_compile_features(SQLite3 PRIVATE c_std_11)
target_include_directories(SQLite3 PRIVATE "./sqlite3")

#
## SAMP-SDK
#
add_library(SAMPSDK STATIC
	"samp-sdk/amxplugin.cpp"
	"samp-sdk/amxplugin2.cpp"
)

if(UNIX)
	target_sources(SAMPSDK PUBLIC "samp-sdk/amx/getch.c")
endif()

target_include_directories(SAMPSDK PUBLIC "./samp-sdk")

#
## SAMP-GDK
#
add_library(SAMPGDK STATIC
	"samp-gdk/sampgdk.c"
)
target_compile_definitions(SAMPGDK PUBLIC 
	SAMPGDK_STATIC=1
	SAMPGDK_AMALGAMATION=1
)
target_include_directories(SAMPGDK PUBLIC "./samp-sdk" "./samp-gdk")
target_link_libraries(SAMPGDK PUBLIC SAMPSDK)

#
## RakNet
#
add_library(RakNet STATIC
	"raknet/BitStream.cpp"
)
target_include_directories(RakNet PUBLIC "./raknet")

#
## Botan cryptographic library
#

if(WIN32)
	set(BOTAN_SRC_DIR "botan/src/win")
else()
	set(BOTAN_SRC_DIR "botan/src/linux")
endif()

add_library(Botan STATIC "${BOTAN_SRC_DIR}/botan_all.cpp")
target_compile_features(Botan PRIVATE cxx_std_17)
target_link_libraries(Botan PRIVATE Threads::Threads)

if(MSVC)
	set(COMPILE_FEATURES_RELEASE /EHs /GR /O2 /Oi /bigobj)
	set(COMPILE_FEATURES_DEBUG /EHs /GR /Zi /FS /bigobj)
	set(COMPILE_FEATURES /arch:SSE2 $<IF:$<NOT:$<CONFIG:Debug>>,${COMPILE_FEATURES_RELEASE},${COMPILE_FEATURES_DEBUG}>)
	set(COMPILER_WARNINGS /W4 /wd4250 /wd4251 /wd4275)
else()
	set(COMPILE_FEATURES_RELEASE -O3 -fstack-protector)
	set(COMPILE_FEATURES_DEBUG -g -fstack-protector)
	set(COMPILE_FEATURES -msse2 $<IF:$<NOT:$<CONFIG:Debug>>,${COMPILE_FEATURES_RELEASE},${COMPILE_FEATURES_DEBUG}>)
	set(COMPILER_WARNINGS -Wall -Wextra -Wpedantic -Wshadow -Wstrict-aliasing -Wstrict-overflow=5 -Wcast-align -Wmissing-declarations -Wpointer-arith -Wcast-qual -Wshorten-64-to-32)
endif()

target_compile_options(Botan PRIVATE ${COMPILER_WARNINGS} ${COMPILE_FEATURES})
target_include_directories(Botan PRIVATE "botan/include" "botan/include/external")
target_compile_definitions(Botan PRIVATE
	BOTAN_DLL=
	_REENTRANT=
)