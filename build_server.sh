protoc --proto_path=./protocal/ --cpp_out=./protocal/ ./protocal/*.proto

g++ -g3 -std=c++11 server.cpp ./protocal/*.cc ./common/*.cpp -o server -lprotobuf