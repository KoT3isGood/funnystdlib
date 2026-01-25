# We want to build just enough to use other stuff
TIER0_FILES := ../tier0/lib.cpp ../tier0/mem.cpp ../tier0/platform.cpp ../tier0/commandline.cpp ../tier0/rand.cpp
TIER1_FILES := ../tier1/utlbuffer.cpp ../tier1/interface.cpp ../tier1/utlstring.cpp ../tier1/utlvector.cpp ../tier1/utlmap.cpp
TIER2_FILES := ../tier2/filesystem.cpp ../tier2/fileformats/ini.cpp ../tier2/tokenizer.cpp ../tier2/fileformats/json.cpp
FILESYSTEM_FILES := ../stdfilesystems/filesystem_libc.cpp
TIER1_OBJS := $(TIER1_FILES:.cpp=.o)
TIER2_OBJS := $(TIER2_FILES:.cpp=.o)
FPC_FILES := library/helper.cpp library/target.cpp library/builder.cpp library/runner.cpp library/c.cpp library/ld.cpp library/clang/c.cpp library/clang/ld.cpp
CC = clang++

ifneq ($(FPC_ARCH),)
ifneq ($(FPC_OS),)
ifneq ($(FPC_ABI),)
REAL_TARGET := -target $(FPC_ARCH)-$(FPC_OS)-$(FPC_ABI) -DFPC_ARCH=\"$(FPC_ARCH)\" -DFPC_OS=\"$(FPC_OS)\" -DFPC_ABI=\"$(FPC_ABI)\"
else
REAL_TARGET := -target $(FPC_ARCH)-$(FPC_OS) -DFPC_ARCH=\"$(FPC_ARCH)\" -DFPC_OS=\"$(FPC_OS)\"
endif
endif
endif

CCFLAGS = $(REAL_TARGET) -g -I../public -Ipublic

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
CCFLAGS += -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -std=c++11 -Wl,-export_dynamic
endif
ifeq ($(UNAME_S),Linux)
endif

recompile: ../build/tools/fpc
	build/fpc build

install: ../build/tools/fpc build/libfpcbuild.a build/libfpc.so build/libtier0.so build/libtier1.a build/libtier2.a build/libfilesystem_std.so install_temp builddir
	$(CC) -fPIC main.cpp library/helper.cpp library/target.cpp library/builder.cpp -lc -lstdc++ $(CCFLAGS) -o build/fpc -ltier0 -Lbuild build/libtier1.a build/libtier2.a -Wl,--disable-new-dtags -Wl,-rpath,'$$ORIGIN'
	build/fpc build -fpcdebug
	mv build/fpc_temp build/fpc
	mv build/libfpc_temp.so build/libfpc.so
	build/fpc build -fpcdebug
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

build/libfpc.so: $(FPC_FILES) builddir build/libfpcbuild.a build/libtier1.a build/libtier2.a
	$(CC) $(CCFLAGS) $(FPC_FILES) library/libfpc.cpp -fPIC -shared -o build/libfpc.so build/libtier1.a build/libtier2.a

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

