# Makefile for customizing build for 75322 (PC-based compilation)

# EXCLUDE_COMPONENTS / INCLUDE_COMPONENTS
# List of components (common and custom) that need to be ignored or added in this product build.
# Note that these 2 options should not be used together.
EXCLUDE_COMPONENTS =
INCLUDE_COMPONENTS = driver hal_api cont_mode_lib
INCLUDE_COMPONENTS += driver_docs

# COMPONENT_CPPFLAGS
# List of preprocessor directives (passed as -D options when compiling) for this product.

#Debug flags
#DEBUG_FLAGS = TRACE_DEBUG API_DEBUG CONT_MODE_DEBUG SYNC_MODE_DEBUG SYNC_TEST_FLOW
COMPONENT_FLAGS =
COMPONENT_FLAGS += $(addsuffix '=1', $(DEBUG_FLAGS))
# List of linker full libraries that will be used during the link-time (with option `-l`)
COMPONENT_LIBS = pthread
