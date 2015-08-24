# Grand Unified Makefile

# File naming conventions:
#	project/module.cpp       = Part of the library
#	project/module-test.cpp  = Unit test for part of the library
#	project/app-module.cpp   = Part of the application
# If the current directory ends in -lib, the library is installable;
# otherwise, if any application modules exist, the application is installable;
# otherwise, nothing is installable.

LIBROOT      ?= ..
NAME         := $(shell ls */*.{c,h,cpp,hpp} 2>/dev/null | sed -E 's!/.*!!' \
				| uniq -c | sort | tail -n 1 | sed -E 's!^[ 0-9]+!!')
TAG          := $(shell echo "$$PWD" | sed -E 's!^([^/]*/)*([^/]*-)?!!')
PREFIX       := /usr/local
HOST         := $(shell uname | tr A-Z a-z | sed -E 's/[^a-z].*//')
TARGET       := $(shell gcc -v 2>&1 | grep '^Target:' | sed -E -e 's/^Target: //' -e 's/[0-9.]*$$//' | tr A-Z a-z)
XHOST        := $(shell echo $(TARGET) | tr A-Z a-z | sed -E -e 's/-gnu$$//' -e 's/.*-//' -e 's/[^a-z].*//')
CXX          := g++
CXXFLAGS     := -I. -g2 -march=core2 -mfpmath=sse -Wall -Wextra -Werror -Wold-style-cast -Woverloaded-virtual
DEFINES      := -DNDEBUG=1
OPT          := -O2
TESTOPT      := -O1
AR           := ar
ARFLAGS      := -rsu
LDFLAGS      :=
LDLIBS       :=
EXE          :=
DEPENDS      := dependencies.make
MD           := multimarkdown
MDFLAGS      := --process-html
STATICLIB    := build/$(TARGET)/lib$(NAME).a
SOURCES      := $(wildcard $(NAME)/*.c $(NAME)/*.cpp)
APPSOURCES   := $(wildcard $(NAME)/app-*.cpp)
TESTSOURCES  := $(wildcard $(NAME)/*-test.cpp)
LIBSOURCES   := $(filter-out $(APPSOURCES) $(TESTSOURCES),$(SOURCES))
HEADERS      := $(wildcard $(NAME)/*.h $(NAME)/*.hpp)
LIBHEADERS   := $(filter-out $(NAME)/library.hpp,$(shell grep -EL '// NOT INSTALLED' $(HEADERS) Makefile)) # Dummy entry to avoid empty list
APPOBJECTS   := $(patsubst $(NAME)/%.cpp,build/$(TARGET)/%.o,$(APPSOURCES))
TESTOBJECTS  := $(patsubst $(NAME)/%.cpp,build/$(TARGET)/%.o,$(TESTSOURCES))
LIBOBJECTS   := $(patsubst $(NAME)/%.cpp,build/$(TARGET)/%.o,$(LIBSOURCES))
DOCINDEX     := $(wildcard $(NAME)/index.md)
DOCSOURCES   := $(sort $(wildcard $(NAME)/*.md))
DOCS         := doc/style.css doc/index.html $(patsubst $(NAME)/%.md,doc/%.html,$(DOCSOURCES))
CORELIBS 	 := $(shell ls $(LIBROOT)/*-lib/*/*.hpp | sed -E 's!-lib/.*hpp!-lib!' | sort -u)
LIBREGEX     := $(shell sed -E 's!([^A-Za-z0-9/_-])!\\&!g' <<< "$(LIBROOT)")
LIBTAG       :=
SCRIPTS      := $(LIBROOT)/prion-lib/scripts

ifeq ($(HOST),cygwin)
	EXE := .exe
	LDFLAGS += -Wl,--enable-auto-import
	LIBTAG := cygwin
endif

ifeq ($(HOST),darwin)
	CXX := clang++
	DEFINES += -D_DARWIN_C_SOURCE=1
	LIBTAG := mac
endif

ifeq ($(HOST),linux)
	LIBTAG := linux
endif

ifeq ($(CXX),clang++)
	CXXFLAGS += -std=c++1z -stdlib=libc++
else
	CXXFLAGS += -std=c++14
endif

ifeq ($(XHOST),mingw)
	MINGW := $(shell type -P gcc | sed -E 's!/bin/.+!!')
	PREFIX := $(MINGW)
	CXXFLAGS += -mthreads
	DEFINES += -DNOMINMAX=1 -DUNICODE=1 -D_UNICODE=1 -DWINVER=0x601 -D_WIN32_WINNT=0x601 -DPCRE_STATIC=1
	LIBPATH := $(MINGW)/lib $(subst ;, ,$(LIBRARY_PATH))
	LIBTAG := mingw
else
	DEFINES += -D_REENTRANT=1 -D_XOPEN_SOURCE=700
	LDLIBS += -lpthread
	ifneq ($(XHOST),linux)
		LDLIBS += -liconv
	endif
	LIBPATH := /usr/lib $(subst :, ,$(LIBRARY_PATH))
endif

EXTRALIBS := $(shell grep -h PRI_LDLIB $(HEADERS) $(SOURCES) \
	| grep -v 'define PRI_LDLIB' \
	| sed -E 's/PRI_LDLIB\((.*)\).*/\1/' \
	| grep -E '$(LIBTAG):|^[^:]+$$' \
	| sed -E -e 's/$(LIBTAG)://' -e 's/[^ ]+/-l&/g' \
	| sort -u)

PCRELIBS := $(shell find $(LIBPATH) -name 'libpcre16.*' -or -name 'libpcre32.*' 2>/dev/null \
			| grep -Ev '\.pc$$' | sed -E 's!.*/!!')

ifneq ($(filter libpcre16.%,$(PCRELIBS)),)
	DEFINES += -DUNICORN_PCRE16=1
	LDLIBS += -lpcre16
endif

ifneq ($(filter libpcre32.%,$(PCRELIBS)),)
	DEFINES += -DUNICORN_PCRE32=1
	LDLIBS += -lpcre32
endif

ifneq ($(shell grep -Fior 'sdl2/sdl.h' $(NAME)),)
	DEFINES += -DSDL_MAIN_HANDLED=1
	ifeq ($(HOST),cygwin)
		CXXFLAGS += -mwindows
	endif
	# TODO - probably needed for iOS
	# undef SDL_MAIN_HANDLED
	# LDLIBS := -lSDL2main $(LDLIBS)
endif

LD      := $(CXX)
LDLIBS  := $(LDLIBS) -lpcre -lz
APP     := build/$(TARGET)/$(NAME)$(EXE)
TESTER  := build/$(TARGET)/test-$(NAME)$(EXE)

.DELETE_ON_ERROR:

.PHONY: all static app run unit check install symlinks uninstall doc unlink undoc clean cleanall dep help
.PHONY: help-prefix help-app help-test help-install help-suffix print-%

all: static app unit doc

unlink:
	rm -f $(STATICLIB) $(APP) $(TESTER)

undoc:
	rm -f doc/*.html

clean:
	rm -rf build/$(TARGET) *.stackdump __*

cleanall:
	rm -rf build doc *.stackdump __*

dep:
	$(CXX) $(CXXFLAGS) $(DEFINES) $(patsubst %,-I%,$(CORELIBS)) -MM */*.cpp \
		| sed -E -e 's! \.\./$(NAME)/! !g' \
				 -e 's!^[[:graph:]]*\.o:!build/$$(TARGET)/&!' \
				 -e 's!^ +!  !' \
				 -e 's!$(LIBREGEX)!$$(LIBROOT)!g' \
		> $(DEPENDS)

help: help-suffix

help-prefix:
	@echo "Make targets:"
	@echo "    all        = Build everything (default)"
	@echo "    static     = Build the static library"

help-suffix: help-install
	@echo "    doc        = Build the documentation"
	@echo "    unlink     = Delete link targets but not object modules"
	@echo "    undoc      = Delete the documentation"
	@echo "    clean      = Delete all targets for this toolset"
	@echo "    cleanall   = Delete all targets for all toolsets and docs"
	@echo "    dep        = Rebuild makefile dependencies"
	@echo "    help       = List make targets"
	@echo "    print-*    = Query a makefile variable"

print-%:
	@echo "$* = $($*)"

ifeq ($(APPSOURCES),)
app:
run:
help-app: help-prefix
else
app: $(APP)
run: app
	$(APP)
help-app: help-prefix
	@echo "    app        = Build the application"
	@echo "    run        = Build and run the application"
endif

ifeq ($(LIBSOURCES),)
STATICPART :=
else
STATICPART := $(STATICLIB)
endif

ifeq ($(TESTSOURCES),)
unit:
check:
help-test: help-app
else
unit: $(TESTER)
check: all
	@rm -rf __test_*
	$(TESTER)
	@rm -rf __test_*
help-test: help-app
	@echo "    unit       = Build the unit tests"
	@echo "    check      = Build and run the unit tests"
endif

ifeq ($(DOCSOURCES),)
doc:
else
doc: $(DOCS)
endif

ifeq ($(TAG),lib)
# Library is the installable target
static: $(STATICPART) $(NAME)/library.hpp
ifeq ($(LIBSOURCES),)
install: uninstall static
	mkdir -p $(PREFIX)/lib $(PREFIX)/include/$(NAME)
	cp $(LIBHEADERS) $(NAME)/library.hpp $(PREFIX)/include/$(NAME)
else
install: uninstall static
	mkdir -p $(PREFIX)/lib $(PREFIX)/include/$(NAME)
	cp $(LIBHEADERS) $(NAME)/library.hpp $(PREFIX)/include/$(NAME)
	cp $(STATICLIB) $(PREFIX)/lib
endif
ifeq ($(XHOST),mingw)
symlinks:
	$(error make symlinks is not supported on $(XHOST), use make install)
else
ifeq ($(LIBSOURCES),)
symlinks: uninstall static
	mkdir -p $(PREFIX)/lib $(PREFIX)/include
	ln -fs $(abspath $(NAME)) $(PREFIX)/include
else
symlinks: uninstall static
	mkdir -p $(PREFIX)/lib $(PREFIX)/include
	ln -fs $(abspath $(NAME)) $(PREFIX)/include
	ln -fs $(abspath $(STATICLIB)) $(PREFIX)/lib
endif
endif
uninstall:
	rm -f $(PREFIX)/lib/$(notdir $(STATICLIB))
	rm -rf $(PREFIX)/include/$(NAME)
help-install: help-test
	@echo "    install    = Install the application"
	@echo "    symlinks   = Install the application using symlinks"
	@echo "    uninstall  = Uninstall the application"
else
static: $(STATICPART)
ifneq ($(APPSOURCES),)
# Application is the installable target
install: uninstall app
	mkdir -p $(PREFIX)/bin
	cp $(APP) $(PREFIX)/bin
ifeq ($(XHOST),mingw)
symlinks:
	$(error make symlinks is not supported on $(XHOST), use make install)
else
symlinks: uninstall app
	mkdir -p $(PREFIX)/bin
	ln -fs $(abspath $(APP)) $(PREFIX)/bin
endif
uninstall:
	rm -f $(PREFIX)/bin/$(notdir $(APP))
help-install: help-test
	@echo "    install    = Install the library"
	@echo "    symlinks   = Install the library using symlinks"
	@echo "    uninstall  = Uninstall the library"
else
# No installable target
install:
symlinks:
uninstall:
help-install: help-test
endif
endif

-include $(DEPENDS)

LDLIBS := $(EXTRALIBS) $(LDLIBS)

build/$(TARGET)/%-test.o: $(NAME)/%-test.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(TESTOPT) -c $< -o $@

build/$(TARGET)/%.o: $(NAME)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(OPT) -c $< -o $@

$(STATICLIB): $(LIBOBJECTS)
	@mkdir -p $(dir $@)
	@rm -f $@
	$(AR) $(ARFLAGS) $@ $^

$(APP): $(APPOBJECTS) $(STATICPART)
	@mkdir -p $(dir $@)
	$(LD) $(CXXFLAGS) $(DEFINES) $(OPT) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TESTER): $(TESTOBJECTS) $(STATICPART)
	@mkdir -p $(dir $@)
	$(LD) $(CXXFLAGS) $(DEFINES) $(TESTOPT) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(NAME)/library.hpp: $(LIBHEADERS)
	echo "#pragma once" > $@
	echo >> $@
	echo $(sort $(LIBHEADERS)) | tr ' ' '\n' | sed -E 's/.+/#include "&"/' >> $@

ifeq ($(DOCINDEX),)
doc/index.html: $(DOCSOURCES)
	@mkdir -p $(dir $@)
	$(SCRIPTS)/make-index | $(MD) $(MDFLAGS) > $@
endif

doc/%.html: $(NAME)/%.md
	@mkdir -p $(dir $@)
	$(MD) $(MDFLAGS) $< > $@
	$(SCRIPTS)/highlight-code $@

doc/style.css: $(SCRIPTS)/style.css
	@mkdir -p $(dir $@)
	cp $< $@
