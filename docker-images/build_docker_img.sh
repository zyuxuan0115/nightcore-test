#!/bin/bash

# before you run this script, you need to run the 
# following command
# docker login -u "myusername" -p "mypassword" docker.io

ROOT_DIR=$(pwd)

docker build -t zyuxuan0115/nightcore:test \
       -f Dockerfile.nightcore \
       .

docker push zyuxuan0115/nightcore:test

docker build -t zyuxuan0115/nightcore:test-foo \
       -f Dockerfile.foo \
       .

docker push zyuxuan0115/nightcore:test-foo

docker build -t zyuxuan0115/nightcore:test-bar \
       -f Dockerfile.bar \
       .

docker push zyuxuan0115/nightcore:test-bar


