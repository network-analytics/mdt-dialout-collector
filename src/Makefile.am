MAINTAINERCLEANFILES = Makefile.in

SUBDIRS =          \
	bridge           \
	cfgWrapper       \
	core             \
	dataDelivery     \
	dataManipulation \
	dataWrapper      \
	proto            \
	utils

#
# Libs
#
lib_LTLIBRARIES = libgrpc_collector.la
libgrpc_collector_la_SOURCES =
libgrpc_collector_la_LIBADD = bridge/libgrpc_collector_bridge.la               \
			cfgWrapper/libgrpc_collector_cfg_wrapper.la            \
			core/libgrpc_collector_core.la                         \
			dataDelivery/libgrpc_collector_data_delivery.la        \
			dataManipulation/libgrpc_collector_data_manipulation.la\
			dataWrapper/libgrpc_collector_data_wrapper.la          \
			proto/libgrpc_collector_proto.la                       \
			utils/libgrpc_collector_utils.la                       \
			$(DEPS)
#Distributable headers
#include_HEADERS = distributable_header.h

#
# bin program
#
AM_CPPFLAGS = -I$(top_srcdir)/src/include/
AM_LDFLAGS = "-Wl,--copy-dt-needed-entries"

DEPS = $(GRPC_LIBS) $(GRPCCPP_LIBS) $(PROTOBUF_LIBS) \
	$(JSONCPP_LIBS) \
	$(KAFKA_LIBS) \
	$(KAFKACPP_LIBS) \
	$(SPDLOG_LIBS) \
	-lzmq \
	-lconfig++ \
	-lfmt \
	-lpthread

# Program name
bin_PROGRAMS = mdt_dialout_collector pmtelemetryd

mdt_dialout_collector_SOURCES = mdt_dialout_collector.cc
mdt_dialout_collector_LDADD = libgrpc_collector.la

pmtelemetryd_SOURCES = pmtelemetryd.c
pmtelemetryd_LDADD = libgrpc_collector.la
