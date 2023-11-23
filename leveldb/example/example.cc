#include <iostream>
#include <string>

#include "leveldb/db.h"

int main() {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
  std::cout << "status:" << status.ok() << std::endl;
  if (!status.ok()) {
    return 1;
  }
  std::string key = "apple";
  std::string value = "A";
  std::string get;

  leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);

  if (s.ok()) s = db->Get(leveldb::ReadOptions(), key, &get);
  if (s.ok())
    std::cout << "key:" << key << "\nget value:" << get << std::endl;
  else
    std::cout << "error occurred!" << std::endl;

  delete db;

  return 0;
}
