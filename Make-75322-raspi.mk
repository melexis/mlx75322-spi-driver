# Makefile for customizing build for 75322 (Raspberry Pi-pased compilation)

# RELEASE
# Name of the release to build in case of versioned_release
RELEASE_MATURITY = RC
RELEASE_MAJOR    = 1
RELEASE_MINOR    = 3
RELEASE_REVISION = 0

# EXCLUDE_COMPONENTS / INCLUDE_COMPONENTS
# List of components (common and custom) that need to be ignored or added in this product build.
# Note that these 2 options should not be used together.
EXCLUDE_COMPONENTS =
INCLUDE_COMPONENTS = driver hal_api hal_raspi cont_mode_lib
INCLUDE_COMPONENTS += udp_callback
INCLUDE_COMPONENTS += driver_docs

# COMPONENT_CPPFLAGS
# List of preprocessor directives (passed as -D options when compiling) for this product.

#Debug flags
#DEBUG_FLAGS = TRACE_DEBUG CONT_MODE_DEBUG API_DEBUG COM_DEBUG_DETAIL_0 COM_DEBUG_DETAIL_1 COM_DEBUG_DETAIL_2 SYNC_MODE_DEBUG SYNC_TEST_FLOW
COMPONENT_FLAGS = RPI
COMPONENT_FLAGS += $(addsuffix '=1', $(DEBUG_FLAGS))
# List of linker full libraries that will be used during the link-time (with option `-l`)
COMPONENT_LIBS = wiringPi pthread crypt m rt
