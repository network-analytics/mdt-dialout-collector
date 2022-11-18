MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libgrpc_collector_data_delivery.la

libgrpc_collector_data_delivery_la_SOURCES = kafka_delivery.cc zmq_delivery.cc
libgrpc_collector_data_delivery_la_CPPFLAGS = -I$(top_srcdir)/src/include/ -I$(top_builddir)/src/
