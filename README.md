# EE407Project

DV-hop positioning algorithm for NS3.30.1

###Installing
Just download ns-allinone-3.30.1 and then cd into your ns-allinone-3.30.1/ns-3.30.1/src/ directory and clone this repository:

```
$ cd $NS_HOME/src/
$ git clone https://github.com/CunningCam/EE407Project.git
$ cd $NS_HOME
$ ./waf configure --enable-examples
$ ./waf --run dvhop-example
```

The `$NS_HOME` variable is set to the root folder of your NS3.20 installation. In case of the ns-allinone directory structure is `$NS_ROOT/ns3.20`
