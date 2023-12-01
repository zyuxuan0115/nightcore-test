- before running the workload, need to setup machines
  + the script to setup the machines is [here](https://github.com/zyuxuan0115/cloudlab-setup)
  + after running this script for the first time, you also need to ssh from all servers to all other servers, in order to add other server's to the ~/.ssh/known_hosts 
- command to run the workload
  + `./run_once.sh qps1000 1000` or `./run_all`
