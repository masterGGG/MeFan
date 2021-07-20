#########################################################################
# File Name: make.sh
# Author: ian
# mail: ian@taomee.com
# Created Time: Fri 17 May 2019 12:55:27 AM CST
#########################################################################
#!/bin/bash

SRC_DIR=./
DST_DIR=./

protoc -I=./ --python_out=./python ./protoFeedid.proto
protoc -I=./ --cpp_out=./c++ ./protoFeedid.proto
php ~/github/php-protobuf/protoc-gen-php.php -o ./php protoFeedid.proto
#g++ -g -o xxx client.cpp protoFeedid.pb.cc `pkg-config --libs --cflags protobuf`
