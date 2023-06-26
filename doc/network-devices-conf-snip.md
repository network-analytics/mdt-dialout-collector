## Table of Content

<!--ts-->
   * [Cisco-XR 7.4.1@NCS-540](#cisco-xr-741ncs-540)
   * [Cisco-XE 17.06.01prd7@C8000V](#cisco-xe-170601prd7c8000v)
   * [Cisco-NX-OS 10.2(2)@Nexus9000](#cisco-nx-os-1022nexus9000)
   * [JunOS 20.4R3-S2.6@mx10003](#junos-204r3-s26mx10003)
   * [Huawei VRP V800R021C10SPC300T@NE40E](#huawei-vrp-v800r021c10spc300tne40e)
<!--te-->

## Cisco-XR 7.4.1@NCS-540
```SHELL
# Reference documentation: https://www.cisco.com/c/en/us/td/docs/routers/asr9000/software/asr9k-r7-0/telemetry/configuration/guide/b-telemetry-cg-asr9000-70x/b-telemetry-cg-asr9000-70x_chapter_010.html

telemetry model-driven strict-timer
telemetry model-driven destination-group COLLECTOR
telemetry model-driven destination-group COLLECTOR address-family ipv4 192.168.100.254 port 10001
telemetry model-driven destination-group COLLECTOR address-family ipv4 192.168.100.254 port 10001 encoding json
telemetry model-driven destination-group COLLECTOR address-family ipv4 192.168.100.254 port 10001 protocol grpc no-tls
telemetry model-driven sensor-group SENSOR
telemetry model-driven sensor-group SENSOR sensor-path openconfig-interfaces:interfaces
telemetry model-driven
telemetry model-driven subscription SUBSCRIPTION
telemetry model-driven subscription SUBSCRIPTION sensor-group-id SENSOR sample-interval 60000
telemetry model-driven subscription SUBSCRIPTION destination-id COLLECTOR
telemetry model-driven subscription SUBSCRIPTION source-interface Loopback0

```

## Cisco-XE 17.06.01prd7@C8000V
```SHELL
# Reference documentation: https://www.cisco.com/c/en/us/td/docs/ios-xml/ios/prog/configuration/173/b_173_programmability_cg/model_driven_telemetry.html

telemetry ietf subscription 1
!
 encoding encode-kvgpb
 filter xpath /oc-if:interfaces/oc-if:interface
 source-address 192.168.100.100
 stream yang-push
 update-policy periodic 6000
 receiver ip address 192.168.100.254 10001 protocol grpc-tcp

```

## Cisco-NX-OS 10.2(2)@Nexus9000
```SHELL
# Reference documentation: https://www.cisco.com/c/en/us/td/docs/dcn/nx-os/nexus9000/101x/programmability/cisco-nexus-9000-series-nx-os-programmability-guide-release-101x/m-n9k-model-driven-telemetry-101x.html

feature telemetry
!
feature openconfig
!
telemetry
  !
  destination-profile
    use-vrf vrf100
    source-interface Vlan100
  !
  destination-group 1
    ! encoding GPB-KV
    host 192.168.100.254 port 10001 protocol gRPC encoding GPB
      ! enabling gRPC dial-out
      grpc-async
  !
  sensor-group 1
    data-source YANG
    path openconfig-interfaces:interfaces
  !
  subscription 1
    dst-grp 1
    snsr-grp 1 sample-interval 60000
!
```

## JunOS 20.4R3-S2.6@mx10003
```SHELL
# Reference documentation: https://www.juniper.net/documentation/us/en/software/junos/interfaces-telemetry/topics/topic-map/telemetry-grpc-dialout-ta.html

set groups TLM services analytics streaming-server GRPC_SERVER remote-address 192.168.100.254
set groups TLM services analytics streaming-server GRPC_SERVER remote-port 10001
!
set groups TLM services analytics export-profile GRPC_PROFILE local-address 192.168.100.100
set groups TLM services analytics export-profile GRPC_PROFILE reporting-rate 60
set groups TLM services analytics export-profile GRPC_PROFILE format json-gnmi
set groups TLM services analytics export-profile GRPC_PROFILE transport grpc
!
set groups TLM services analytics sensor OC_IF server-name GRPC_SERVER
set groups TLM services analytics sensor OC_IF export-name GRPC_PROFILE
set groups TLM services analytics sensor OC_IF resource /interfaces
```

## Huawei VRP V800R021C10SPC300T@NE40E
```SHELL
# Reference documentation: https://support.huawei.com/enterprise/en/doc/EDOC1100290800/862530fd/example-for-configuring-grpc-in-dial-out-mode

telemetry
 #
 sensor-group SENSOR
  sensor-path openconfig-interfaces:interfaces/interface/state
  sensor-path openconfig-interfaces:interfaces/interface/state/counters
 #
 destination-group TLM
  ipv4-address 192.168.100.254 port 10001 protocol grpc no-tls
 #
 subscription SUBSCRIPTION
  local-source-address ipv4 192.168.100.100
  sensor-group SENSOR
  destination-group TLM
#
```
