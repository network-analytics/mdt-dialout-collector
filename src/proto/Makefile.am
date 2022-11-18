MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libgrpc_collector_proto.la

libgrpc_collector_proto_la_SOURCES = \
			Huawei/huawei_dialout.pb.cc \
			Huawei/huawei_telemetry.pb.cc \
			Huawei/huawei_telemetry.grpc.pb.cc \
			Huawei/huawei_dialout.grpc.pb.cc \
			Cisco/cisco_telemetry.pb.cc \
			Cisco/cisco_dialout.grpc.pb.cc \
			Cisco/cisco_dialout.pb.cc \
			Cisco/cisco_telemetry.grpc.pb.cc \
			Juniper/juniper_dialout.pb.cc \
			Juniper/juniper_gnmi.pb.cc \
			Juniper/juniper_gnmi_ext.pb.cc \
			Juniper/juniper_gnmi_ext.grpc.pb.cc \
			Juniper/juniper_telemetry_header_extension.pb.cc \
			Juniper/juniper_telemetry.pb.cc \
			Juniper/juniper_telemetry.grpc.pb.cc \
			Juniper/juniper_telemetry_header_extension.grpc.pb.cc \
			Juniper/juniper_gnmi.grpc.pb.cc \
			Juniper/juniper_dialout.grpc.pb.cc \
			OpenConfig/openconfig_interfaces.pb.cc \
			OpenConfig/openconfig_interfaces.grpc.pb.cc

libgrpc_collector_proto_la_CPPFLAGS = -I$(top_builddir)/src/include/ -I$(top_srcdir)/src/include/
