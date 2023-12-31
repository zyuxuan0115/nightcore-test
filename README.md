# Build [nightcore](https://github.com/zyuxuan0115/nightcore) on cloudlab
### preparation of cloudlab and nightcore
- the cloudlab profile I used is [vhive-ubuntu20](https://www.cloudlab.us/instantiate.php?project=faas-sched&profile=vhive-ubuntu20)
- the cloudlab machine I used is `Cloud-Clemson c6420` 
- [github page](https://github.com/ut-osa/nightcore/tree/asplos-release) of nightcore
  + `git clone --recurse-submodules https://github.com/zyuxuan0115/nightcore.git`
  + need to install [a newer version of cmake](https://cmake.org/download/)
  + `./build_deps.sh`
  + before compilation, might need to comment out `nightcore/src/common/stat.h`'s line #236
  + `make -j`
- The command to run `Foo` and `Bar`
  + `curl -X POST -d "Hello" http://<manager's IP>:8080/function/Foo`

### upgrade docker
- This [page](https://docs.docker.com/engine/install/ubuntu/#install-from-a-package)
- docker version should be higher or equal to than 19.03
- some useful docker command
  + `sudo docker run -it <container_name> bash`
  + `sudo docker exec -it <container_name> bash`
  + `sudo docker login --username=zyuxuan0115`
  + `sudo docker ps -a`
  + `sudo docker image ls`
  + `sudo docker image rm <IMAGE ID>`
  + `sudo docker system prune` -- in case the space is used up by docker build
      * sometimes need `sudo systemctl restart docker.service` after `sudo docker system prune`
  + `sudo docker logs -f <container_id>` -- see the output from a docker container
  + to check why a container is created but never be successfully setup
      * `sudo docker inspect <container_id>`

### other resources that might be useful
- [gcc-10](http://www.netgull.com/gcc/releases/gcc-10.1.0/)
  + gcc does not need to be installed manually
  + `./contrib/download_prerequisites`
  + `./configure --prefix=/proj/zyuxuanssf-PG0/gcc-10`
    * after building gcc-10, `export CXX=/proj/zyuxuanssf-PG0/gcc-10`
- [Python 3.12](https://devguide.python.org/getting-started/setup-building/#build-dependencies) (for nightcore-benchmark's script)
    * python3 does not need to be installed manually

