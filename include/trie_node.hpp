#pragma once
#include <cstddef>
#include <vector>

template <typename T>
class TrieNode {
 private:
  std::vector<TrieNode<T> *> children_;
  T value_;
  bool is_end_;

 public:
  TrieNode();
  TrieNode(const TrieNode &src);
  TrieNode &operator=(const TrieNode &src);
  ~TrieNode();

  void insert(const char *key, T value);
  T search(const char *key) const;
  T search(const char *key, char end) const;
};

#include "trie_node.tpp"