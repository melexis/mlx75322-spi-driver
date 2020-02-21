ifndef PRODUCT
$(error 'Variable PRODUCT is not defined, and it seems like you are calling this Makefile directly. Please use make doc in project root.')
endif

ifndef DEBUG
DEBUG = 2
endif

ifeq ($(DEBUG), 3)
HIDE_CMD =
NO_OUTPUT =
else
HIDE_CMD = @
NO_OUTPUT = > /dev/null
endif


# Include the project make file to get hold of the project specific stuff
include ../Make-$(PRODUCT).mk

# Export some variables to the system environment for pick up by external
# tools like doxygen
RM := rm -rf
COPY := cp
MKDIR := mkdir -p
TOUCH := touch
ECHO := echo
SOFTLINK := ln -s
BUILDDIR	:= ../build/doc/$(PRODUCT)

# override the source location mentioned in ../Makefile to fix
# the relative offset ./doc/ we are on when we call make.
override SOURCES_DIR = ../src
COMMON_COMPONENT_ROOT = $(SOURCES_DIR)/common
CUSTOM_COMPONENT_ROOT = $(SOURCES_DIR)/$(ROOT_PRODUCT)
PRODUCT_SRCDIR = $(CUSTOM_COMPONENT_ROOT)
LIBRARY_ROOT = $(SOURCES_DIR)

# ==================================================
# Configure and figure out the components to be used
# ==================================================

# Figure out the name of each component without the full relative path.
COMMON_COMPONENTS = $(subst /,, $(subst $(COMMON_COMPONENT_ROOT), ,$(sort $(dir $(wildcard $(COMMON_COMPONENT_ROOT)/*/)))))
CUSTOM_COMPONENTS = $(subst /,, $(subst $(CUSTOM_COMPONENT_ROOT), ,$(sort $(dir $(wildcard $(CUSTOM_COMPONENT_ROOT)/*/)))))
# The LIBRARIES are defined from parent Make-product.mk (taking into account product specific configuration Make-<PRODUCT_NAME>.mk)

# Remove all the 'common' components from the list where a 'custom' component of the same name exists.
COMMON_COMPONENTS := $(filter-out $(CUSTOM_COMPONENTS), $(COMMON_COMPONENTS))

# Remove any explicitly excluded components from both COMMON and CUSTOM!
ifneq ($(INCLUDE_COMPONENTS),)
	COMMON_COMPONENTS := $(filter $(INCLUDE_COMPONENTS), $(COMMON_COMPONENTS))
	CUSTOM_COMPONENTS := $(filter $(INCLUDE_COMPONENTS), $(CUSTOM_COMPONENTS))
endif
COMMON_COMPONENTS := $(filter-out $(EXCLUDE_COMPONENTS), $(COMMON_COMPONENTS))
CUSTOM_COMPONENTS := $(filter-out $(EXCLUDE_COMPONENTS), $(CUSTOM_COMPONENTS))

COMMON_COMPONENT_DIRS = $(addprefix $(COMMON_COMPONENT_ROOT)/, $(COMMON_COMPONENTS))
CUSTOM_COMPONENT_DIRS = $(addprefix $(CUSTOM_COMPONENT_ROOT)/, $(CUSTOM_COMPONENTS))

COMPONENT_DIRS = $(COMMON_COMPONENT_DIRS)
COMPONENT_DIRS += $(CUSTOM_COMPONENT_DIRS)

# Internal variables.
.PHONY: help clean doxy

help:
	@$(ECHO) "Please use \`make <target>' where <target> is one of"
	@$(ECHO) "  doxy       to make standalone Doxygen HTML files"

$(BUILDDIR):
	$(HIDE_CMD)$(MKDIR) $@

$(BUILDDIR)/html/doxygen: $(BUILDDIR)
	$(HIDE_CMD)$(MKDIR) $@

# Doxygen documentation that is stored in the source files is referenced from the
# rest documentation, we need to make sure we publish it at the same time
DOXYGEN_INPUT = $(addsuffix /src, $(COMMON_COMPONENT_DIRS) $(CUSTOM_COMPONENT_DIRS)) $(PRODUCT_SRCDIR)
.PHONY: doxy
doxy: $(BUILDDIR)/html/doxygen
	@$(ECHO) "** Running doxygen on source code, destination of doxygen build is the final html folder."
	$(HIDE_CMD) DOXYGEN_INPUT="$(DOXYGEN_INPUT)" COMPONENT_FLAGS="$(COMPONENT_CPPFLAGS)" doxygen doxygen/doxyfile $(NO_OUTPUT)

.PHONY: clean
clean:
	@$(ECHO) "Cleaning..."
	$(HIDE_CMD)rm -rf $(BUILDDIR)/html
