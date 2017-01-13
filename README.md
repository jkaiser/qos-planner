# Quality of Service Planner (QoS Planner) (for Lustre)

The QoS Planner is an external framework for the distributed file system Lustre. The main task is to receive resource requests (X MB/s for files Y1, Y2,... for Z seconds), check whether these are available and, if so, ensure that the user really sees this reserved performance.

Please note that the system still is under developement.

## Required third party libaries

* cmake
* gflags
* protobuf >= 3.10
* spdlog from https://github.com/gabime/spdlog
* nlohmann/json from https://nlohmann.github.io/json/

For example, under an Ubuntu 16.04.1 LTS you could install them with this script:
```bash
sudo aptitude install cmake
sudo aptitude install libgflags2v5 libgflags-dev

# install spdlog
sudo aptitude install libspdlog-dev

#istall protobuf >=3.0
wget https://github.com/google/protobuf/releases/download/v3.1.0/protobuf-cpp-3.1.0.tar.gz
tar xf protobuf-cpp-3.1.0.tar.gz
cd protobuf-3.1.0
./configure && make -j8
sudo make install

cd ..

# install nlohman/json
git clone https://github.com/nlohmann/json.git
mkdir json_build
cd json_build
cmake ../json && make -j8
sudo make install
```

### How to use it

#### Server
The server must be run on a machine that provides Lustre's cmd-line tool ``lctl`` and ``lfs``. Currently, it also must be able to ssh into the machines running Lustre's OSS servers without any password. This very likely will change in the future.

```bash
./server -ost_limits <ost_limits_file>
```

The ost_limits_file defines the maximum MB/s of each OST in Lustre. It is defined in the JSON format. An example would be:
```
cat ost_limits_file
[
  { "name": "0", "lustre_name": "toto-OST0000_UUID", "max_mbs": 100},
  { "name": "1", "lustre_name": "toto-OST0001_UUID", "max_mbs": 100}
]
```
The ``lustre_name`` is the uuid of the OST.

#### Client
The client can run on any machine. It only must be able to communicate to the server.
The client provides several subcommands:
 * ```./client ls``` : Enlist all resource reservations
 * ```./client reserve```  : Reserve resources
 * ```./client remove``` : Remove a given reservation
 
An example reservation call for 10 MB/s for 20 seconds (counting from now) for the file /mnt/lustre/intput.txt with a reservation ID of 1 is:
```bash
./client reserve -throughput 10 -duration 20 -filenames /mnt/lustre/intput.txt -id 1
```
