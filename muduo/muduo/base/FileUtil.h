// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#pragma once

#include <sys/types.h>  // for off_t

#include "muduo/base/StringPiece.h"
#include "muduo/base/noncopyable.h"

namespace muduo {
namespace FileUtil {

// read small file < 64KB
class ReadSmallFile : noncopyable {
 public:
  explicit ReadSmallFile(StringArg filename);
  ~ReadSmallFile();

 public:
  // return errno
  template <typename String>
  int readToString(int maxSize, String* content, int64_t* fileSize, int64_t* modifyTime, int64_t* createTime);

  /// Read at maximum kBufferSize into buf_
  // return errno
  int readToBuffer(int* size);

  const char* buffer() const {
    return buf_;
  }

  static const int kBufferSize = 64 * 1024;

 private:
  int fd_;
  int err_;
  char buf_[kBufferSize];
};

// read the file content, returns errno if error happens.
template <typename String>
int readFile(StringArg filename, int maxSize, String* content, int64_t* fileSize = NULL, int64_t* modifyTime = NULL,
             int64_t* createTime = NULL) {
  ReadSmallFile file(filename);
  return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

/**
 * @brief 封装文件指针, 可以将 char* 指向的数据写入文件, 同时有一个 64 KB 的缓冲区
 *
 * @note 非线程安全
 */
class AppendFile : noncopyable {
 public:
  explicit AppendFile(StringArg filename);

  ~AppendFile();

 public:
  /**
   * @brief 向 buffer_ 中添加日志
   *
   * @param logline
   * @param len
   */
  void append(const char* logline, size_t len);
  /**
   * @brief 将 buffer_ 写入到文件中
   *
   */
  void flush();

  off_t writtenBytes() const {
    return writtenBytes_;
  }

 private:
  size_t write(const char* logline, size_t len);

 private:
  FILE* fp_;                // 文件指针
  char buffer_[64 * 1024];  // 64 KB
  off_t writtenBytes_;
};

}  // namespace FileUtil
}  // namespace muduo
