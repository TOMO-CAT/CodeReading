# README

## 依赖

> brpc 鼓励静态链接依赖，以便于每个运行 brpc 服务的机器不必再安装依赖。

brpc 有如下依赖：

* gflags
* protobuf
* leveldb

## 安装依赖

安装依赖：

```bash
sudo apt-get install -y git g++ make libssl-dev libgflags-dev libprotobuf-dev libprotoc-dev protobuf-compiler libleveldb-dev
```

如果你需要静态链接 leveldb：

```bash
sudo apt-get install -y libsnappy-dev
```

如果你需要在样例中启用 cpu/heap 的 profiler：

```bash
sudo apt-get install -y libgoogle-perftools-dev
```

如果你要运行测试，那么需要安装并编译 libgtest-dev（它没有被默认编译）：

```bash
sudo apt-get install -y cmake libgtest-dev && cd /usr/src/gtest && sudo cmake . && sudo make && sudo mv lib/libgtest* /usr/lib/ && cd -
```

gtest 源码目录可能变动，如果 /usr/src/gtest 不存在，请尝试 /usr/src/googletest/googletest。

## 编译

```bash
sh config_brpc.sh --headers=/usr/include --libs=/usr/lib
make
```

* 修改编译器为 clang，添加选项 `--cxx=clang++ --cc=clang`
* 不想链接调试符号，添加选项 `--nodebugsymbols`，然后编译将会得到更轻量的二进制文件
* 使用 glog 版的 brpc，添加选项 `--with-glog`
* 要启用 thrift 支持，首先安装 thrift 并且添加选项 `--with-thrift`

## 运行样例

```bash
cd example/echo_c++
make
./echo_server &
./echo_client
```

## Reference

[1] <https://brpc.incubator.apache.org/zh/docs/getting_started/>
