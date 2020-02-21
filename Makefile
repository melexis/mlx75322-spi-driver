ROOT_PRODUCT = 75322
PRODUCT ?= $(ROOT_PRODUCT)
DRIVER_NAME := lib75322_spi_drv

# RELEASE
# Name of the release to build in case of versioned_release
RELEASE_MATURITY = RC
RELEASE_MAJOR    = 1
RELEASE_MINOR    = 3
RELEASE_REVISION = 0

CC := gcc
ECHO := echo

include Make-$(PRODUCT).mk
#-----------------------------------------------------------------------------
# basic utilities (depends on the OS this Makefile runs on):
#
ifeq ($(OS),Windows_NT)
	MKDIR      := mkdir -p
	RM         := rm -rf
	TARGET_EXT := .dll
	CP         := cp -f
else ifeq ($(OSTYPE),cygwin)
	MKDIR      := mkdir -p
	RM         := rm -rf
	TARGET_EXT := .dll
	TARGET_EXE_EXT := .exe
	CP         := cp -f
else
	MKDIR      := mkdir -p
	RM         := rm -rf
	TARGET_EXT := .so
	TARGET_EXE_EXT :=
	CP         := cp -f
endif

OUT_DIR := $(CURDIR)/build
OBJ_DIR := $(CURDIR)/obj
SRC_DIRS := $(patsubst %, $(CURDIR)/src/$(ROOT_PRODUCT)/%/src, $(INCLUDE_COMPONENTS))
DOC_DIRS := $(patsubst %, $(CURDIR)/src/$(ROOT_PRODUCT)/%/doc, $(INCLUDE_COMPONENTS))

DRIVER_TARGET := $(OUT_DIR)/$(DRIVER_NAME)

SRCS = $(sort $(wildcard $(SRC_DIRS)/*.c))

SRCS_CFILES_PATT = $(addsuffix /*.c, $(SRC_DIRS))
SRCS_HFILES_PATT = $(addsuffix /*.h, $(SRC_DIRS))
SRCS_RSTFILES_PATT = $(addsuffix /*.rst, $(DOC_DIRS))

SRCS = $(sort $(wildcard $(SRCS_CFILES_PATT)))
RST_DOCS = $(sort $(wildcard $(SRCS_RSTFILES_PATT)))
HEADERS = $(sort $(wildcard $(SRCS_HFILES_PATT)))

RST_DOCS = $(sort $(wildcard $(DOC_DIR)/*.rst))

INCLUDES += $(patsubst %, -I%, $(SRC_DIRS))

CPPFLAGS  = -Wall
CPPFLAGS += -fpic
CPPFLAGS += -std=c99
CPPFLAGS += -c -g -ffunction-sections -fdata-sections -fms-extensions -O -Wall -W
CPPFLAGS += $(INCLUDES)
CPPFLAGS += $(addprefix -D, $(COMPONENT_FLAGS))
LIBS += $(addprefix -l, $(COMPONENT_LIBS))

OBJS = $(SRCS:$(CURDIR)/src/$(ROOT_PRODUCT)/%.c=$(OBJ_DIR)/%.o)
DEPS = $(patsubst %.o,%.d, $(OBJS))
DEPFLAGS = -Wp,-MM,-MP,-MT,$@,-MF,$(@:.o=.d) $(INCLUDES)

DEBUG ?= 2
ifneq ($(DEBUG), 3)
HIDE_CMD=@
endif

.PHONY: help
help:
	@echo "A build environment for the SPI driver and some explicit tools/helpers/docs for it"
	@echo
	@echo "Targets:"
	@echo "- help:         Print the information about a Makefile capabilities"
	@echo "- all:          Builds the driver and all helpers/tester/tools related"
	@echo "- doxy:         Builds the driver Doxygen documentation"
	@echo "- lib:          Builds the driver as a standalone library file"
	@echo "- clean:        Remove all files built"
	@echo
	@echo "Variables:"
	@echo "- DEBUG=3       Raised make debug level"

.PHONY: all
all: lib includes

.PHONY: lib
lib: $(DRIVER_TARGET)$(TARGET_EXT)

.PHONY: includes
includes: lib
	@echo "Copy component's header files"
	$(HIDE_CMD) $(MKDIR) $(OUT_DIR)/include
	$(HIDE_CMD) $(CP) $(HEADERS) -t $(OUT_DIR)/include

$(DRIVER_TARGET)$(TARGET_EXT): $(OBJS) $(DEPS)
	@$(MKDIR) $(OUT_DIR)
	$(HIDE_CMD)$(CC) -shared $(OBJS) $(LIBS) -o $@
	@echo "Build completed"

$(OBJS): $(OBJ_DIR)/%.o : $(CURDIR)/src/$(ROOT_PRODUCT)/%.c
	@$(MKDIR) $(dir $@)
	$(HIDE_CMD)$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) $< -o $@

$(DEPS): $(OBJ_DIR)/%.d : $(CURDIR)/src/$(ROOT_PRODUCT)/%.c
	@$(MKDIR) $(dir $@)
	$(HIDE_CMD)$(CC) -MM -MT $(@:.d=.o) $(CFLAGS) $(DEPFLAGS) $< > $@

.PHONY: clean
clean:
	$(HIDE_CMD)$(RM) $(OBJ_DIR)
	$(HIDE_CMD)$(RM) $(OUT_DIR)
	@echo "Cleaning completed"

.PHONY: doxy
doxy:
	$(HIDE_CMD)$(MAKE) -C ./doc $@ ROOT_PRODUCT=$(ROOT_PRODUCT) PRODUCT=$(PRODUCT) DEBUG=$(DEBUG)


DEPS = $(OBJS:%.o=%.d)
NODEPS_GOALS=stylechecker clean doxy all
ifeq ($(filter $(NODEPS_GOALS), ${MAKECMDGOALS}), )
	-include $(DEPS)
endif
