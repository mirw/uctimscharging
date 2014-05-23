# $Id: Makefile 544 2008-03-28 17:29:22Z vingarzan $ Makefile $Date: 2008-03-28 17:29:22 +0000 (Fri, 28 Mar 2008) $ $Author: vingarzan $ dvi Dragos Vingarzan
# 
# Makefile for the CDiameterPeer - standalone compilation
#

CC := gcc


# Sources 

sources_base := \
	$(wildcard cdp/*.c) \
	$(wildcard utils/*.c) \
	$(wildcard ocs/*.c) \
	$(wildcard *.c)

sources_exclude := $(wildcard cdp/mod.*) $(wildcard cdp/cdp_load.*)

sources_base := $(filter-out $(sources_exclude), $(sources_base))

sources := $(sources_base)

alldep := Makefile

mkdep := $(CC) -MM

objects_base 	:= $(sources_base:.c=.o) 
objects		:= $(sources:.c=.o) 

depends_base	:= $(sources_base:.c=.d)
depends		:= $(sources:.c=.d)

# Defines and libs

libs := -lxml2

defines_gen := -DSHM_MEM -DSHM_MMAP -DPKG_MALLOC -DARCH=\"i386\" -DOS=\"linux\" \
-DFAST_LOCK -DADAPTIVE_WAIT -DHAVE_SCHED_YIELD -DADAPTIVE_WAIT_LOOPS=128 -D__CPU_i386 \
-I/usr/include/libxml2 

defines_dbg := -DDBG_QM_MALLOC 

defines_release := -DF_MALLOC

cflags := -O0 -g3 -Wall 


defines := $(defines_gen) $(defines_dbg) 

ifeq ($(mode),release)
	cflags := -O3 -Wall
	defines := $(defines_gen) $(defines_release)
endif



#Rules

%.o: %.c  $(alldep)
		@echo $@
		$(CC) $(cflags) $(defines) -c $< 		-o $@

%.d: %.c $(alldep)
		@set -e; $(mkdep) $(defines) $< \
		|  sed 's#\(\($*D)\)\?$(*F)\)\.o[ :]*#$*.o $@ : #g' > $@; \
		[ -s $@ ] || rm -f $@



all: $(depends_base) $(objects_base)
	$(CC) -o uctimsocs $(objects_base) $(libs)
		
		

clean:
	rm -rf $(objects) uctimsocs
	rm -f $(PROG) $(ODIR)/*.o *~ $(SRC_DIR)/*~
	rm -rf cdp/*.o
	rm -rf cdp/*~
	rm -rf ocs/*.o
	rm -rf ocs/*~
	rm -rf utils/*.o
	rm -rf utils/*~
	rm -rf *.o
	rm -rf *~

distclean: 
	rm -rf $(objects) $(depends) uctimsocs core*


