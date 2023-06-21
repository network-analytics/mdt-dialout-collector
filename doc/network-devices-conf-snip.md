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
```

## Huawei VRP V800R021C10SPC300T@NE40E
```SHELL
```
