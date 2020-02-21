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


# Makefile for Sphinx documentation

.PHONY: make-doc html generate
make-doc: html

# Include the projec make file to get hold of the project specific stuff
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
GENERATION_DIR := source/generated
SPHINX_SETTINGS := $(GENERATION_DIR)/sphinx_settings_$(PRODUCT).py
PLANTUML_SETTINGS := $(GENERATION_DIR)/plantuml_settings.iuml
REVISION_TABLE := $(GENERATION_DIR)/revision-table.txt
GENERATION_DIR_PROCESS := source/process/generated
REVISION_TABLE_PROCESS := $(GENERATION_DIR_PROCESS)/revision-table-process.txt
COMPONENTS_DIR := source/components
LIBRARIES_DIR := source/libraries
GIT_DIR = ../.git

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

LIBRARIES = $(INCLUDE_FULL_LIBS) $(INCLUDE_LINK_LIBS)

COMMON_COMPONENT_DIRS = $(addprefix $(COMMON_COMPONENT_ROOT)/, $(COMMON_COMPONENTS))
CUSTOM_COMPONENT_DIRS = $(addprefix $(CUSTOM_COMPONENT_ROOT)/, $(CUSTOM_COMPONENTS))

COMPONENT_DIRS = $(COMMON_COMPONENT_DIRS)
COMPONENT_DIRS += $(CUSTOM_COMPONENT_DIRS)
LIBRARY_DIRS = $(addprefix $(LIBRARY_ROOT)/, $(LIBRARIES))

COMMON_COMPONENT_RFILES_PATT = $(addsuffix /doc/*.rst, $(COMMON_COMPONENT_DIRS))
COMMON_COMPONENT_RFILES_PATT += $(addsuffix /doc/*.txt, $(COMMON_COMPONENT_DIRS))
COMMON_COMPONENT_RFILES_PATT += $(addsuffix /doc/generated/*.txt, $(COMMON_COMPONENT_DIRS))
COMMON_COMPONENT_RFILES = $(wildcard $(COMMON_COMPONENT_RFILES_PATT))

CUSTOM_COMPONENT_RFILES_PATT = $(addsuffix /doc/*.rst, $(CUSTOM_COMPONENT_DIRS))
CUSTOM_COMPONENT_RFILES_PATT += $(addsuffix /doc/*.txt, $(CUSTOM_COMPONENT_DIRS))
CUSTOM_COMPONENT_RFILES_PATT += $(addsuffix /doc/generated/*.txt, $(CUSTOM_COMPONENT_DIRS))
CUSTOM_COMPONENT_RFILES = $(wildcard $(CUSTOM_COMPONENT_RFILES_PATT))

COMPONENTS_RFILES = $(CUSTOM_COMPONENT_RFILES) $(COMMON_COMPONENT_RFILES)
COMPONENTS_RFILES_WO_PATH = $(notdir $(COMPONENTS_RFILES))

UTEST_RFILES_FOLDER = ../build/unit_test/$(PRODUCT)/utest_summary
COMPONENT_UTRFILES_PATT = $(patsubst %,$(UTEST_RFILES_FOLDER)/%_utest*.rst,$(COMMON_COMPONENTS) $(CUSTOM_COMPONENTS))
UTEST_RFILES = $(wildcard $(COMPONENT_UTRFILES_PATT))
UTEST_RFILES_WO_PATH = $(notdir $(UTEST_RFILES))

LIBRARY_RFILES_PATT = $(addsuffix /doc/*manual.rst, $(LIBRARY_DIRS))
LIBRARIES_RFILES = $(wildcard $(LIBRARY_RFILES_PATT))

RST_FILES = $(foreach dir, ./source ,$(wildcard $(dir)/*.rst))
RST_FILES += $(COMPONENTS_RFILES)
RST_FILES += $(UTEST_RFILES)
RST_FILES_FILTERED = $(filter-out ./source/index.rst,$(RST_FILES))
RST_FILES_FILTERED := $(filter-out %_report.rst,$(RST_FILES_FILTERED))
RST_FILES_FILTERED_SORTED := $(sort $(RST_FILES_FILTERED))

# ==================================================
# Original sphinx Make instructions.
# ==================================================

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = sphinx-build
PAPER         =

# Added target to generate a git pre-commit hook file.
# This file is copied to .git/hooks folder and will
# be called upon commit to enforce that
# only one .rst file willi be part of the commit.
#
# One file per commit is required because git notes can
# only be added to a commit id.  If each .rst file needs
# its own review status and git note, we need to
# make sure each file has its own commit id.
ifeq ($(CI),1)
git-hook:
	@$(ECHO) "** No git hook in case of CI"
else
git-hook:
	$(HIDE_CMD)rm -f $(GIT_DIR)/hooks/pre-commit
	$(HIDE_CMD)cd $(GIT_DIR)/hooks/ && $(SOFTLINK) ../../tools/hooks/pre-commit pre-commit
	$(HIDE_CMD)chmod a+x $(GIT_DIR)/hooks/*
endif

# User-friendly check for sphinx-build
ifeq ($(shell which $(SPHINXBUILD) >/dev/null 2>&1; $(ECHO) $$?), 1)
$(error The '$(SPHINXBUILD)' command was not found. Make sure you have Sphinx installed, then set the SPHINXBUILD environment variable to point to the full path of the '$(SPHINXBUILD)' executable. Alternatively you can add the directory with the executable to your PATH. If you dont have Sphinx installed, grab it from http://sphinx-doc.org/)
endif

# Internal variables.
PAPEROPT_a4     = -D latex_paper_size=a4
PAPEROPT_letter = -D latex_paper_size=letter
ALLSPHINXOPTS   = -E -d $(BUILDDIR)/doctrees $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) source/
# the i18n builder cannot share the environment and doctrees with the others
I18NSPHINXOPTS  = $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) source/
ifneq ($(DEBUG), 3)
LATEXMKOPTS		= "-silent"
LATEXOPTS		= "-interaction=batchmode"
endif

.PHONY: help clean html dirhtml singlehtml pickle json htmlhelp qthelp devhelp epub latex latexpdf text man changes linkcheck doctest gettext

help:
	@$(ECHO) "Please use \`make <target>' where <target> is one of"
	@$(ECHO) "  html       to make standalone HTML files"
	@$(ECHO) "  singlehtml to make a single large HTML file"
	@$(ECHO) "  latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter"
	@$(ECHO) "  latexpdf   to make LaTeX files and run them through pdflatex"

# ==================================================
# Additional Melexis targets
# ==================================================

# Stage all files in the build folder.
# Note that the order of targets in the staging is important.
generate: list_components $(REVISION_TABLE) $(COMPONENT_DIRS) $(LIBRARY_DIRS) $(REVISION_TABLE_PROCESS) $(SPHINX_SETTINGS) $(PLANTUML_SETTINGS)

ifeq ($(RELEASE),1)
generate: copy_utest_reports copy_bugfinder_result copy_codeprover_result copy_resource_report
html: copy_utest_coverage
singlehtml: copy_utest_coverage
endif

ifeq ($(CI),1)
generate: copy_utest_reports copy_resource_report
html: copy_utest_coverage
singlehtml: copy_utest_coverage
endif

.PHONY: debug
debug:
	@$(ECHO) $(RST_FILES_FILTERED_SORTED)

# On sphinx build, we dynamically create a revision table from the GIT metadate.
# This is done to avoid edit of the file, what would invalidate the release itself
.PHONY: $(REVISION_TABLE)
$(REVISION_TABLE): $(GENERATION_DIR)
	$(HIDE_CMD)rm -f $(REVISION_TABLE)
	@$(ECHO) "** Fetching review notes for documentation."
	$(HIDE_CMD)git fetch origin refs/notes/*:refs/notes/* $(NO_OUTPUT)
	$(HIDE_CMD)git submodule foreach --recursive git fetch origin refs/notes/*:refs/notes/* $(NO_OUTPUT)
	@$(ECHO) "** Creating revision table that goes in project index."
	@$(ECHO) ".. list-table:: Document revision table" >> $(REVISION_TABLE)
	@$(ECHO) "    :header-rows: 1" >> $(REVISION_TABLE)
	@$(ECHO) "    :widths: 10, 15, 20, 55" >> $(REVISION_TABLE)
	@$(ECHO) "" >> $(REVISION_TABLE)
	@$(ECHO) "    * - Commit" >> $(REVISION_TABLE)
	@$(ECHO) "      - Date" >> $(REVISION_TABLE)
	@$(ECHO) "      - File" >> $(REVISION_TABLE)
	@$(ECHO) "      - Notes" >> $(REVISION_TABLE)
	$(HIDE_CMD)for var in $(RST_FILES_FILTERED_SORTED); do \
		rst_file_path=$$(dirname "$$var"); \
		rst_file_name=$$(basename "$$var"); \
		cd $$rst_file_path; \
		commit=`git log --pretty=format:'%h%n' --abbrev-commit -n 1 $$rst_file_name`; \
		date=`git log --pretty=format:'%ad%n' --date=short -n 1 $$rst_file_name`; \
		notes=`git log --show-notes=review --pretty=format:'%N%n' -n 1 $$rst_file_name`; \
		cd - $(NO_OUTPUT); \
		printf '    * - %s\n' "$${commit##*/}" >> $(REVISION_TABLE); \
		printf '      - %s\n' "$${date##*/}" >> $(REVISION_TABLE); \
		printf '      - %s\n' "$${rst_file_name##*/}" >> $(REVISION_TABLE); \
		printf '      - %s\n' "$${notes##*/}" >> $(REVISION_TABLE); \
	done

$(BUILDDIR):
	$(HIDE_CMD)$(MKDIR) $@

$(BUILDDIR)/html/doxygen: $(BUILDDIR)
	$(HIDE_CMD)$(MKDIR) $@

# Components directory is phony: we need to recreate this folder on every build as the set of components differs with the project being build
.PHONY: $(COMPONENTS_DIR) $(GENERATION_DIR)
$(COMPONENTS_DIR):
	$(HIDE_CMD)$(RM) $@
	$(HIDE_CMD)$(MKDIR) $@

$(COMPONENTS_DIR)/images: $(COMPONENTS_DIR)
	$(HIDE_CMD)$(MKDIR) $@

# Library directory is phony: we need to recreate this folder on every build as the set of libraries differs with the project/layer being build
.PHONY: $(LIBRARIES_DIR)
$(LIBRARIES_DIR):
	$(HIDE_CMD)$(RM) $@
	$(HIDE_CMD)$(MKDIR) $@

$(GENERATION_DIR):
	$(HIDE_CMD)$(RM) $@
	$(HIDE_CMD)$(MKDIR) $@

$(GENERATION_DIR)/images: $(GENERATION_DIR)
	$(HIDE_CMD)$(MKDIR) $@

$(GENERATION_DIR_PROCESS):
	$(HIDE_CMD)$(MKDIR) $@

# Component documentation is outside this project, we need to link it in
# Target is phony: we need to recreate this on every build as the set of components differs with the project being build
.PHONY: $(COMPONENT_DIRS)
$(COMPONENT_DIRS): $(COMPONENTS_DIR)
	$(HIDE_CMD)if [ -f "$@/doc/Makefile" ]; then $(MAKE) -C $@/doc PRODUCT=$(PRODUCT) DEBUG=$(DEBUG) RELEASE=$(RELEASE); fi
ifneq (,$(findstring cp,$(SOFTLINK)))
	$(HIDE_CMD)$(MKDIR) $(COMPONENTS_DIR)/$(notdir $@)
	$(HIDE_CMD)$(SOFTLINK) $@/doc/* $(COMPONENTS_DIR)/$(notdir $@)
else
	$(HIDE_CMD)$(SOFTLINK) ../../$@/doc $(COMPONENTS_DIR)/$(notdir $@)

endif

# Library documentation is outside this project, we need to link it in
# Target is phony: we need to recreate this on every build as the set of libraries differs with the project/layer being build
.PHONY: $(LIBRARY_DIRS)
$(LIBRARY_DIRS): $(LIBRARIES_DIR)
ifneq (,$(findstring cp,$(SOFTLINK)))
	$(HIDE_CMD)$(SOFTLINK) ../../$(LIBRARIES_RFILES) $(LIBRARIES_DIR)
else
	$(HIDE_CMD)$(SOFTLINK) ../../$(LIBRARIES_RFILES) $(LIBRARIES_DIR)
endif

# Component utest report is outside this project, we need to copy it over
.PHONY: copy_utest_reports
copy_utest_reports: $(COMPONENTS_DIR)
	@$(ECHO) "** Pulling in utest restructured text reports, copy to build folder."
	$(HIDE_CMD)$(ECHO) -n $(UTEST_RFILES) | xargs -d ' ' -n 1 -I {} $(SOFTLINK) ../../{} $(COMPONENTS_DIR)

# Component utest coverage is outside this project, we need to copy it over
.PHONY: copy_utest_coverage
copy_utest_coverage: $(COMPONENTS_DIR)
	@$(ECHO) "** Pulling in utest coverage reports, copy to build folder."
	$(HIDE_CMD)cp -R ../build/unit_test/$(PRODUCT)/coverage_html $(BUILDDIR)/html/utest_coverage

# Resource report is in build folder, we need to copy it over
.PHONY: copy_resource_report
copy_resource_report:
	@$(ECHO) "** Pulling in resource report, copy to build folder."
	$(HIDE_CMD)rm -f $(GENERATION_DIR)/resource_report.rst
	$(HIDE_CMD)$(SOFTLINK) ../../../build/resource_report/$(PRODUCT)/resource_usage.rst $(GENERATION_DIR)/report-resource-usage.rst
	$(HIDE_CMD)$(COPY) ../build/resource_report/$(PRODUCT)/*.png $(GENERATION_DIR)/

# Bugfinder result is in build folder, we need to copy it over
.PHONY: copy_bugfinder_result
copy_bugfinder_result:
	@$(ECHO) "** Pulling in BugFinder result, copy to build folder."
	$(HIDE_CMD)rm -f source/generated/bugfinder.pdf
	$(HIDE_CMD)$(SOFTLINK) ../../../build/bugfinder/$(PRODUCT)/Polyspace-Doc/MLX$(PRODUCT)_BugFinder.pdf source/generated/bugfinder.pdf

# Codeprover result is in build folder, we need to copy it over
.PHONY: copy_codeprover_result
copy_codeprover_result:
	@$(ECHO) "** Pulling in CodeProver result, copy to build folder."
	$(HIDE_CMD)rm -f source/generated/codeprover.pdf
	$(HIDE_CMD)$(SOFTLINK) ../../../build/codeprover/$(PRODUCT)/Polyspace-Doc/MLX$(PRODUCT)_Developer.pdf source/generated/codeprover.pdf

# Pull in a specific version of the process documentation through git submodule.
# The version needs to reflect the exact same SW development process that was used while
# creating this project.
#
# In the process repository (which is a git submodule), these few make lines do the following:
# 0) Change all links in process to project template to point to this project.
# 1) Create the process revsion history rst section that is displayed in the process index page
# 2) Rename the process index.rst to index-process.rst to avoid conflict with project index.rst
# 3) Flatten the rst files folder structure to have all rst files in the same, next to the project (to satisfy the link to the images)
#
# Some additional notes:
# Instead of git submodule, git archive could have been used to get the process rst files.
# Though, to be able to get the tag annotation data for pulling it dynamically into process revision-table.txt, a true git submodule is needed.
#
# On sphinx build, we dynamically create a revision table from the GIT metadate.
# This is done to avoid edit of the file, what would invalidate the release itself
# A stable, reveiwed and accepted process documentation shall be used when linked into this
# project documentation.

PROCESS_CID = NONE
PROCESS_CID_SHORT = NONE
TAG=NONE

.PHONY: $(REVISION_TABLE_PROCESS)
$(REVISION_TABLE_PROCESS): $(GENERATION_DIR_PROCESS)
	@$(ECHO) "** Generating process revision table from tag information."
	@$(ECHO) "" > $(REVISION_TABLE_PROCESS)
	@$(ECHO) "=============== ===========================" >> $(REVISION_TABLE_PROCESS)
	@$(ECHO) "Release ID      Details" >> $(REVISION_TABLE_PROCESS)
	@$(ECHO) "=============== ===========================" >> $(REVISION_TABLE_PROCESS)
ifneq ($(TAG), )
	@$(ECHO) "$(TAG)" >> $(REVISION_TABLE_PROCESS)
else
	@$(ECHO) "$(PROCESS_CID_SHORT)         using a non-tagged ver." >> $(REVISION_TABLE_PROCESS)
endif
	@$(ECHO) "=============== ===========================" >> $(REVISION_TABLE_PROCESS)
	@$(ECHO) "" >> $(REVISION_TABLE_PROCESS)

TARGET_VAR="HTML_BUILD"
latex latexpdf: TARGET_VAR="PDF_BUILD"

# Flags passed to RST (..only) and plantuml (ifdef) preprocessor
DOC_FLAGS  = $(COMPONENT_CPPFLAGS)
DOC_FLAGS += $(COMMON_COMPONENTS)
DOC_FLAGS += $(CUSTOM_COMPONENTS)
DOC_FLAGS += $(INCLUDE_FULL_LIBS)
DOC_FLAGS += $(INCLUDE_LINK_LIBS)

$(SPHINX_SETTINGS): $(GENERATION_DIR)
	$(HIDE_CMD)$(TOUCH) $@
	@$(ECHO) "def add_project_tags(tags):" > $@
	@$(ECHO) "    tags.add('PROD$(PRODUCT)')" >> $@
	$(HIDE_CMD)$(foreach flag, $(DOC_FLAGS), \
		$(ECHO) "    tags.add('$(flag)')" >> $@ && \
	) true
ifeq ($(HIDE_CONFIDENTIAL_INFO),1)
	@$(ECHO) "    tags.add('HIDE_CONFIDENTIAL_INFO')" >> $@
endif
ifeq ($(RELEASE),1)
	@$(ECHO) "    tags.add('RELEASE')" >> $@
endif
ifeq ($(CI),1)
	@$(ECHO) "    tags.add('CI')" >> $@
endif
	@$(ECHO) "    tags.add('$(TARGET_VAR)')" >> $@
	@$(ECHO) "" >> $@

$(PLANTUML_SETTINGS): $(GENERATION_DIR)
	$(HIDE_CMD)$(TOUCH) $@
	@$(ECHO) "!define PROD$(PRODUCT)" >> $@
	$(HIDE_CMD)$(foreach flag, $(DOC_FLAGS), \
		$(ECHO) "!define $(subst =, ,$(flag))" >> $@ && \
	) true
ifeq ($(RELEASE),1)
	@$(ECHO) "!define RELEASE" >> $@
endif
ifeq ($(CI),1)
	@$(ECHO) "!define CI" >> $@
endif
	@$(ECHO) "!define $(TARGET_VAR)" >> $@
	@$(ECHO) "" >> $@

# Doxygen documentation that is stored in the source files is referenced from the
# rest documentation, we need to make sure we publish it at the same time
DOXYGEN_INPUT = $(addsuffix /src, $(COMMON_COMPONENT_DIRS) $(CUSTOM_COMPONENT_DIRS)) $(PRODUCT_SRCDIR)
# also parse any inc folders from submoduled libraries
DOXYGEN_INPUT += $(wildcard $(addsuffix /inc/., $(COMMON_COMPONENT_DIRS) $(CUSTOM_COMPONENT_DIRS)) $(PRODUCT_SRCDIR))
DOXYGEN_INPUT += $(addsuffix /src, $(LIBRARY_DIRS))
DOXYGEN_INPUT += $(addsuffix /inc, $(LIBRARY_DIRS))
.PHONY: doxy
doxy: $(BUILDDIR)/html/doxygen
	@$(ECHO) "** Running doxygen on source code, destination of doxygen build is the final html folder."
	$(HIDE_CMD) DOXYGEN_INPUT="$(DOXYGEN_INPUT)" COMPONENT_FLAGS="$(COMPONENT_CPPFLAGS)" doxygen doxygen/doxyfile $(NO_OUTPUT)

.PHONY: clean
clean:
	@$(ECHO) "Cleaning..."
	$(HIDE_CMD)rm -rf $(BUILDDIR)/html
	$(HIDE_CMD)rm -rf $(BUILDDIR)/latex
	$(HIDE_CMD)rm -rf $(BUILDDIR)/doctrees
	$(HIDE_CMD)rm -rf $(GENERATION_DIR)
	$(HIDE_CMD)rm -rf $(GENERATION_DIR_PROCESS)
	$(HIDE_CMD)rm -rf $(COMPONENTS_DIR)
	$(HIDE_CMD)rm -rf $(LIBRARIES_DIR)

#
# Targets for documentation.
#

ifeq ($(RELEASE),1)
# For release build, take version string from product makefile
DOC_VERSION=MLX$(PRODUCT)_$(RELEASE_MATURITY)_$(RELEASE_MAJOR)_$(RELEASE_MINOR)_$(RELEASE_REVISION)
TARGETNAME=SW$(PRODUCT)_$(RELEASE_MATURITY)_$(RELEASE_MAJOR)_$(RELEASE_MINOR)_$(RELEASE_REVISION)
else
# ... otherwise, use the git describe output (tag-nb_of_commits-hash) as identification
DOC_VERSION=$(shell git describe --always --long --abbrev=8 --dirty --tags)
TARGETNAME=fw_$(PRODUCT)
endif

.PHONY: doxywarnings
doxywarnings: $(BUILDDIR)/html/doxygen
	@$(ECHO) "** Running doxygen on source code to find undocumented members"
	$(HIDE_CMD) doxygen doxygen/doxyfile_warnings 2>&1 $(NO_OUTPUT)

html: git-hook generate doxy
	@$(ECHO) "** Building RST documentation using Sphinx."
	$(HIDE_CMD)PRODUCT=$(PRODUCT) VERSION=$(DOC_VERSION) $(SPHINXBUILD) -b html $(ALLSPHINXOPTS) $(BUILDDIR)/html $(NO_OUTPUT)
	@$(ECHO)
	@$(ECHO) Build finished.

singlehtml: git-hook generate doxy
	$(HIDE_CMD)PRODUCT=$(PRODUCT) VERSION=$(DOC_VERSION) $(SPHINXBUILD) -b singlehtml $(ALLSPHINXOPTS) $(BUILDDIR)/singlehtml $(NO_OUTPUT)
	@$(ECHO)
	@$(ECHO) Build finished.

latex: git-hook generate doxy
	$(HIDE_CMD)PRODUCT=$(PRODUCT) VERSION=$(DOC_VERSION) $(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex $(NO_OUTPUT)
	@$(ECHO)
	@$(ECHO) Build finished.
	@$(ECHO) "Run \`make' in that directory to run these through (pdf)latex" \
	      "(use \`make latexpdf' here to do that automatically)."

latexpdf: git-hook generate doxy
	$(HIDE_CMD)PRODUCT=$(PRODUCT) VERSION=$(DOC_VERSION) $(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex $(NO_OUTPUT)
	@$(ECHO) "Running LaTeX files through pdflatex..."
	$(MAKE) -C $(BUILDDIR)/latex LATEXOPTS=$(LATEXOPTS) LATEXMKOPTS=$(LATEXMKOPTS) all-pdf
	@$(ECHO) "pdflatex finished; the PDF files are in $(BUILDDIR)/latex."

# ==========================
# Some helper output targets
# ==========================

.PHONY: list_components
list_components:
	@echo ""
	@$(COLOR_GREEN) echo "Common components:"; $(COLOR_RESET) echo "    $(COMMON_COMPONENTS)"
	@$(COLOR_YELLOW) echo "Custom components:"; $(COLOR_RESET) echo "    $(CUSTOM_COMPONENTS)"
	@$(COLOR_BLUE) echo "Full libraries:"; $(COLOR_RESET) echo "    $(INCLUDE_FULL_LIBS)"
	@$(COLOR_VIOLET) echo "Link libraries:"; $(COLOR_RESET) echo "    $(INCLUDE_LINK_LIBS)"

