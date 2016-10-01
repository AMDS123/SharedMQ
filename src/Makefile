CXX = g++

SUBDIRS = util \
		  shm \
		  test

ROOT_DIR=$(shell pwd)

INC = -I$(ROOT_DIR)/../include/shm -I$(ROOT_DIR)/../include/util/ -I/usr/local/include/log4cplus/

OBJS = $(ROOT_DIR)/shm/shmmq.o \
    $(ROOT_DIR)/shm/shmmqprocessor.o \
    $(ROOT_DIR)/shm/shmmqcommu.o \
    $(ROOT_DIR)/util/configreader.o \
    $(ROOT_DIR)/util/errors.o \
    $(ROOT_DIR)/util/notify.o \

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

$(TARGET_LIB): $(OBJS)
	ar cqs $@ $^ $(LIB)

%.o: %.cpp
	$(CXX) $(INC) $(LIB) -c -o $@ $<

clean:
	$(call make_subdir , clean) 
	rm -f $(TARGET_LIB)
	../tools/deleteshm.sh
