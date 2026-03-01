#ifndef BOOK_HPP
#define BOOK_HPP
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include "blocklist.hpp"
struct BookData { char isbn[21], name[61], author[61], keywords[61]; double price; int stock; BookData() : price(0.0), stock(0) { isbn[0] = '\0'; name[0] = '\0'; author[0] = '\0'; keywords[0] = '\0'; } BookData(const std::string& isbn_str) : price(0.0), stock(0) { strncpy(isbn, isbn_str.c_str(), 20); isbn[20] = '\0'; name[0] = '\0'; author[0] = '\0'; keywords[0] = '\0'; } bool operator<(const BookData& other) const { return std::string(isbn) < std::string(other.isbn); } bool operator==(const BookData& other) const { return std::string(isbn) == std::string(other.isbn); } };
class BookSystem {
private:
    BlockLinkedList<int> isbn_index, name_index, author_index, keyword_index; std::string data_file; mutable std::fstream file; int record_count; bool initialized;
    void openFile() { if (!file.is_open()) { file.open(data_file, std::ios::in | std::ios::out | std::ios::binary); if (!file) { file.open(data_file, std::ios::out | std::ios::binary); file.close(); file.open(data_file, std::ios::in | std::ios::out | std::ios::binary); } if (!initialized) { file.seekg(0, std::ios::end); record_count = file.tellg() / sizeof(BookData); initialized = true; } } }
    void writeRecord(int pos, const BookData& book) { openFile(); file.seekp(pos * sizeof(BookData)); file.write(reinterpret_cast<const char*>(&book), sizeof(BookData)); file.flush(); if (pos >= record_count) record_count = pos + 1; }
    BookData readRecord(int pos) { openFile(); BookData book; file.seekg(pos * sizeof(BookData)); file.read(reinterpret_cast<char*>(&book), sizeof(BookData)); return book; }
    std::vector<std::string> splitKeywords(const std::string& kw) { std::vector<std::string> result; std::string cur; for (char c : kw) { if (c == '|') { if (!cur.empty()) { result.push_back(cur); cur.clear(); } } else cur += c; } if (!cur.empty()) result.push_back(cur); return result; }
    bool hasDuplicateKeywords(const std::string& kw) { auto kws = splitKeywords(kw); std::sort(kws.begin(), kws.end()); for (size_t i = 1; i < kws.size(); i++) if (kws[i] == kws[i-1]) return true; return false; }
public:
    BookSystem() : isbn_index("book_isbn.dat"), name_index("book_name.dat"), author_index("book_author.dat"), keyword_index("book_keyword.dat"), data_file("books.dat"), record_count(0), initialized(false) {}
    bool select(const std::string& isbn, int& out_pos) { if (isbn.empty() || isbn.size() >= 21) return false; for (char c : isbn) if (c < 32 || c > 126) return false; int pos; if (!isbn_index.find(isbn, pos)) { BookData book(isbn); openFile(); pos = record_count++; writeRecord(pos, book); isbn_index.insert(isbn, pos); } out_pos = pos; return true; }
    bool modify(int pos, const std::string& new_isbn, const std::string& new_name, const std::string& new_author, const std::string& new_keywords, double new_price, bool has_isbn, bool has_name, bool has_author, bool has_keywords, bool has_price) { BookData book = readRecord(pos); std::string old_isbn = book.isbn, old_name = book.name, old_author = book.author, old_keywords = book.keywords; if (has_isbn) { if (new_isbn.empty() || new_isbn.size() >= 21) return false; if (new_isbn == old_isbn) return false; int t; if (isbn_index.find(new_isbn, t)) return false; } if (has_keywords && !new_keywords.empty() && hasDuplicateKeywords(new_keywords)) return false; isbn_index.erase(old_isbn); if (!old_name.empty()) name_index.erase(old_name); if (!old_author.empty()) author_index.erase(old_author); if (!old_keywords.empty()) for (const auto& kw : splitKeywords(old_keywords)) keyword_index.erase(kw); if (has_isbn) { strncpy(book.isbn, new_isbn.c_str(), 20); book.isbn[20] = '\0'; } if (has_name) { strncpy(book.name, new_name.c_str(), 60); book.name[60] = '\0'; } if (has_author) { strncpy(book.author, new_author.c_str(), 60); book.author[60] = '\0'; } if (has_keywords) { strncpy(book.keywords, new_keywords.c_str(), 60); book.keywords[60] = '\0'; } if (has_price) book.price = new_price; isbn_index.insert(book.isbn, pos); if (book.name[0] != '\0') name_index.insert(book.name, pos); if (book.author[0] != '\0') author_index.insert(book.author, pos); if (book.keywords[0] != '\0') for (const auto& kw : splitKeywords(book.keywords)) keyword_index.insert(kw, pos); writeRecord(pos, book); return true; }
    bool import(int pos, int quantity, double total_cost) { if (quantity <= 0 || total_cost <= 0) return false; BookData book = readRecord(pos); book.stock += quantity; writeRecord(pos, book); return true; }
    bool buy(const std::string& isbn, int quantity, double& total_price, int& pos) { if (!isbn_index.find(isbn, pos)) return false; if (quantity <= 0) return false; BookData book = readRecord(pos); if (book.stock < quantity) return false; book.stock -= quantity; writeRecord(pos, book); total_price = book.price * quantity; return true; }
    std::vector<BookData> showAll() { std::vector<BookData> r; for (const auto& p : isbn_index.getAll()) r.push_back(readRecord(p.second)); std::sort(r.begin(), r.end()); return r; }
    std::vector<BookData> showByISBN(const std::string& isbn) { std::vector<BookData> r; int pos; if (isbn_index.find(isbn, pos)) r.push_back(readRecord(pos)); return r; }
    std::vector<BookData> showByName(const std::string& name) { std::vector<BookData> r; for (int pos : name_index.findAll(name)) r.push_back(readRecord(pos)); std::sort(r.begin(), r.end()); return r; }
    std::vector<BookData> showByAuthor(const std::string& author) { std::vector<BookData> r; for (int pos : author_index.findAll(author)) r.push_back(readRecord(pos)); std::sort(r.begin(), r.end()); return r; }
    std::vector<BookData> showByKeyword(const std::string& keyword) { std::vector<BookData> r; if (keyword.empty() || keyword.find('|') != std::string::npos) return r; for (int pos : keyword_index.findAll(keyword)) r.push_back(readRecord(pos)); std::sort(r.begin(), r.end()); r.erase(std::unique(r.begin(), r.end()), r.end()); return r; }
    std::string getISBN(int pos) { return readRecord(pos).isbn; }
};
#endif
