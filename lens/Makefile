# Defaults flags:
CC        = cc
CFLAGS    = -Wall -O4
MACHINE   = LINUX
#CC        = icc
#CFLAGS    = -w1 -O3 -march=pentiumiii -mcpu=pentiumpro -DAVOID_NAN_TEST
#MACHINE   = INTEL
MAKE      = /usr/bin/make
SYSLIB    = -export-dynamic -ldl -L/usr/X11R6/lib

# Set default HOSTTYPE
ifndef HOSTTYPE
  ifeq ($(OS),Windows_NT)
    HOSTTYPE   = i686
  else
    HOSTTYPE   = Linux
  endif
endif

# Windows 95/98/NT using Cygwin:
ifeq ($(HOSTTYPE),i686)
  MACHINE   = WINDOWS
  CFLAGS    = -Wall -O4
  MAKE      = make
  EXT       = .exe
  SYSLIB    =
endif
# Macintosh X:
ifeq ($(HOSTTYPE),macintosh)
  MACHINE   = MACINTOSH
  MAKE      = make
  CC        = cc
  CFLAGS    = -pipe -O3 -Wall -fno-common -arch ppc
  EXT       = 
  SYSLIB    =
endif
# Older Linux machines:
ifeq ($(HOSTTYPE),i386_linux2)
  SYSLIB    = -export-dynamic -ldl
endif
# New condor
ifeq ($(HOSTTYPE),i386)
  MACHINE   = CONDOR
endif
# Eagle:
ifeq ($(HOSTTYPE),hp9000s800)
  MACHINE   = EAGLE
  CC        = cc
  CFLAGS    = -Ae +Oall -w
  MAKE      = /usr/local/bin/make
  SYSLIB    = /usr/lib/X11R5/libX11.sl -ldld
endif
# SP2:
ifeq ($(HOSTTYPE),rs6000)
  MACHINE   = SP
  CFLAGS    = -O4 -I/usr/include
  MAKE      = /usr/local/bin/make
  SYSLIB    = -lld
endif
# Alpha 21264s
ifeq ($(HOSTTYPE),alpha)
  MACHINE   = ALPHA
  CC        = cc
  CFLAGS    = -arch ev6 -fast -O4
  MAKE      = /usr/home/dr/bin/make
  SYSLIB    = -non_shared -ldnet_stub
endif

DEF= CC="$(CC)" CFLAGS="$(CFLAGS)" MACHINE=$(MACHINE) SYSLIB="$(SYSLIB)" \
	TOP="$(shell pwd)" EXT=$(EXT)

DEF_NO_TK= CC="$(CC)" CFLAGS="-DNO_TK $(CFLAGS)" NO_TK=no_tk MACHINE=$(MACHINE) SYSLIB="$(SYSLIB)" \
	TOP="$(shell pwd)" EXT=$(EXT)

MAKE_LENS  = $(MAKE) -C Src lens$(EXT) $(DEF) EXEC=lens PKGS=
MAKE_ALENS = $(MAKE) -C Src alens$(EXT) $(DEF) EXEC=alens PKGS="-DADVANCED"
MAKE_LIBLENS  = $(MAKE) -C Src liblens $(DEF) EXEC=lens PKGS=
MAKE_LIBLENS_NO_TK  = $(MAKE) -C Src liblens $(DEF_NO_TK) EXEC=lens PKGS=
MAKE_LIBALENS = $(MAKE) -C Src libalens $(DEF) EXEC=alens PKGS="-DADVANCED"
MAKE_LIBALENS_NO_TK = $(MAKE) -C Src libalens $(DEF_NO_TK) EXEC=alens PKGS="-DADVANCED"
MAKE_CLIENTS  = $(MAKE) -C Clients all
MAKE_CLEAN = $(MAKE) -C Src $(DEF) clean

all:: dirs
	$(MAKE_CLEAN)
	$(MAKE_LIBLENS_NO_TK)
clients:: dirs
	$(MAKE_CLEAN)
	$(MAKE_CLIENTS)
lens:: dirs
	$(MAKE_CLEAN)
	$(MAKE_LENS)
liblens:: dirs
	$(MAKE_CLEAN)
	$(MAKE_LIBLENS)
liblens_no_tk:: dirs
	$(MAKE_CLEAN)
	$(MAKE_LIBLENS_NO_TK)
alens:: dirs
	$(MAKE_CLEAN)
	$(MAKE_ALENS)
libalens:: dirs
	$(MAKE_CLEAN)
	$(MAKE_LIBALENS)
libalens_no_tk:: dirs
	$(MAKE_CLEAN)
	$(MAKE_LIBALENS_NO_TK)
clean::
	$(MAKE_CLEAN)

# This builds the necessary system-dependent subdirectories on unix machines
ifeq ($(MACHINE),WINDOWS)
dirs::
else
dirs:: Obj/$(HOSTTYPE) Bin/$(HOSTTYPE)
Obj/$(HOSTTYPE)::
	if test ! -d Obj/$(HOSTTYPE); \
	then mkdir Obj/$(HOSTTYPE); fi
Bin/$(HOSTTYPE)::
	if test ! -d Bin/$(HOSTTYPE); \
	then mkdir Bin/$(HOSTTYPE); fi
endif
