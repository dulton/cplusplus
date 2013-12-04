.PHONY : FORCE
CLEAN    := $(filter clean,$(MAKECMDGOALS))

INCLUDE_PATH        := $(sort $(addprefix -I,$(STANDARD_INCLUDE_PATH))) $(sort $(addprefix -I,$(INCLUDE_PATH))) 

MAKE_FILES			+= makefile rules.mk

DEPENDENCE_FILES    += $(addprefix $(OBJ_DIR),$(subst .c,.d,$(filter %.c,$(PROJECT_FILES))))
DEPENDENCE_FILES    += $(addprefix $(OBJ_DIR),$(subst .cc,.d,$(filter %.cc,$(PROJECT_FILES))))
DEPENDENCE_FILES    += $(addprefix $(OBJ_DIR),$(subst .cpp,.d,$(filter %.cpp,$(PROJECT_FILES))))

OBJECT_FILES	    += $(addprefix $(OBJ_DIR),$(subst .c,.o,$(filter %.c,$(PROJECT_FILES))))
OBJECT_FILES	    += $(addprefix $(OBJ_DIR),$(subst .cc,.o,$(filter %.cc,$(PROJECT_FILES))))
OBJECT_FILES	    += $(addprefix $(OBJ_DIR),$(subst .cpp,.o,$(filter %.cpp,$(PROJECT_FILES))))
OBJECT_FILES	    += $(filter %.o,$(PROJECT_FILES))

LIB_FILES   += $(filter %.a,$(PROJECT_FILES)) 
LIB_FILES   += $(filter %.a,$(STANDARD_LIBRARY_FILES)) 
LIB_FILES   += $(filter %.so,$(PROJECT_FILES)) 
LIB_FILES   += $(filter %.so,$(STANDARD_LIBRARY_FILES)) 
LIB_PATH    := $(addprefix -L ,$(sort $(dir $(LIB_FILES)))) $(addprefix -L ,$(sort $(dir $(STANDARD_LIBRARY_PATH)))) 
LIB_FILES   := $(subst lib,-l,$(basename $(notdir $(LIB_FILES)))) 

LIBRARY_FILES = $(LIB_PATH) $(LIB_FILES)


INTERMEDIATE_FILES  = $(DEPENDENCE_FILES) $(OBJECT_FILES) 
INTERMEDIATE_DIRECTORIES = $(sort $(dir $(INTERMEDIATE_FILES))) $(BIN_DIR) $(LIB_DIR)

ifneq "$(INTERMEDIATE_DIRECTORIES)" ""
ifneq "$(INTERMEDIATE_DIRECTORIES)" "./"
INTERMEDIATE_DIRECTORIES := $(addsuffix MakeDir,$(INTERMEDIATE_DIRECTORIES))
endif
endif

vpath %.c   $(SOURCE_PATH)
vpath %.C   $(SOURCE_PATH)
vpath %.cpp $(SOURCE_PATH)
vpath %.CPP $(SOURCE_PATH)
vpath %.cc  $(SOURCE_PATH)
vpath %.CC  $(SOURCE_PATH)

$(INTERMEDIATE_FILES) : $(INTERMEDIATE_DIRECTORIES) $(CLEAN) $(MAKE_FILES) 

###############################################################################
# Rule to create a directory 
###############################################################################
%/MakeDir:
	+@if ( test ! -d $(@D) ); then mkdir -p $(@D) ; fi
	+@echo MakeDir > $@

###############################################################################
# Compilation and build dependancy of a single C or CPP file 
###############################################################################

$(OBJ_DIR)%.o : %.c 
	@echo Compiling $*.c ...
	+$(CC) $(C_FLAGS) $(INCLUDE_PATH) $(PROJECT_DEFINES)    -o $@ $< $(MESSAGE_FILTER)

$(OBJ_DIR)%.d : %.c
	@echo Building dependency for $*.c ...
	+@$(CC) $(C_FLAGS) $(INCLUDE_PATH) $(PROJECT_DEFINES)   -M $< | $(SED) -e "1s*^*$(OBJ_DIR)$*.d $(OBJ_DIR)$*.o *g" > $@

$(OBJ_DIR)%.o : %.cpp
	@echo Compiling $*.cpp ...
	+$(CPP) $(CPP_FLAGS) $(INCLUDE_PATH) $(PROJECT_DEFINES) -o $@ $< $(MESSAGE_FILTER)
	
$(OBJ_DIR)%.d : %.cpp
	+@echo Building dependency for $*.cpp ... 
	+$(CPP) $(CPP_FLAGS) $(INCLUDE_PATH) $(PROJECT_DEFINES) -M $< | $(SED) -e "1s*^*$(OBJ_DIR)$*.d $(OBJ_DIR)$*.o *g" > $@

$(OBJ_DIR)%.o : %.cc
	@echo Compiling $*.cc ...
	+$(CPP) $(CPP_FLAGS) $(INCLUDE_PATH) $(PROJECT_DEFINES) -o $@ $< $(MESSAGE_FILTER)
	
$(OBJ_DIR)%.d : %.cc
	+@echo Building dependency for $*.cc ... 
	+$(CPP) $(CPP_FLAGS) $(INCLUDE_PATH) $(PROJECT_DEFINES) -M $< | $(SED) -e "1s*^*$(OBJ_DIR)$*.d $(OBJ_DIR)$*.o *g" > $@

		
ifneq "$(filter clean,$(MAKECMDGOALS))" "clean"
   -include $(DEPENDENCE_FILES)
endif

