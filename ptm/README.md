#### Data-stream enrichment - specifically: node_id / platform_id

##### Example of pmacct's pretag-map (PTM) file format
```TEXT
set_label=nkey%daisy-router-01%pkey%Cisco-XR ip=1.1.1.1/32
set_label=nkey%daisy-router-02%pkey%Cisco-XE ip=2.2.2.2/32
set_label=nkey%daisy-router-03%pkey%Huawei-VRP ip=3.3.3.3/32
set_label=nkey%daisy-router-04%pkey%JunOS ip=4,4,4,4/32
```

if not already existing, crate a file "ptm/label_map.ptm" resembling the above format.
The client IP address (ip field) is matched & the data-stream is enriched
with (set_label field, nkey-value) & (set_label field, pkey-value) according to this data format:

```JSON

"label": {
    "node_id": <nkey-value>,
    "platform_id": <pkey-value>
},
```

When new rows are added to the PTM, you can refresh the daemon seamlessly:

```SHELL
$ sudo kill -USR1 `cat /var/run/mdt_dialout_collector.pid`
```

