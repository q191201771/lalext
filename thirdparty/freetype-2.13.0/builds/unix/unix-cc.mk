#
# FreeType 2 template for Unix-specific compiler definitions
#

# Copyright (C) 1996-2023 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


CC           := gcc
COMPILER_SEP := $(SEP)
FT_LIBTOOL_DIR ?= $(PLATFORM_DIR)

LIBTOOL := $(FT_LIBTOOL_DIR)/libtool


# The object file extension (for standard and static libraries).  This can be
# .o, .tco, .obj, etc., depending on the platform.
#
O  := lo
SO := o


# The executable file extension.  Although most Unix platforms use no
# extension, we copy the extension detected by autoconf.  Useful for cross
# building on Unix systems for non-Unix systems.
#
E := 


# The library file extension (for standard and static libraries).  This can
# be .a, .lib, etc., depending on the platform.
#
A  := la
SA := a


# The name of the final library file.  Note that the DOS-specific Makefile
# uses a shorter (8.3) name.
#
LIBRARY := lib$(PROJECT)


# Path inclusion flag.  Some compilers use a different flag than `-I' to
# specify an additional include path.  Examples are `/i=' or `-J'.
#
I := -I


# C flag used to define a macro before the compilation of a given source
# object.  Usually it is `-D' like in `-DDEBUG'.
#
D := -D


# The link flag used to specify a given library file on link.  Note that
# this is only used to compile the demo programs, not the library itself.
#
L := -l


# Target flag.
#
T := -o$(space)


# C flags
#
#   These should concern: debug output, optimization & warnings.
#
#   Use the ANSIFLAGS variable to define the compiler flags used to enforce
#   ANSI compliance.
#
#   We use our own FreeType configuration files overriding defaults.
#
CPPFLAGS := 
CFLAGS   := -c -Wall -g -O2 -fvisibility=hidden -DDARWIN_NO_CARBON   -I/opt/homebrew/Cellar/libpng/1.6.39/include/libpng16 -I/opt/homebrew/Cellar/harfbuzz/7.3.0/include/harfbuzz -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/Cellar/libpng/1.6.39/include/libpng16 -I/opt/homebrew/Cellar/glib/2.76.3/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.76.3/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.42/include -I/opt/homebrew/Cellar/graphite2/1.3.14/include -I/opt/homebrew/Cellar/brotli/1.0.9/include -pthread \
            $DFT_CONFIG_CONFIG_H="<ftconfig.h>" \
            $DFT_CONFIG_MODULES_H="<ftmodule.h>" \
            $DFT_CONFIG_OPTIONS_H="<ftoption.h>"

# ANSIFLAGS: Put there the flags used to make your compiler ANSI-compliant.
#
ANSIFLAGS :=  -pedantic -std=c99

# C compiler to use -- we use libtool!
#
# CC might be set on the command line; we store this value in `CCraw'.
# Consequently, we use the `override' directive to ensure that the
# libtool call is always prepended.
#
CCraw       := $(CC)
override CC := $(LIBTOOL) --mode=compile $(CCraw)

# Resource compiler to use on Cygwin/MinGW, usually windres.
#
RCraw := 
ifneq ($(RCraw),)
  RC := $(LIBTOOL) --tag=RC --mode=compile $(RCraw)
endif

# Linker flags.
#
LDFLAGS :=  -lz -lbz2 -L/opt/homebrew/Cellar/libpng/1.6.39/lib -lpng16 -L/opt/homebrew/Cellar/harfbuzz/7.3.0/lib -lharfbuzz -L/opt/homebrew/Cellar/brotli/1.0.9/lib -R/opt/homebrew/Cellar/brotli/1.0.9/lib -lbrotlidec -pthread -lpthread

# export symbols
#
CCraw_build  := gcc	# native CC of building system
E_BUILD      := 	# extension for executable on building system
EXPORTS_LIST := $(OBJ_DIR)/ftexport.sym
CCexe        := $(CCraw_build)	# used to compile `apinames' only


# Library linking.
#
LINK_LIBRARY = $(LIBTOOL) --mode=link $(CCraw) -o $@ $(OBJECTS_LIST) \
                          -rpath $(libdir) -version-info $(version_info) \
                          $(LDFLAGS) -no-undefined \
                          -export-symbols $(EXPORTS_LIST)

# For the demo programs.
FT_DEMO_CFLAGS :=  -I/opt/homebrew/Cellar/librsvg/2.56.1/include/librsvg-2.0 -I/opt/homebrew/Cellar/gdk-pixbuf/2.42.10_1/include/gdk-pixbuf-2.0 -I/opt/homebrew/Cellar/libtiff/4.5.0/include -I/opt/homebrew/Cellar/zstd/1.5.5/include -I/opt/homebrew/Cellar/jpeg-turbo/2.1.5.1/include -I/opt/homebrew/Cellar/glib/2.76.3/include -I/opt/homebrew/Cellar/cairo/1.16.0_5/include/cairo -I/opt/homebrew/Cellar/glib/2.76.3/include -I/opt/homebrew/Cellar/glib/2.76.3/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.76.3/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.42/include -I/opt/homebrew/Cellar/pixman/0.42.2/include/pixman-1 -I/opt/homebrew/Cellar/fontconfig/2.14.2/include -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/Cellar/libpng/1.6.39/include/libpng16 -I/opt/homebrew/Cellar/libxcb/1.15_1/include -I/opt/homebrew/Cellar/libxrender/0.9.11/include -I/opt/homebrew/Cellar/libxext/1.3.5/include -I/opt/homebrew/Cellar/libx11/1.8.4/include -I/opt/homebrew/Cellar/libxcb/1.15_1/include -I/opt/homebrew/Cellar/libxau/1.0.11/include -I/opt/homebrew/Cellar/libxdmcp/1.1.4/include -I/opt/homebrew/Cellar/xorgproto/2022.2/include -I/Library/Developer/CommandLineTools/SDKs/MacOSX12.sdk/usr/include/ffi -DHAVE_LIBRSVG
FT_DEMO_LDFLAGS :=  -L/opt/homebrew/Cellar/librsvg/2.56.1/lib -L/opt/homebrew/Cellar/glib/2.76.3/lib -L/opt/homebrew/Cellar/gdk-pixbuf/2.42.10_1/lib -L/opt/homebrew/Cellar/glib/2.76.3/lib -L/opt/homebrew/opt/gettext/lib -L/opt/homebrew/Cellar/cairo/1.16.0_5/lib -lrsvg-2 -lm -lgio-2.0 -lgdk_pixbuf-2.0 -lgobject-2.0 -lglib-2.0 -lintl -lcairo

# EOF
