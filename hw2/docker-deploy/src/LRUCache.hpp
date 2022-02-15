#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "responsehead.hpp"

using namespace std;

struct DLinkedNode {
  string url;
  ResponseHead response;
  DLinkedNode* prev;
  DLinkedNode* next;
  DLinkedNode()
      : url(""), response(ResponseHead()), prev(nullptr), next(nullptr) {}
  DLinkedNode(string _key, ResponseHead _value)
      : url(_key), response(_value), prev(nullptr), next(nullptr) {}
};

class LRUCache {
 private:
  unordered_map<string, DLinkedNode*> cache;
  DLinkedNode* head;
  DLinkedNode* tail;
  int size;
  int capacity;

 public:
  LRUCache(int _capacity) : capacity(_capacity), size(0) {
    // 使用伪头部和伪尾部节点
    head = new DLinkedNode();
    tail = new DLinkedNode();
    head->next = tail;
    tail->prev = head;
  }

  bool inCache(string& key) {
    if (!cache.count(key)) {
      return false;
    }
    return true;
  }

  ResponseHead get(string& key) {
    DLinkedNode* node = cache[key];
    moveToHead(node);
    return node->response;
  }

  void put(string& key, ResponseHead value) {
    if (!cache.count(key)) {
      // 如果 key 不存在，创建一个新的节点
      DLinkedNode* node = new DLinkedNode(key, value);
      // 添加进哈希表
      cache[key] = node;
      // 添加至双向链表的头部
      addToHead(node);
      ++size;
      if (size > capacity) {
        // 如果超出容量，删除双向链表的尾部节点
        DLinkedNode* removed = removeTail();
        // 删除哈希表中对应的项
        cache.erase(removed->url);
        cout << "_________evict " << removed->url << " out of cache_______________!" << endl;
        // 防止内存泄漏
        delete removed;
        --size;
      }
    } else {
      // 如果 key 存在，先通过哈希表定位，再修改 value，并移到头部
      DLinkedNode* node = cache[key];
      node->response = value;
      moveToHead(node);
    }
  }

  void addToHead(DLinkedNode* node) {
    node->prev = head;
    node->next = head->next;
    head->next->prev = node;
    head->next = node;
  }

  void removeNode(DLinkedNode* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }

  void moveToHead(DLinkedNode* node) {
    removeNode(node);
    addToHead(node);
  }

  DLinkedNode* removeTail() {
    DLinkedNode* node = tail->prev;
    removeNode(node);
    return node;
  }
};
