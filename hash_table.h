#pragma once
#include <iostream>
#include <string>
#include <cmath>
#include "constants.h"

using namespace std;

template <typename T>
class HashNode {
public:
    string key;
    T value;
    HashNode* next;

    HashNode(string key, T value) {
        this->key = key;
        this->value = value;
        next = nullptr;
    }
};

template <typename T>
class HashTable {
private:
    HashNode<T>* table[HASH_TABLE_SIZE];

    int hashFunction(string key) {
        int hash = 0;
        for (int i = 0; i < key.length(); i++) {
            hash = (hash * 31 + key[i]) % HASH_TABLE_SIZE;
        }
        return abs(hash);
    }

public:
    HashTable() {
        for (int i = 0; i < HASH_TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }

    void insert(string key, T value) {
        int index = hashFunction(key);
        HashNode<T>* current = table[index];

        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        HashNode<T>* newNode = new HashNode<T>(key, value);
        newNode->next = table[index];
        table[index] = newNode;
    }

    T* get(string key) {
        int index = hashFunction(key);
        HashNode<T>* current = table[index];

        while (current != nullptr) {
            if (current->key == key) {
                return &(current->value);
            }
            current = current->next;
        }
        return nullptr;
    }

    void remove(string key) {
        int index = hashFunction(key);
        HashNode<T>* current = table[index];
        HashNode<T>* prev = nullptr;

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    table[index] = current->next;
                }
                else {
                    prev->next = current->next;
                }
                delete current;
                return;
            }
            prev = current;
            current = current->next;
        }
    }

    void getAllItems(T items[], int& count) {
        count = 0;
        for (int i = 0; i < HASH_TABLE_SIZE; i++) {
            HashNode<T>* current = table[i];
            while (current != nullptr && count < MAX_SATELLITES) {
                items[count++] = current->value;
                current = current->next;
            }
        }
    }
};
