# 待续

* 完善 Dockerfile
  * 和本地 user 保持一致，不使用 root
  * 映射 .gitconfig 和 .ssh 相关信息
* 将 scripts 中的 docker_build.sh 和 docker_run.sh 合并成一个脚本
* xmake 的使用

## Dockerfile

### 1. 安装 xmake

```bash
wget https://xmake.io/shget.text -O - | bash
```
