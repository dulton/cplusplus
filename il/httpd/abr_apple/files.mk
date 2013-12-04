#################################################################
SOURCE_PATH  += src


APP_FILES += main.cpp
APP_FILES += abr_transaction.cpp
APP_FILES += abr_status.cpp
APP_FILES += abr_parser.cpp
APP_FILES += abr_algorithm.cpp
APP_FILES += abr_error.cpp
APP_FILES += abr_timeout.cpp
APP_FILES += abr_debug.cpp
APP_FILES += my_player.cpp

#################################################################
INCLUDE_PATH += $(SOURCE_PATH)


INCLUDE_PATH += /home/ay/MyLib/boost/
INCLUDE_PATH += /home/ay/MyLib/ace/

LIBS = /home/ay/MyLib/lib/libace.a
PROJECT_FILES +=   $(APP_FILES) $(LIBS)


