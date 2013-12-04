################################################################################
#Comment the line below to debug make
ifneq "$(V)" "1"
.SILENT :
endif

##### PROJECT ARGUMENTS  #######################################################
TARGET   = abr_player
OBJ_DIR  = ../obj/abr_player/
BIN_DIR  = bin/

##### UBUNTU 10.10  ##############################################################
STANDARD_INCLUDE_PATH += /usr/include/

STANDARD_INCLUDE_PATH += /usr/include/c++/4.4/
STANDARD_INCLUDE_PATH += /usr/include/c++/4.4/backward
STANDARD_INCLUDE_PATH += /usr/include/gcc/4.4/i686-linux-gnu
STANDARD_INCLUDE_PATH += /usr/include/gcc/4.4/i686-linux-gnu/bits

#STANDARD_LIBRARY_PATH  +=  
STANDARD_LIBRARY_FILES += libc.a 
STANDARD_LIBRARY_FILES += librt.a 
STANDARD_LIBRARY_FILES += libpthread.a 
STANDARD_LIBRARY_FILES += libm.a 
STANDARD_LIBRARY_FILES += libstdc++.so
STANDARD_LIBRARY_FILES += libdl.so


##### TOOL CHAIN  #############################################################
SED	 = sed
CP	 = cp
CC   = gcc
CPP  = gcc
LD   = gcc

##### COMPILER & UTILITES FLAGS ################################################
C_FLAGS       = -O0 -g3 -Wall -c -fmessage-length=0  
CPP_FLAGS	  = -O0 -g3 -Wall -c -fmessage-length=0 
LINKER_FLAGS  =  

##### MACRO DEFINES  ###########################################################
PROJECT_DEFINES += -DACE_NTRACE=1
PROJECT_DEFINES += -DACE_NDEBUG=1

include files.mk
include rules.mk

#$(LIB_DIR)libace.a: 
#	echo $(LIB_FILES)
#	make -C libace all

##### MAIN TARGET ##############################################################
$(BIN_DIR)$(TARGET) : $(OBJECT_FILES)  $(PROJECT_FILES) files.mk
	@echo Linking ...
	$(LD)  -o $@  $(OBJECT_FILES) $(LIBRARY_FILES) $(LINKER_FLAGS)


##### MAIN RULE  ###############################################################
#.PHONY all:
all : $(BIN_DIR)$(TARGET) 
	 @echo make: $(BIN_DIR)$(TARGET) is up to date 

#### CLEAN RULE  ###############################################################
.PHONY clean:

clean:
	@echo ------------------------- Cleaning $(PRJ_NAME) -------------------------
	@rm -vf  $(INTERMEDIATE_FILES) $(INTERMEDIATE_DIRECTORIES)
	@rm -vrf $(OBJ_DIR)
	@rm -vrf $(BIN_DIR)
	
