#
# invoke "make BUILD=DEBUG" or "make BUILD=RELEASE"
# 
# You can override this from the command line,
# e.g. make BUILD=DEBUG
BUILD=DEBUG
#BUILD=RELEASE
#BUILD=STATIC

TWIST_HOME=./twist5

PLATFORMSUF = 	
ifdef WINDOWS
	PLATFORMSUF = _win
endif
ifeq ($(TARGET),CROSSCOMPILE)
	CXX = i686-w64-mingw32-g++
	LD = i686-w64-mingw32-g++
	WINDRES = i686-w64-mingw32-windres
	PLATFORMSUF = _win
else
	CXX = g++
	LD = g++
endif

CFLAGS = -std=c++11 -Iinclude -I$(TWIST_HOME)/include -W -Wall -Wno-unused -DUSE_MOUSE
LIBS = 

WANT_CURL = ON
ifeq ($(WANT_CURL),ON)
	CFLAGS += -DUSE_CURL `pkg-config --cflags libcurl`
	LIBS += `pkg-config --libs libcurl`
endif

NAME = tins22

ifeq ($(BUILD),RELEASE)
	CFLAGS += -O3
	LFLAGS += -s
	BUILDDIR = build/release$(PLATFORMSUF)
endif
ifeq ($(BUILD),DEBUG)
	CFLAGS += -g -DDEBUG
#-DUSE_MONITORING
	BUILDDIR = build/debug$(PLATFORMSUF)
endif
ifeq ($(BUILD),STATIC)
	CFLAGS += -O3
	LFLAGS += -s
	BUILDDIR = build/static$(PLATFORMSUF)
endif

OBJDIR=$(BUILDDIR)/obj

ifdef WINDOWS
	CFLAGS += -D__GTHREAD_HIDE_WIN32API
	LFLAGS += -Wl,--subsystem,windows
	ifeq ($(BUILD),RELEASE)
		LIBS += -lallegro_monolith
	endif
	ifeq ($(BUILD),DEBUG)
		LIBS += -lallegro_monolith-debug
	endif
	BINSUF = .exe
	ICONOBJ = $(OBJDIR)/icon.o	
else
	ALLEGRO_MODULES=allegro allegro_primitives allegro_font allegro_main allegro_dialog allegro_image allegro_audio allegro_acodec allegro_ttf allegro_color
	ifeq ($(BUILD),RELEASE)
		ALLEGRO_LIBS = $(addsuffix -5, $(ALLEGRO_MODULES))
		LIBS += `pkg-config --libs $(ALLEGRO_LIBS)`
	endif
	ifeq ($(BUILD),DEBUG)
		ALLEGRO_LIBS = $(addsuffix -debug-5, $(ALLEGRO_MODULES))
		LIBS += `pkg-config --libs $(ALLEGRO_LIBS)`
	endif
	ifeq ($(BUILD),STATIC)
		LFLAGS += '-Wl,-rpath,$$ORIGIN/../lib',--enable-new-dtags
		# This will only statically link allegro but not its dependencies.
		# See https://www.allegro.cc/forums/thread/616656
		ALLEGRO_LIBS = $(addsuffix -static-5, $(ALLEGRO_MODULES))
		LIBS += `pkg-config --libs --static $(ALLEGRO_LIBS)`
	endif
	BINSUF =
endif

BIN = $(BUILDDIR)/$(NAME)$(BINSUF)

$(shell mkdir -p $(OBJDIR) >/dev/null)

vpath %.cpp $(TWIST_HOME)/src:src:test

SRC = $(wildcard src/*.cpp) $(wildcard $(TWIST_HOME)/src/*.cpp)
OBJ = $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRC)))

.PHONY: all
all: $(BIN)

$(BIN) : updateversion $(OBJ) $(ICONOBJ)
	$(LD) -o $(BIN) $(OBJ) $(ICONOBJ) $(LIBS) $(LFLAGS)
	@echo
	@echo "Build complete. Run $(BIN)"

# automatic dependency generation,
# as recommended by http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJDIR)/$*.Td

# Use temporary dep file & rename to avoid messing up deps when compilation fails.
$(OBJDIR)/%.o : %.cpp $(OBJDIR)/%.d
	$(CXX) $(CFLAGS) $(DEPFLAGS) -o $@ -c $<
	@mv -f $(OBJDIR)/$*.Td $(OBJDIR)/$*.d

$(ICONOBJ) : icon.rc icon.ico
	$(WINDRES) -I rc -O coff -i icon.rc -o $(ICONOBJ)

$(OBJDIR)/%.d: ;
.PRECIOUS: $(OBJDIR)/%.d

-include $(OBJDIR)/*.d

CFLAGS_TEST = $(CFLAGS) `pkg-config cppunit --cflags`
LDFLAGS_TEST = `pkg-config cppunit --libs`
TESTBIN = $(BUILDDIR)/test_runner$(BINSUF)

SRC_TEST = $(wildcard test/*.cpp)
OBJ_TEST = $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRC_TEST)) gamemodel.cpp json.cpp block.cpp bun.cpp paths.cpp data.cpp debug.cpp util.cpp)

$(TESTBIN): $(OBJ_TEST) $(OBJ)
	$(CXX) -o $(TESTBIN) $(OBJ_TEST) $(LIBS) $(LDFLAGS_TEST) 

.PHONY: test
test: $(TESTBIN)
	echo Running test suite ...
	./$(TESTBIN)

.PHONY: clean
clean:
	-$(RM) $(OBJDIR)/*.o $(OBJDIR)/*.d $(TESTBIN)

.PHONY:distclean
distclean: clean
	-$(RM) $(BIN)

BUILDDATE=$(shell date +%Y%m%d)
GITHASH=$(shell git log -1 --format='%H')
APPNAME="TINS2022 entry"
VERSION=0.1

.PHONY: updateversion
updateversion:
	@echo "Preparing data/version.ini"
	@echo "# Autogenerated file, do not edit\n\
[version]\n\
builddate=$(BUILDDATE)\n\
version=$(VERSION)\n\
githash=$(GITHASH)\n\
" > data/version.ini
	@echo "Preparing ./version.inc"
	@echo "\\ Autogenerated file, do not edit\n\
#define APPLICATION_NAME \"$(APPNAME)\"\n\
#define APPLICATION_SHORT_VERSION \"$(VERSION)\"\n\
#define APPLICATION_VERSION \"$(VERSION).$(BUILDDATE)\"\n\
" > version.inc

version.inc: updateversion