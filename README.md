# Malcolm
This repository contains the source code for our SIGMETRICS'23 paper entitled [Malcolm: Multi-agent Learning for Cooperative Load Management at Rack Scale](https://doi.org/10.1145/35706110).

### Installation

First, clone the repository and all its sub-modules on all machines in your cluster. 
Then, move to the eRPC directory. Add the following line to `CMakeLists.txt`:

`set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-address-of-packed-member")`.

Change `4096` to `32768` on line 26 of `src/transport.h`. 
Then, install eRPC following instructions from [here](https://github.com/erpc-io/eRPC). 
Note that eRPC requires allocating huge pages on each numa node. 
eRPC also relies on infiniBand or dpdk enabled NICs. 
In addition to the packages required by eRPC, Malcolm requires the following libraries to be installed: `boost_program_options`, `libnuma`, `libdl`, `libibverbs`, `libgoogle-glog-dev`, `libiberty-dev`, `libdl`, and `libfmt-dev`.

### Configuration 
`scripts/runTests.py` provides an automated script for running a Malcolm cluster. 
You need to configure `ACCOUNT`, `PASSWORD` and `DIRECTORY` variables in `scripts/runCluster.py` before using the script.
You also need to define `HostConfigurations` that fits your cluster. 
The script assumes that the repository is cloned into `DIRECTORY` on all machines.

### Citation

To cite our paper, please use the following:
```
@article{10.1145/3570611,
 author = {Hossein Abbasi Abyaneh, Ali and Liao, Maizi and Zahedi, Seyed Majid},
 title = {Malcolm: Multi-agent Learning for Cooperative Load Management at Rack Scale},
 journal = {Proceedings of the ACM on Measurement and Analysis of Computing Systems},
 year = {2022},
 volume = {6},
 number = {3},
 publisher = {ACM}
}
```
