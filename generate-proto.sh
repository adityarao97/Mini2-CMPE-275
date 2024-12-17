#!/bin/bash

base=$(pwd)

protoc --cpp_out ${base}/generated-src --grpc_out ${base}/generated-src \
       -I ${base}/message \
       --plugin=protoc-gen-grpc=/opt/homebrew/Cellar/grpc/1.68.0/bin/grpc_cpp_plugin \
       ${base}/message/reader.proto
