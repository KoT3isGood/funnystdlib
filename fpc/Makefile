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

TEMPORAL_DIR = .make

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

ifeq ($(FPC_LIBRARY_NAMIN),minix)

TIER0 = libtier0.so
LIBFPC = libfpc.so
FILESYSTEM_STD = libfilesystem_std.so
FPC = fpc

else ifeq ($(FPC_LIBRARY_NAMIN),apple)

TIER0 = libtier0.dylib
LIBFPC = libfpc.dylib
FILESYSTEM_STD = libfilesystem_std.dylib
FPC = fpc

else ifeq ($(FPC_LIBRARY_NAMIN),windows)

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


CCFLAGS = $(REAL_TARGET) -g -I../public -Ipublic

ifeq ($(OS),Windows_NT)
CREATEDIR = mkdir
else
CREATEDIR = mkdir -p
endif


recompile: ../build/tools/fpc
	-build/fpc build

install: ../build/tools/fpc build/libfpcbuild.a build/libfpc.so build/libtier0.so build/libtier1.a build/libtier2.a build/libfilesystem_std.so install_temp builddir
	$(CC) -fPIC $(FPC_FILES) $(CCFLAGS) -o build/fpc -ltier0 -Lbuild build/libtier1.a build/libtier2.a -Wl,--disable-new-dtags -Wl,-rpath,'$$ORIGIN'
	-build/fpc build
	mv build/fpc_temp build/fpc
	mv build/libfpc_temp.so build/libfpc.so
	-build/fpc build
	mv build/fpc_temp build/fpc
	mv build/libfpc_temp.so build/libfpc.so

build/libtier0.so: $(TIER0_FILES) builddir
	$(CC) $(CCFLAGS) -fPIC -shared -o build/libtier0.so $(TIER0_FILES)

%.o: %.cpp
	$(CC) $(CCFLAGS) -fPIC -c $< -o $@

build/libtier1.a: $(TIER1_OBJS) builddir build/libtier0.so
	ar rcs build/libtier1.a $(TIER1_OBJS)

build/libtier2.a: $(TIER2_OBJS) builddir build/libtier1.a
	ar rcs build/libtier2.a $(TIER2_OBJS)

build/libfilesystem_std.so: $(FILESYSTEM_FILES) build/libtier1.a build/libtier0.so builddir
	$(CC) $(CCFLAGS) -fPIC -shared -o build/libfilesystem_std.so $(FILESYSTEM_FILES) build/libtier1.a -ltier0 -Lbuild

build/libfpcbuild.a: buildfile/interfaces.o builddir
	ar rcs build/libfpcbuild.a buildfile/interfaces.o

build/libfpc.so: $(LIBFPC_FILES) builddir build/libfpcbuild.a build/libtier1.a build/libtier2.a
	$(CC) $(CCFLAGS) $(LIBFPC_FILES) library/libfpc.cpp -fPIC -shared -o build/libfpc.so build/libtier1.a build/libtier2.a -ltier0 -Lbuild

builddir:
	mkdir -p build

../build/tools/fpc:
	mkdir -p ../build/tools/fpc

install_fpc: ../build/tools/fpc
	cp -r build/* ../build/tools/fpc

install_temp: builddir
	cp -r public build 
	cp -r ../public/tier0 build/public
	cp -r ../public/tier1 build/public
	cp -r ../public/tier2 build/public

auto: install install_fpc

