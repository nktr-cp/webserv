#include "trie_node.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "http_request.hpp"

template <typename T>
TrieNode<T>::TrieNode() : children_(26, NULL), is_end_(false) {}
template <typename T>
TrieNode<T>::TrieNode(const TrieNode &src) {
  *this = src;
}
template <typename T>
TrieNode<T> &TrieNode<T>::operator=(const TrieNode<T> &src) {
  if (this != &src) {
    this->children_ = src.children_;
    this->value_ = src.value_;
    this->is_end_ = src.is_end_;
  }
  return *this;
}
template <typename T>
TrieNode<T>::~TrieNode() {
  for (std::size_t i = 0; i < this->children_.size(); i++) {
    if (this->children_[i] != NULL) delete this->children_[i];
  }
}

template <typename T>
void TrieNode<T>::insert(const char *key, T value_) {
  TrieNode<T> *current = this;
  for (std::size_t i = 0; key[i] != '\0'; i++) {
    if (current->children_[key[i] - 'A'] == NULL)
      current->children_[key[i] - 'A'] = new TrieNode<T>();
    current = current->children_[key[i] - 'A'];
  }
  current->value_ = value_;
  current->is_end_ = true;
}

template <typename T>
T TrieNode<T>::search(const char *key) const {
  const TrieNode<T> *current = this;
  try {
    for (std::size_t i = 0; key[i] != '\0'; i++) {
      if (current->children_.at(key[i] - 'A') == NULL) return T();
      current = current->children_.at(key[i] - 'A');
    }
  } catch (const std::out_of_range &e) {
    return T();
  }
  return current->value_;
}

template <typename T>
T TrieNode<T>::search(const char *key, const char end) const {
  const TrieNode<T> *current = this;
  try {
    for (std::size_t i = 0; key[i] != '\0' && key[i] != end; i++) {
      if (current->children_.at(key[i] - 'A') == NULL) return T();
      current = current->children_.at(key[i] - 'A');
    }
  } catch (const std::out_of_range &e) {
    return T();
  }
  if (current->is_end_) return current->value_;
  return T();
}
