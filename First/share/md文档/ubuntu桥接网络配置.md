# ubuntu桥接网络配置

1. vim /etc/netplan/50-cloud-init.yaml

```shell
network:
    ethernets:
        ens33:
            dhcp4: false
            addresses: [10.0.80.66/24]
            gateway4: 10.0.80.254
            nameservers:
                    addresses: [10.0.1.9, 8.8.8.8]
    version: 2
```

   2.netplane apply           //生效

3. vim /etc/resolv.conf 

~~~shell
nameserver 10.0.1.9
~~~

