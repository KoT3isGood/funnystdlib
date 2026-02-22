# 
# funny project compiler Makefile
#
# We do these tasks inhere:
# - builds minimal fpc
# - builds fpc via fpc minimal
# - builds fpc via fpc

CC = clang++
LINK = clang++
AR = ar rcs

TEMPORAL_DIRECTORY = .make
OUTPUT_DIRECTORY = ../build/tools/fpc

SYSROOT = 






# Compiled files
TIER0_FILES := ../tier0/lib.cpp ../tier0/mem.cpp ../tier0/platform.cpp ../tier0/commandline.cpp ../tier0/rand.cpp

TIER1_FILES := ../tier1/utlbuffer.cpp ../tier1/interface.cpp ../tier1/utlstring.cpp ../tier1/utlvector.cpp ../tier1/utlmap.cpp
TIER1_OBJS := $(TIER1_FILES:.cpp=.o)

TIER2_FILES := ../tier2/filesystem.cpp ../tier2/fileformats/ini.cpp ../tier2/tokenizer.cpp ../tier2/fileformats/json.cpp
TIER2_OBJS := $(TIER2_FILES:.cpp=.o)

FILESYSTEM_FILES := ../stdfilesystems/filesystem_libc.cpp

LIBFPC_FILES := library/helper.cpp library/target.cpp library/builder.cpp library/runner.cpp library/c.cpp library/ld.cpp library/clang/c.cpp library/clang/ld.cpp

FPC_FILES := main.cpp library/helper.cpp library/target.cpp library/builder.cpp





# By default we assume that we will have .so
FPC_LIBRARY_NAMING = minix

# Detecting windows
ifeq ($(FPC_OS),pc-windows)
FPC_LIBRARY_NAMING = windows
else ifeq ($(FPC_OS),windows)
FPC_LIBRARY_NAMING = windows
else ifeq ($(OS),Windows_NT)
FPC_LIBRARY_NAMING = windows
endif

# Detecting macos 
ifeq ($(FPC_OS),apple-darwin)
FPC_LIBRARY_NAMING = apple
else ifeq ($(FPC_OS),macos)
FPC_LIBRARY_NAMING = apple
else ifeq ($(shell uname -s),Darwin)
FPC_LIBRARY_NAMING = apple
endif

ifeq ($(FPC_LIBRARY_NAMING),minix)

TIER0 = libtier0.so
LIBFPC = libfpc.so
FILESYSTEM_STD = libfilesystem_std.so
FPC = fpc
EXECUTABLE_FLAGS := -Wl,--disable-new-dtags -Wl,-rpath,'$$ORIGIN'

else ifeq ($(FPC_LIBRARY_NAMING),apple)

TIER0 = libtier0.dylib
LIBFPC = libfpc.dylib
FILESYSTEM_STD = libfilesystem_std.dylib
FPC = fpc
EXECUTABLE_FLAGS := -Wl,-rpath,'@loader_path'

else ifeq ($(FPC_LIBRARY_NAMING),windows)

TIER0 = tier0.dll
LIBFPC = fpc.dll
FPC = fpc.exe
FILESYSTEM_STD = filesystem_std.dll

endif

TIER1 = libtier1.a
TIER2 = libtier2.a

# Allow cross-compilation
# Mainly for stuff such as iSH
ifneq ($(FPC_ARCH),)
ifneq ($(FPC_OS),)
ifneq ($(FPC_ABI),)
REAL_TARGET := -target $(FPC_ARCH)-$(FPC_OS)-$(FPC_ABI) -DFPC_ARCH=\"$(FPC_ARCH)\" -DFPC_OS=\"$(FPC_OS)\" -DFPC_ABI=\"$(FPC_ABI)\"
else
REAL_TARGET := -target $(FPC_ARCH)-$(FPC_OS) -DFPC_ARCH=\"$(FPC_ARCH)\" -DFPC_OS=\"$(FPC_OS)\"
endif
endif
endif

ifneq ($(SYSROOT),)
REAL_TARGET := --sysroot $(SYSROOT)
endif


CCFLAGS := $(REAL_TARGET) -g -I../public -Ipublic -fPIC

ifeq ($(OS),Windows_NT)
MKDIR = mkdir
else
MKDIR = mkdir -p
endif

.PHONY: all build install install_headers

all: install

build: install_headers \
		$(TEMPORAL_DIRECTORY) \
		$(TEMPORAL_DIRECTORY)/libfpcbuild.a \
		$(TEMPORAL_DIRECTORY)/$(LIBFPC) \
		$(TEMPORAL_DIRECTORY)/$(TIER0) \
		$(TEMPORAL_DIRECTORY)/libtier1.a \
		$(TEMPORAL_DIRECTORY)/libtier2.a \
		$(TEMPORAL_DIRECTORY)/$(FILESYSTEM_STD)
	$(CC) $(FPC_FILES) $(CCFLAGS) $(EXECUTABLE_FLAGS) -o $(TEMPORAL_DIRECTORY)/$(FPC) -ltier0 -L$(TEMPORAL_DIRECTORY) $(TEMPORAL_DIRECTORY)/libtier1.a $(TEMPORAL_DIRECTORY)/libtier2.a
	-$(TEMPORAL_DIRECTORY)/$(FPC) build

install: build
	-$(TEMPORAL_DIRECTORY)/$(FPC) build --install $(OUTPUT_DIRECTORY)


$(TEMPORAL_DIRECTORY)/$(TIER0): $(TIER0_FILES) $(TEMPORAL_DIRECTORY)
	$(CC) $(CCFLAGS) -shared -o $@ $(TIER0_FILES)

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(TEMPORAL_DIRECTORY)/libtier1.a: $(TIER1_OBJS) $(TEMPORAL_DIRECTORY) $(TEMPORAL_DIRECTORY)/$(TIER0)
	ar rcs $@ $(TIER1_OBJS)

$(TEMPORAL_DIRECTORY)/libtier2.a: $(TIER2_OBJS) $(TEMPORAL_DIRECTORY) $(TEMPORAL_DIRECTORY)/libtier1.a
	ar rcs $@ $(TIER2_OBJS)

$(TEMPORAL_DIRECTORY)/$(FILESYSTEM_STD): $(FILESYSTEM_FILES) $(TEMPORAL_DIRECTORY)/libtier1.a $(TEMPORAL_DIRECTORY)/$(TIER0) $(TEMPORAL_DIRECTORY)
	$(CC) $(CCFLAGS) -fPIC -shared -o $@ $(FILESYSTEM_FILES) $(TEMPORAL_DIRECTORY)/libtier1.a -ltier0 -L$(TEMPORAL_DIRECTORY)

$(TEMPORAL_DIRECTORY)/libfpcbuild.a: buildfile/interfaces.o $(TEMPORAL_DIRECTORY)
	ar rcs $@ buildfile/interfaces.o

$(TEMPORAL_DIRECTORY)/$(LIBFPC): $(LIBFPC_FILES) $(TEMPORAL_DIRECTORY) $(TEMPORAL_DIRECTORY)/libfpcbuild.a $(TEMPORAL_DIRECTORY)/libtier1.a $(TEMPORAL_DIRECTORY)/libtier2.a
	$(CC) $(CCFLAGS) $(LIBFPC_FILES) library/libfpc.cpp -shared -o $@ $(TEMPORAL_DIRECTORY)/libtier1.a $(TEMPORAL_DIRECTORY)/libtier2.a -ltier0 -L$(TEMPORAL_DIRECTORY)

$(TEMPORAL_DIRECTORY):
	$(MKDIR) $(TEMPORAL_DIRECTORY)

$(OUTPUT_DIRECTORY):
	$(MKDIR) $(OUTPUT_DIRECTORY)

install_headers: $(TEMPORAL_DIRECTORY)
	cp -r public $(TEMPORAL_DIRECTORY) 
	cp -r ../public/tier0 $(TEMPORAL_DIRECTORY)/public
	cp -r ../public/tier1 $(TEMPORAL_DIRECTORY)/public
	cp -r ../public/tier2 $(TEMPORAL_DIRECTORY)/public

