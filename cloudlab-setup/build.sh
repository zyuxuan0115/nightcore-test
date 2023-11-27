cp bashrc ~/.bashrc && source ~/.bashrc
rm -rf /proj/zyuxuanssf-PG0/docker
mkdir /proj/zyuxuanssf-PG0/docker && cd /proj/zyuxuanssf-PG0/docker
wget https://download.docker.com/linux/ubuntu/dists/focal/pool/stable/amd64/containerd.io_1.6.9-1_amd64.deb
wget https://download.docker.com/linux/ubuntu/dists/focal/pool/stable/amd64/docker-ce-cli_20.10.9~3-0~ubuntu-focal_amd64.deb
wget https://download.docker.com/linux/ubuntu/dists/focal/pool/stable/amd64/docker-ce_20.10.9~3-0~ubuntu-focal_amd64.deb
wget https://download.docker.com/linux/ubuntu/dists/focal/pool/stable/amd64/docker-scan-plugin_0.9.0~ubuntu-focal_amd64.deb
sudo dpkg -i containerd.io_1.6.9-1_amd64.deb
sudo dpkg -i docker-ce-cli_20.10.9~3-0~ubuntu-focal_amd64.deb
sudo dpkg -i docker-ce_20.10.9~3-0~ubuntu-focal_amd64.deb
sudo dpkg -i docker-scan-plugin_0.9.0~ubuntu-focal_amd64.deb
