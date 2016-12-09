CXX = g++

SUBDIRS = util shm test

ROOT_DIR=$(shell pwd)

INC = -I$(ROOT_DIR)/../include/shm -I$(ROOT_DIR)/../include/util/
LIB = -lpthread
SHM_OBJS = $(addsuffix .o, $(basename $(wildcard $(ROOT_DIR)/shm/*.cpp)))
UTIL_OBJS = $(addsuffix .o, $(basename $(wildcard $(ROOT_DIR)/util/*.cpp)))

OBJS = $(SHM_OBJS) $(UTIL_OBJS)

export CXX INC OBJS LIB

define make_subdir
 @for subdir in $(SUBDIRS) ; do \
 ( cd $$subdir && make $1) \
 done;
endef

HEADER_FILES += $(shell find ../include/ -name '*.hpp')

TARGET_LIB = libshmmq.a

all: $(SUBDIRS) $(TARGET_LIB)

install:
	mkdir -p /usr/local/include/shmmq/
	cp -f $(HEADER_FILES) /usr/local/include/shmmq/
	cp -f $(TARGET_LIB) /usr/local/lib/

uninstall:
	rm -rf /usr/local/include/shmmq
	rm -f /usr/local/lib/$(TARGET_LIB)

$(SUBDIRS): ECHO
	make -C $@

ECHO:
	@echo $(SUBDIRS)
	@echo begin compile

$(TARGET_LIB): $(OBJS) $(HEADER_FILES)
	ar cqs $@ $^

%.o: %.cpp
	$(CXX) $(INC) -c -o $@ $< $(LIB)

clean:
	$(call make_subdir , clean)
	-rm -f $(TARGET_LIB)
	-../tools/deleteshm.sh
