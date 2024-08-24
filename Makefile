# Constants and Logic
TRUE := T
FALSE :=
NOT = $(if $1,$(FALSE),$(TRUE))
EMPTY :=
BACKSLASH := \$(EMPTY)
QUOTE := "

# Helper functions
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
makewithdirext=$(patsubst $2%$3,$4%$5,$1)
makeobj=$(call makewithdirext,$1,$(SRCDIR),$(SRC_EXTENSION),$(OBJDIR),$(OBJ_EXTENSION))
makedepend=$(call makewithdirext,$1,$(SRCDIR),$(SRC_EXTENSION),$(DEPENDDIR),$(DEPENDS_EXTENSION))
makestatic=$(call makewithdirext,$1,$(LIBDIR)lib,$(STATIC_LIB_EXTENSION),,)
makedynamic=$(call makewithdirext,$1,$(LIBDIR)lib,$(DYNAMIC_LIB_EXTENSION),,)
map=$(foreach e,$2,$(call $1,$e))

# OS-specific variables and functions
ifeq ($(OS),Windows_NT)

IS_WINDOWS := $(TRUE)
MKDIR = $(if $1,if not exist $(QUOTE)$1$(QUOTE) mkdir $(QUOTE)$1$(QUOTE))
REMOVE = $(if $1,del /Q $1)
ECHO = $(if $1,echo $1,echo.)
STATIC_LIB_EXTENSION := .lib
DYNAMIC_LIB_EXTENSION := .dll
TOUCH = type nul >>"$(call backslashize,$1)" & copy "$(call backslashize,$1)" +,, >nul
EXEC_EXTENSION := .exe
backslashize = $(subst /,$(BACKSLASH),$1)

else

IS_WINDOWS := $(FALSE)
MKDIR = $(if $1,mkdir -p "$1")
REMOVE = $(if $1,rm -rf "$1")
ECHO = echo "$1"
STATIC_LIB_EXTENSION := .lib
DYNAMIC_LIB_EXTENSION := .so
TOUCH = touch $1
EXEC_EXTENSION :=
backslashize = $1

endif

# Programs
COMPILER := g++
EMPTY_ECHO := echo ""
RUNNER :=

# Extensions
SRC_EXTENSION := .cpp
OBJ_EXTENSION := .o
DEPENDS_EXTENSION := .d

# Directories
SRCDIR := source/
INCDIR := source/
BUILDDIR := build/
OBJDIR := $(BUILDDIR)objects/
BINDIR := $(BUILDDIR)bin/
LIBDIR := libs/
DEPENDDIR := $(BUILDDIR)depends/
BASE_DIRS := $(sort $(SRCDIR) $(INCDIR) $(BUILDDIR) $(OBJDIR) $(BINDIR) $(LIBDIR) $(DEPENDDIR))

# Output Options
EXECNAME := indexer
ARGS := 

# Files
SRCS := $(call rwildcard,$(SRCDIR),*$(SRC_EXTENSION))
STATIC_LIBS := $(call rwildcard,$(LIBDIR),*$(STATIC_LIB_EXTENSION))
DYNAMIC_LIBS := $(call rwildcard,$(LIBDIR),*$(DYNAMIC_LIB_EXTENSION))
ALL_OBJS := $(call rwildcard,$(OBJDIR),*$(OBJ_EXTENSION))
DIRS := $(sort $(dir $(SRCS)))
DEPEND_DIRS := $(DIRS:$(SRCDIR)%=$(DEPENDDIR)%)
OBJECT_DIRS := $(DIRS:$(SRCDIR)%=$(OBJDIR)%)
EXECUTABLE := $(BINDIR)$(EXECNAME)$(EXEC_EXTENSION)
MAIN_FILE := $(SRCDIR)$(EXECNAME)$(SRC_EXTENSION)

# Compiler options
DEBUG := $(TRUE)
OPTIMIZATION_LEVEL := $(if $(DEBUG),0,3)
ENABLED_WARNINGS := all extra
DISABLED_WARNINGS := unused-parameter unknown-pragmas
ERRORS := $(ENABLED_WARNINGS) $(DISABLED_WARNINGS:%=no-%)
DEFINES := $(if $(DEBUG),DEBUG,RELEASE) _GLIBCXX_HAVE_STACKTRACE
FLAGS := PIC $(if $(DEBUG),lto) $(if $(and $(DEBUG),$(call NOT,$(IS_WINDOWS))),sanitize=leak sanitize=address)
STANDARD := c++17

# Linker options
LIBS := m gdi32 comdlg32 comctl32 uuid oleaut32 ole32 dwmapi user32 dbghelp

# Make the options suitable for passing to compiler/flagize them
_DEBUG := $(if $(DEBUG),-g3 -gdwarf-2)
_ERRORS := $(ERRORS:%=-W%)
_DEFINES := $(DEFINES:%=-D%)
_FLAGS := $(FLAGS:%=-f%)
_STANDARD := -std=$(STANDARD)

# Make the linker options to suitable flags
_FOUND_LIBS_STATIC := $(call map,makestatic,$(STATIC_LIBS))
_FOUND_LIBS_DYNAMIC := $(call map,makedynamic,$(DYNAMIC_LIBS))
_FOUND_LIBS := $(_FOUND_LIBS_STATIC) $(_FOUND_LIBS_DYNAMIC)
FOUND_LIBS_STATIC_PREFIX := $(basename $(notdir $(_FOUND_LIBS)))
FOUND_LIBS := $(FOUND_LIBS_STATIC_PREFIX) $(basename $(notdir $(_FOUND_LIBS_DYNAMIC)))
_LIB_DIRS := $(sort $(dir $(_FOUND_LIBS)))
LIB_DIRS := $(_LIB_DIRS:%=-L"%")
LIBS_FLAG := $(LIBS:%=-l%)
FOUND_LIBS_FLAG := $(FOUND_LIBS:%=-l"%")
_LIBS := $(FOUND_LIBS_FLAG) $(LIBS_FLAG)

CFLAGS := -I"./$(INCDIR)" -O$(OPTIMIZATION_LEVEL) $(_DEFINES) $(_STANDARD) $(_FLAGS) $(_ERRORS) $(_DEBUG) $(STACKTRACE) -static
LDFLAGS := $(CFLAGS) $(LIB_DIRS) $(_LIBS)

# Processed files
OBJS := $(call map,makeobj,$(SRCS))
DEPENDS := $(call map,makedepend,$(SRCS))

run: $(EXECUTABLE)
	@$(call ECHO,RUNNING $(EXECNAME))
	@$(call ECHO,===================)
	@$(call ECHO,)
	@$(EXECUTABLE) $(ARGS)
.PHONY: run

$(EXECUTABLE): $(OBJS) | $(BINDIR)
	@$(call ECHO,LINKING $(EXECNAME))
	@$(COMPILER) $^ -o $(EXECUTABLE) $(LDFLAGS)

build: $(EXECUTABLE)
.PHONY: build

all: build
.PHONY: all

clean:
	@echo RM $(ALL_OBJS)
	@$(call REMOVE,$(foreach o,$(ALL_OBJS),"$(call backslashize,$o)"))
.PHONY: clean

DEPEND_FILE = $(patsubst $(SRCDIR)%$(SRC_EXTENSION),$(DEPENDDIR)%$(DEPENDS_EXTENSION),$<)
OBJ_FILE = $(patsubst $(SRCDIR)%$(SRC_EXTENSION),$(OBJDIR)%$(OBJ_EXTENSION),$<)
$(OBJDIR)%$(OBJ_EXTENSION) $(DEPENDDIR)%$(DEPENDS_EXTENSION): $(SRCDIR)%$(SRC_EXTENSION) | $(OBJECT_DIRS) $(DEPEND_DIRS)
	@$(call ECHO,COMPILE $<)
	@$(COMPILER) $(CFLAGS) -MMD -MP -MF $(DEPEND_FILE) -c $< -o $(OBJ_FILE)

ALL_DIRS := $(sort $(OBJECT_DIRS) $(DEPEND_DIRS) $(BASE_DIRS))
$(ALL_DIRS):
	@$(call ECHO,MKDIR $@)
	@mkdir "$(call backslashize,$@)"

init: $(BASE_DIRS)
	@$(call ECHO,Initializing Folders)
	@$(call ECHO,Initialization Completed)
.PHONY: init

debug:
	@$(call ECHO,$(DEPENDS))
.PHONY: debug

-include $(wildcard $(DEPENDS))
