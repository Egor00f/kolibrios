LIBRARY = freetype

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
STRIP = kos32-strip

INSTALLDIR:= C:/MinGW/msys/1.0/home/autobuild/tools/win32/lib

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -Wall -Winline -fno-ident -fomit-frame-pointer

LDFLAGS = -shared -s -T dll.lds --entry _DllStartup --image-base=0
LDFLAGS+= --out-implib lib$(LIBRARY).dll.a

ARFLAGS = crs

INCLUDES= -I../newlib/libc/include -I./include -I./include/freetype/config 

LIBS:=  -ldll -lgcc -lc.dll

DEFINES:= -DFT_CONFIG_CONFIG_H="<ftconfig.h>" -DFT_CONFIG_MODULES_H="<ftmodule.h>" -DFT2_BUILD_LIBRARY

SOURCES =				\
	src/autofit/autofit.c		\
	src/base/ftbase.c		\
	src/base/ftbbox.c		\
	src/base/ftbdf.c		\
	src/base/ftbitmap.c		\
	src/base/ftcid.c		\
	src/base/ftdebug.c		\
	src/base/ftfstype.c		\
	src/base/ftgasp.c		\
	src/base/ftglyph.c		\
	src/base/ftgxval.c		\
	src/base/ftinit.c		\
	src/base/ftlcdfil.c		\
	src/base/ftmm.c			\
	src/base/ftotval.c		\
	src/base/ftpatent.c		\
	src/base/ftpfr.c		\
	src/base/ftstroke.c		\
	src/base/ftsynth.c		\
	src/base/ftsystem.c		\
	src/base/fttype1.c		\
	src/bdf/bdf.c			\
	src/bzip2/ftbzip2.c		\
	src/cache/ftcache.c		\
	src/cff/cff.c			\
	src/cid/type1cid.c		\
	src/gzip/ftgzip.c		\
	src/lzw/ftlzw.c			\
	src/otvalid/otvalid.c		\
	src/pcf/pcf.c			\
	src/pfr/pfr.c			\
	src/psaux/psaux.c		\
	src/pshinter/pshinter.c		\
	src/psnames/psnames.c		\
	src/raster/raster.c		\
	src/sfnt/sfnt.c			\
	src/smooth/smooth.c		\
	src/truetype/truetype.c		\
	src/type1/type1.c		\
	src/type42/type42.c		\
	src/winfonts/winfnt.c
 
OBJECTS     = $(patsubst %.c, %.o, $(SOURCES))

ifeq ($(findstring ebox,$(MAKECMDGOALS)),ebox)
CFLAGS+=-march=pentium-mmx
endif

# targets 

all:lib$(LIBRARY).a $(LIBRARY).dll

ebox:lib$(LIBRARY).a $(LIBRARY).dll

lib$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) cvrs lib$(LIBRARY).a $(OBJECTS)
	mv -f lib$(LIBRARY).a $(INSTALLDIR)

$(LIBRARY).dll: lib$(LIBRARY).def $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $@ lib$(LIBRARY).def $(OBJECTS) $(LIBS)
	$(STRIP) $@
	#sed -f ../newlib/cmd1.sed lib$(LIBRARY).def > mem
	#sed -f ../newlib/cmd2.sed mem >$(LIBRARY).inc
	mv -f $@ ../../bin
	mv -f lib$(LIBRARY).dll.a $(INSTALLDIR)
	

%.o : %.c Makefile
	$(CC) $(INCLUDES) $(CFLAGS) $(DEFINES)  -o $@ $<


clean: 
	-rm -f *.o 
 
