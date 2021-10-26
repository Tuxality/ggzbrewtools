VERSION_VER  = 0.1
VERSION_DATE = built on `date`

UNPACK_TARGET  = unpack
UNPACK_OBJECTS = unpack.o

PACK_TARGET  = pack
PACK_OBJECTS = pack.o

CXX        = g++
STRIP      = strip
CXXFLAGS   = -std=c++17 -O3
PRELDFLAGS =
LDFLAGS    = -lboost_iostreams -lz

ifeq ($(OS),Windows_NT)
	UNPACK_TARGET_NAME = $(UNPACK_TARGET).exe
	PACK_TARGET_NAME   = $(PACK_TARGET).exe
	PRELDFLAGS += -static -static-libgcc -static-libstdc++

	ifeq ($(CROSS),32)
		CXX   = i686-w64-mingw32-g++
		STRIP = i686-w64-mingw32-strip
	else ifeq ($(CROSS),64)
		CXX   = x86_64-w64-mingw32-g++
		STRIP = x86_64-w64-mingw32-strip
	else
		LDFLAGS = -lboost_iostreams-mt -lz
	endif
else
	UNPACK_TARGET_NAME = $(UNPACK_TARGET)
	PACK_TARGET_NAME   = $(PACK_TARGET)

	ifeq ($(UNAME_S),Darwin)
		CXXFLAGS   += -I/usr/local/opt/boost/include
		PRELDFLAGS += -L/usr/local/opt/boost/lib
	endif
endif

.PHONY = all unpack_make pack_make clean unpack_clean pack_clean run

all : unpack_make pack_make

unpack_make: $(UNPACK_TARGET_NAME)

pack_make: $(PACK_TARGET_NAME)

version.h: Makefile
	@printf "#define __VERSION_VER__  \"%s\"\r\n" "$(VERSION_VER)" > version.h
	@printf "#define __VERSION_DATE__ \"%s\"\r\n" "$(VERSION_DATE)" >> version.h

%.o: %.cpp version.h
	@echo Compiling $<...
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(UNPACK_TARGET_NAME) : $(UNPACK_OBJECTS)
$(PACK_TARGET_NAME)   : $(PACK_OBJECTS)

$(UNPACK_TARGET_NAME) $(PACK_TARGET_NAME) :
	@echo Linking $@...
	@$(CXX) $(CXXFLAGS) $(PRELDFLAGS) $^ -o $@ $(LDFLAGS)
	@echo Stripping $@...
	@$(STRIP) $@

clean: unpack_clean pack_clean

unpack_clean:
	@echo Cleaning unpack...
	@rm -f $(UNPACK_OBJECTS) $(UNPACK_TARGET_NAME) $(UNPACK_TARGET_NAME).exe

pack_clean:
	@echo Cleaning pack...
	@rm -f $(PACK_OBJECTS) $(PACK_TARGET_NAME) $(PACK_TARGET_NAME).exe	

run : $(UNPACK_TARGET_NAME) $(PACK_TARGET_NAME)
	@./$(UNPACK_TARGET_NAME)
	@./$(PACK_TARGET_NAME)
