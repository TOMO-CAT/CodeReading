# 用法

## 环境搭建

### 1. Docker

```bash
bash scripts/docker.sh
```

### 2. Local

```bash
# xmake
curl -fsSL https://xmake.io/shget.text | bash
```

## 编译

```bash
# 编译所有子项目
xmake -a
xmake --all

# 生成 compile_commands
xmake project -k compile_commands
```
