
EXTRA_DIST = oraapi.proto

noinst_PROGRAMS = sandbox oracli orad gen-sum pbmaker

noinst_LIBRARIES = libsandbox.a liboraapi.a

AM_CXXFLAGS = -std=gnu++0x -I$(top_srcdir)/lib

libsandbox_a_SOURCES = \
	sandbox.h \
	util.cc

sandbox_SOURCES = \
	sandbox_elf.cc \
	sandbox.cc
sandbox_LDADD = ../lib/libmoxievm.a libsandbox.a $(ELF_LIBS) $(CRYPTO_LIBS)

liboraapi_a_SOURCES = \
	oraapi.pb.cc oraapi.pb.h \
	oraapi.h \
	libhttp.cc

pbmaker_SOURCES = pbmaker.cc
pbmaker_LDADD = liboraapi.a $(CRYPTO_LIBS) $(EVENT_LIBS) $(UNIVALUE_LIBS) $(PROTOBUF_LIBS)

oracli_SOURCES = \
	oracli.cc
oracli_LDADD = liboraapi.a $(CRYPTO_LIBS) $(EVENT_LIBS) $(UNIVALUE_LIBS) $(PROTOBUF_LIBS) $(SSL_LIBS)

orad_SOURCES = \
	orad.cc
orad_LDADD = liboraapi.a ../lib/libmoxievm.a libsandbox.a $(ELF_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS) $(UNIVALUE_LIBS) $(PROTOBUF_LIBS)

gen_sum_SOURCES = gen-sum.cc
gen_sum_LDADD = libsandbox.a $(CRYPTO_LIBS)

BUILT_SOURCES = oraapi.pb.cc oraapi.pb.h
CLEANFILES = oraapi.pb.cc oraapi.pb.h

oraapi.pb.cc: oraapi.proto
	protoc --cpp_out=. oraapi.proto
oraapi.pb.h: oraapi.proto
	protoc --cpp_out=. oraapi.proto

