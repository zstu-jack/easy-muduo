protoc --proto_path=./protocal/ --cpp_out=./protocal/ ./protocal/*.proto

g++ -g3 -std=c++11 example_proto_server.cpp ./protocal/*.cc ./common/*.cpp -o server -lprotobuf