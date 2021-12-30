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