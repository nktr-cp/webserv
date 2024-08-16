#include "trie_node.hpp"
#include "http_request.hpp"

template<typename T>
TrieNode<T>::TrieNode() :
    children(26, NULL),
    is_end(false)
    {}
template<typename T>
TrieNode<T>::TrieNode(const TrieNode &src) {
    *this = src;
}
template<typename T>
TrieNode<T> &TrieNode<T>::operator=(const TrieNode<T> &src) {
    if (this != &src) {
        this->children = src.children;
        this->value = src.value;
        this->is_end = src.is_end;
    }
    return *this;
}
template<typename T>
TrieNode<T>::~TrieNode() {
    for (std::size_t i = 0; i < this->children.size(); i++) {
        if (this->children[i] != NULL)
            delete this->children[i];
    }
}

template<typename T>
void TrieNode<T>::insert(const char *key, T value) {
    TrieNode<T> *current = this;
    for (std::size_t i = 0; key[i] != '\0'; i++) {
        if (current->children[key[i] - 'A'] == NULL)
            current->children[key[i] - 'A'] = new TrieNode<T>();
        current = current->children[key[i] - 'A'];
    }
    current->value = value;
    current->is_end = true;
}

template<typename T>
T TrieNode<T>::search(const char *key) const {
    const TrieNode<T> *current = this;
    try {
        for (std::size_t i = 0; key[i] != '\0'; i++) {
            if (current->children.at(key[i] - 'A') == NULL)
                return T();
            current = current->children.at(key[i] - 'A');
        }
    }
    catch (const std::out_of_range &e) {
        return T();
    }
    return current->value;
}

template<typename T>
T TrieNode<T>::search(const char *key, const char end) const {
    const TrieNode<T> *current = this;
    try {
        for (std::size_t i = 0; key[i] != '\0' && key[i] != end; i++) {
            if (current->children.at(key[i] - 'A') == NULL)
                return T();
            current = current->children.at(key[i] - 'A');
        }
    }
    catch (const std::out_of_range& e) {
        return T();
    }
    if (current->is_end)
        return current->value;
    return T();
}

template class TrieNode<HttpMethod>;
