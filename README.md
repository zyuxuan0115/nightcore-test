### setup ssh key for all 4 nodes
in `~/.ssh/authorized_keys`, add `id_ed25519.pub` from other machines

### change the host name of the manager machine
in `/etc/hosts` add `127.0.0.1 nightcore-manager`

### upgrade docker
- This [page](https://docs.docker.com/engine/install/ubuntu/#install-from-a-package)
- docker version should be higher than 19.02
