# - Try to find Jsoncpp
# Once done, this will define
#
#  JSONCPP_FOUND - system has Jsoncpp
#  JSONCPP_INCLUDE_DIRS - the Jsoncpp include directories
#  JSONCPP_LIBRARIES - link these to use Jsoncpp

# Include dir
find_path(JSONCPP_INCLUDE_DIR
  NAMES json/features.h
  PATH_SUFFIXES jsoncpp
  PATHS /usr/include/jsoncpp/json
)

# Finally the library itself
find_library(JSONCPP_LIBRARY
	NAMES jsoncpp
	PATHS
	/usr/lib
	/usr/lib64
	./jsoncpp/
)

#set(JSONCPP_PROCESS_INCLUDES JSONCPP_INCLUDE_DIR)
#set(JSONCPP_PROCESS_LIBS JSONCPP_LIBRARY)
#libfind_process(JSONCPP) 
