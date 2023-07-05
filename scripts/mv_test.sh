#!/bin/bash

# 指定目录
source_dir=$1

# 遍历指定目录下的所有文件名匹配形式为"test_${file}.cpp"的文件
for file in "$source_dir"/test_*.cpp; do
  if [ -f "$file" ]; then
    # 提取${file}的值，即目标目录名
     target_dir="${file%/*}/$(basename "$file" .cpp | sed 's/^test_//')"
  
    # 检查目标目录是否存在，如果不存在则创建
    if [ ! -d "$target_dir" ]; then
      mkdir -p "$target_dir"
    fi
    
    # 移动文件到目标目录
    mv "$file" "$target_dir/"
  fi
done

