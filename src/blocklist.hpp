#ifndef BLOCKLIST_HPP
#define BLOCKLIST_HPP
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
template<typename T>
class BlockLinkedList {
private:
    std::string filename;
    struct Node { char key[65]; int next; T value; Node() : next(-1) { key[0] = '\0'; } Node(const std::string& k, const T& v) : next(-1), value(v) { strncpy(key, k.c_str(), 64); key[64] = '\0'; } };
    struct Block { int count; int next_block; Node nodes[40]; Block() : count(0), next_block(-1) {} };
    mutable std::fstream file; int block_count;
    void openFile() { if (!file.is_open()) { file.open(filename, std::ios::in | std::ios::out | std::ios::binary); if (!file) { file.open(filename, std::ios::out | std::ios::binary); file.close(); file.open(filename, std::ios::in | std::ios::out | std::ios::binary); block_count = 0; writeHeader(); } else { file.seekg(0); file.read(reinterpret_cast<char*>(&block_count), sizeof(int)); } } }
    void writeHeader() { file.seekp(0); file.write(reinterpret_cast<char*>(&block_count), sizeof(int)); }
    Block readBlock(int pos) { Block b; file.seekg(sizeof(int) + pos * sizeof(Block)); file.read(reinterpret_cast<char*>(&b), sizeof(Block)); return b; }
    void writeBlock(int pos, const Block& b) { file.seekp(sizeof(int) + pos * sizeof(Block)); file.write(reinterpret_cast<const char*>(&b), sizeof(Block)); }
    int allocBlock() { int pos = block_count++; writeHeader(); Block b; writeBlock(pos, b); return pos; }
public:
    BlockLinkedList(const std::string& fname) : filename(fname), block_count(0) {}
    ~BlockLinkedList() { if (file.is_open()) file.close(); }
    void insert(const std::string& key, const T& value) {
        openFile();
        if (block_count == 0) { int pos = allocBlock(); Block b; b.nodes[0] = Node(key, value); b.count = 1; writeBlock(pos, b); return; }
        Block curr = readBlock(0); int curr_pos = 0;
        while (true) {
            int insert_pos = 0;
            while (insert_pos < curr.count && std::string(curr.nodes[insert_pos].key) < key) insert_pos++;
            if (insert_pos < curr.count && std::string(curr.nodes[insert_pos].key) == key) { curr.nodes[insert_pos].value = value; writeBlock(curr_pos, curr); return; }
            if (curr.count < 40) { for (int i = curr.count; i > insert_pos; i--) curr.nodes[i] = curr.nodes[i - 1]; curr.nodes[insert_pos] = Node(key, value); curr.count++; writeBlock(curr_pos, curr); return; }
            if (curr.next_block == -1) {
                int new_pos = allocBlock(); Block new_block; int mid = curr.count / 2;
                for (int i = mid; i < curr.count; i++) new_block.nodes[i - mid] = curr.nodes[i];
                new_block.count = curr.count - mid; curr.count = mid; curr.next_block = new_pos;
                if (insert_pos < mid) { for (int i = curr.count; i > insert_pos; i--) curr.nodes[i] = curr.nodes[i - 1]; curr.nodes[insert_pos] = Node(key, value); curr.count++; }
                else { int ni = insert_pos - mid; for (int i = new_block.count; i > ni; i--) new_block.nodes[i] = new_block.nodes[i - 1]; new_block.nodes[ni] = Node(key, value); new_block.count++; }
                writeBlock(curr_pos, curr); writeBlock(new_pos, new_block); return;
            }
            curr_pos = curr.next_block; curr = readBlock(curr_pos);
        }
    }
    bool find(const std::string& key, T& value) { openFile(); if (block_count == 0) return false; Block curr = readBlock(0); while (true) { for (int i = 0; i < curr.count; i++) if (curr.nodes[i].key == key) { value = curr.nodes[i].value; return true; } if (curr.next_block == -1) return false; curr = readBlock(curr.next_block); } }
    bool erase(const std::string& key) { openFile(); if (block_count == 0) return false; int curr_pos = 0; Block curr = readBlock(0); while (true) { for (int i = 0; i < curr.count; i++) if (curr.nodes[i].key == key) { for (int j = i; j < curr.count - 1; j++) curr.nodes[j] = curr.nodes[j + 1]; curr.count--; writeBlock(curr_pos, curr); return true; } if (curr.next_block == -1) return false; curr_pos = curr.next_block; curr = readBlock(curr_pos); } }
    std::vector<T> findAll(const std::string& key) { std::vector<T> result; openFile(); if (block_count == 0) return result; Block curr = readBlock(0); while (true) { for (int i = 0; i < curr.count; i++) if (curr.nodes[i].key == key) result.push_back(curr.nodes[i].value); if (curr.next_block == -1) break; curr = readBlock(curr.next_block); } return result; }
    std::vector<std::pair<std::string, T>> getAll() { std::vector<std::pair<std::string, T>> result; openFile(); if (block_count == 0) return result; Block curr = readBlock(0); while (true) { for (int i = 0; i < curr.count; i++) result.push_back({curr.nodes[i].key, curr.nodes[i].value}); if (curr.next_block == -1) break; curr = readBlock(curr.next_block); } return result; }
    void clear() { if (file.is_open()) file.close(); std::remove(filename.c_str()); block_count = 0; }
};
#endif
