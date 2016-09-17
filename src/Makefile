CXX = g++

SUBDIRS = util \
		  shm \
		  test

ROOT_DIR=$(shell pwd)

INC = -I$(ROOT_DIR)/../include/shm -I$(ROOT_DIR)/../include/util/ -I/usr/local/include/log4cplus/

LIB = -llog4cplus

OBJS = $(ROOT_DIR)/shm/shmmq.o \
    $(ROOT_DIR)/shm/shmmqprocessor.o \
    $(ROOT_DIR)/shm/shmmqcommu.o \
    $(ROOT_DIR)/util/configreader.o \
    $(ROOT_DIR)/util/errors.o \
    $(ROOT_DIR)/util/notify.o \
    $(ROOT_DIR)/util/mylog.o

export CXX INC OBJS LIB

define make_subdir
 @for subdir in $(SUBDIRS) ; do \
 ( cd $$subdir && make $1) \
 done;
endef

all: $(SUBDIRS)

$(SUBDIRS):ECHO
	make -C $@

ECHO:
	@echo $(SUBDIRS)
	@echo begin compile

%.o: %.cpp
	$(CXX) $(INC) $(LIB) -c -o $@ $<

clean:
	$(call make_subdir , clean) 
	../tools/deleteshm.sh
