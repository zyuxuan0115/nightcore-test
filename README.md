### Build nightcore without any container
- [github page](https://github.com/ut-osa/nightcore/tree/asplos-release) of nightcore
  + `git clone --recurse-submodules git@github.com:zyuxuan0115/nightcore.git`
  + need to install
    * [the newer version of cmake (3.25)](https://cmake.org/download/)
    * [gcc-10](http://www.netgull.com/gcc/releases/gcc-10.1.0/)
      - `./contrib/download_prerequisites`
      - `./configure --prefix=/proj/zyuxuanssf-PG0/gcc-10`
      - after building gcc-10, `export CXX=/proj/zyuxuanssf-PG0/gcc-10`
    * [Python 3.12](https://devguide.python.org/getting-started/setup-building/#build-dependencies) (for nightcore-benchmark's script)
  + `./build_deps.sh`
  + Before compilation, need to comment out `nightcore/src/common/stat.h`'s line #236
  + `make -j`

### setup ssh key for all 4 nodes
in `~/.ssh/authorized_keys`, add `id_ed25519.pub` from other machines

### change the host name of the manager machine
- on manager node, in `/etc/hosts` add `127.0.0.1 nightcore-manager`
- on worker nodes, in `/etc/hosts` add `127.0.0.1 nightcore-worker-i`, where i is the index of workers

### upgrade docker
- This [page](https://docs.docker.com/engine/install/ubuntu/#install-from-a-package)
- docker version should be higher or equal to than 19.03
- some useful docker command
  + `sudo docker run -it <container_name> bash`
  + `sudo docker ps -a`
  + `sudo docker system prune` -- in case the space is used up by docker build
  + `sudo docker logs -f <container_id>` -- see the output from a docker container

