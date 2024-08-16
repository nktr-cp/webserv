#pragma once
#include <vector>

template<typename T>
class TrieNode {
    private:
    std::vector<TrieNode<T> *> children = std::vector<TrieNode<T> *>(26, NULL);
    T value;
    bool is_end = false;

    public:
    TrieNode();
    TrieNode(const TrieNode &src);
    TrieNode &operator=(const TrieNode &src);
    ~TrieNode();

    void insert(const char *key, T value);
    T search(const char *key) const;
    T search(const char *key, char end) const;
};
