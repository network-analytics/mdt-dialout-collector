#### Data-stream enrichment - specifically: node_id / platform_id

##### Example of CSV file format
```TEXT
1.1.1.1,daisy-router-01,Cisco-XR
2.2.2.2,daisy-router-02,Cisco-XE
3.3.3.3,daisy-router-03,Huawei-VRP
4,4,4,4,daisy-router-04,JunOS
```

The client IP address (field1) is matched & the data-stream is enriched
with field2 & field3 according to this data format:

```JSON

"label": {
    "node_id": <field2>,
    "platform_id": <field3>
},
```

