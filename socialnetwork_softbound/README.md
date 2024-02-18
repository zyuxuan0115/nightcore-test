# How to build the benchmark on your machine
- install C++ dependency

```bash
> ./microservice-deps-cpp.sh
```

- build the workload

```bash
> cd DeathStarBench/socialNetwork
> mkdir build && cd build
> cmake ..
> make
```

# How to run the benchmark in docker containers
- delete the `build` directory
- before running the workload, need to setup machines
  + the script to setup the machines is [here](https://github.com/zyuxuan0115/cloudlab-setup)
  + after running this script for the first time, you also need to ssh from all servers to all other servers, in order to add other server's to the ~/.ssh/known_hosts 
- command to run the workload
  + `./run_once.sh qps1000 1000` or `./run_all`