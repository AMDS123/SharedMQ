CXX = g++

SUBDIRS = util \
		  shm \
		  test

ROOT_DIR=$(shell pwd)/

INC = -I$(ROOT_DIR)/../include/shm -I$(ROOT_DIR)/../include/util/

OBJS = $(ROOT_DIR)/shm/shmmq.o $(ROOT_DIR)/shm/shmmqprocessor.o $(ROOT_DIR)/shm/shmmqcommu.o $(ROOT_DIR)/util/configreader.o

export CXX INC OBJS

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
	$(CXX) $(INC) -c -o $@ $<

clean:
	$(call make_subdir , clean) 
	../tools/deleteshm.sh

#cleanobjs:
#	rm $(OBJS) sender.o rec.o
