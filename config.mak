#
# PLATFORM=win32 for Windows
# PLATFORM=linux for Linux
#
# DESTPLATFORM=win32 for crosscompilation etc.
#
# LINUXVARIANT=gcc for gcc
# LINUXVARIANT=icc for Intel's C++ compiler
#
# WIN32VARIANT=mingw for MinGW, or crosscompiling Linux -> Win32
# (don't use WIN32VARIANT=cygwin with PLATFORM=linux)
# WIN32VARIANT=cygwin for Cygwin (ie. for AIDC use primarily)
#
PLATFORM=linux
#DESTPLATFORM=linux
DESTPLATFORM=win32
LINUXVARIANT=gcc
WIN32VARIANT=mingw
