### setup ssh key for all 4 nodes
in `~/.ssh/authorized_keys`, add `id_ed25519.pub` from other machines

### change the host name of the manager machine
in `/etc/hosts` add `127.0.0.1 nightcore-manager`

### upgrade docker
- This [page](https://docs.docker.com/engine/install/ubuntu/#install-from-a-package)
- docker version should be higher or equal to than 19.03
- some useful docker command
  + `sudo docker run -it <container_name> bash`
  + `sudo docker ps -a`
  + `sudo docker system prune` -- in case the space is used up by docker build
  + `sudo docker logs -f` -- see the output from a docker container

