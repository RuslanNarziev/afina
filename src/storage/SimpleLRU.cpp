#include "SimpleLRU.h"
#include <iostream>
#include <sstream>

namespace Afina {
namespace Backend {

void SimpleLRU::free_mem(size_t free_size) {
    while(_size > _max_size - free_size) {
        _size -= (_lru_end->key.size() + _lru_end->value.size());
        _lru_index.erase(_lru_end->key);
        _lru_end = _lru_end->prev;
        _lru_end->next.reset(nullptr); 
    }
    _size += free_size;
}

bool SimpleLRU::_add_node(const std::string& key, const std::string& value) {
    lru_node* cur_node = new lru_node{key, value, nullptr, nullptr};
    lru_node& node = *cur_node; 
    _lru_index.insert(std::pair<const std::string&, lru_node&>(cur_node->key, *cur_node));
    node.prev = nullptr;
    node.next = std::move(_lru_head);
    if(node.next.get() != nullptr)
        node.next.get()->prev = cur_node;
    _lru_head = std::move(std::unique_ptr<lru_node>(cur_node));
    free_mem(key.size() + value.size());
    node.value = value;
    return true;
}

bool SimpleLRU::_update_node(const std::string& value, lru_node& node) {
    if(node.prev != nullptr) {
        lru_node* cur_node = node.prev->next.get();
        if(cur_node == _lru_end)
            _lru_end = node.prev;
        node.prev->next.release();
        node.prev->next = std::move(node.next);
        if(node.prev->next.get() != nullptr)
            node.prev->next.get()->prev = node.prev;
        node.prev = nullptr;
        node.next = std::move(_lru_head);
        node.next.get()->prev = cur_node;
        _lru_head = std::move(std::unique_ptr<lru_node>(cur_node));
    }
    if(node.value.size() < value.size())
        free_mem(value.size() - node.value.size());
    else
        _size -= (node.value.size() - value.size());
    node.value = value;
    return true;
}

bool SimpleLRU::Put(const std::string &key, const std::string &value) { 
    if(key.size() + value.size() > _max_size)
        return false;
    auto search = _lru_index.find(key);
    if(search != _lru_index.end())
        return _update_node(value, search->second.get());
    return _add_node(key, value); 
}

bool SimpleLRU::Set(const std::string &key, const std::string &value) { 
    if(key.size() + value.size() > _max_size)
        return false;
    auto search = _lru_index.find(key);
    if(search != _lru_index.end())
        return _update_node(value, search->second.get());
    return false;
}

bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if(key.size() + value.size() > _max_size)
        return false;
    auto search = _lru_index.find(key);
    if(search != _lru_index.end())
        return false;
    return _add_node(key, value);
}

bool SimpleLRU::Delete(const std::string &key) {
    auto search = _lru_index.find(key);
    if(search != _lru_index.end()) {
        lru_node& node = search->second.get();
        _size -= key.size() + node.value.size();
        _lru_index.erase(key);
        if(node.prev != nullptr) {
            lru_node& prev = *(node.prev);
            lru_node* cur_node = prev.next.get();
            lru_node* next_node = cur_node->next.get();
            if(cur_node == _lru_end)
                _lru_end = node.prev;
            cur_node->next.release();
            prev.next.reset(next_node);
            if(prev.next.get() != nullptr)
                prev.next.get()->prev = cur_node;
        } else {
			lru_node* next_node = _lru_head.get()->next.get();
            _lru_head.get()->next.release();
            _lru_head.reset(next_node);
            if(_lru_head.get() != nullptr)
                _lru_head.get()->prev = nullptr;
            else
                _lru_end = nullptr;
        }
        return true;
    }
    return false;
}

bool SimpleLRU::Get(const std::string &key, std::string &value) {
    auto search = _lru_index.find(key);
    if(search != _lru_index.end()) {
        lru_node& node = search->second.get();
        value = node.value;
        return _update_node(value, node);
    }
    return false;
}


} // namespace Backend
} // namespace Afina

