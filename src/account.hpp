#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include "blocklist.hpp"
struct AccountData { char user_id[31], password[31], username[31]; int privilege; AccountData() : privilege(0) { user_id[0] = '\0'; password[0] = '\0'; username[0] = '\0'; } AccountData(const std::string& uid, const std::string& pwd, const std::string& uname, int priv) : privilege(priv) { strncpy(user_id, uid.c_str(), 30); user_id[30] = '\0'; strncpy(password, pwd.c_str(), 30); password[30] = '\0'; strncpy(username, uname.c_str(), 30); username[30] = '\0'; } };
struct LoginSession { char user_id[31], selected_isbn[21]; int privilege; LoginSession() : privilege(0) { user_id[0] = '\0'; selected_isbn[0] = '\0'; } LoginSession(const std::string& uid, int priv) : privilege(priv) { strncpy(user_id, uid.c_str(), 30); user_id[30] = '\0'; selected_isbn[0] = '\0'; } };
class AccountSystem {
private:
    BlockLinkedList<int> user_index; std::string data_file; mutable std::fstream file; std::vector<LoginSession> login_stack; int record_count; bool initialized;
    void openFile() { if (!file.is_open()) { file.open(data_file, std::ios::in | std::ios::out | std::ios::binary); if (!file) { file.open(data_file, std::ios::out | std::ios::binary); file.close(); file.open(data_file, std::ios::in | std::ios::out | std::ios::binary); } if (!initialized) { file.seekg(0, std::ios::end); record_count = file.tellg() / sizeof(AccountData); initialized = true; } } }
    void writeRecord(int pos, const AccountData& acc) { openFile(); file.seekp(pos * sizeof(AccountData)); file.write(reinterpret_cast<const char*>(&acc), sizeof(AccountData)); file.flush(); if (pos >= record_count) record_count = pos + 1; }
    AccountData readRecord(int pos) { openFile(); AccountData acc; file.seekg(pos * sizeof(AccountData)); file.read(reinterpret_cast<char*>(&acc), sizeof(AccountData)); return acc; }
    bool isValidUserId(const std::string& id) { if (id.empty() || id.size() >= 31) return false; for (char c : id) if (!std::isalnum(c) && c != '_') return false; return true; }
    bool isValidPassword(const std::string& pwd) { if (pwd.empty() || pwd.size() >= 31) return false; for (char c : pwd) if (!std::isalnum(c) && c != '_') return false; return true; }
    bool isValidUsername(const std::string& name) { if (name.size() >= 31) return false; for (char c : name) if (c < 32 || c > 126) return false; return true; }
public:
    AccountSystem() : user_index("account_index.dat"), data_file("accounts.dat"), record_count(0), initialized(false) {}
    void init() { int pos; if (!user_index.find("root", pos)) { AccountData root("root", "sjtu", "root", 7); openFile(); pos = record_count++; writeRecord(pos, root); user_index.insert("root", pos); } }
    int getCurrentPrivilege() const { return login_stack.empty() ? 0 : login_stack.back().privilege; }
    std::string getCurrentUserId() const { return login_stack.empty() ? "" : login_stack.back().user_id; }
    bool isLoggedIn() const { return !login_stack.empty(); }
    bool su(const std::string& user_id, const std::string& password = "") { int pos; if (!user_index.find(user_id, pos)) return false; AccountData acc = readRecord(pos); if (!password.empty()) { if (std::string(acc.password) != password) return false; } else { if (getCurrentPrivilege() <= acc.privilege) return false; } login_stack.push_back(LoginSession(user_id, acc.privilege)); return true; }
    bool logout() { if (login_stack.empty()) return false; login_stack.pop_back(); return true; }
    bool registerUser(const std::string& user_id, const std::string& password, const std::string& username) { if (!isValidUserId(user_id) || !isValidPassword(password) || !isValidUsername(username)) return false; int pos; if (user_index.find(user_id, pos)) return false; AccountData acc(user_id, password, username, 1); openFile(); pos = record_count++; writeRecord(pos, acc); user_index.insert(user_id, pos); return true; }
    bool useradd(const std::string& user_id, const std::string& password, int privilege, const std::string& username) { if (!isValidUserId(user_id) || !isValidPassword(password) || !isValidUsername(username)) return false; if (privilege != 1 && privilege != 3 && privilege != 7) return false; if (getCurrentPrivilege() <= privilege) return false; int pos; if (user_index.find(user_id, pos)) return false; AccountData acc(user_id, password, username, privilege); openFile(); pos = record_count++; writeRecord(pos, acc); user_index.insert(user_id, pos); return true; }
    bool passwd(const std::string& user_id, const std::string& new_password, const std::string& old_password = "") { int pos; if (!user_index.find(user_id, pos)) return false; AccountData acc = readRecord(pos); if (getCurrentPrivilege() != 7) { if (old_password.empty() || std::string(acc.password) != old_password) return false; } if (!isValidPassword(new_password)) return false; strncpy(acc.password, new_password.c_str(), 30); acc.password[30] = '\0'; writeRecord(pos, acc); return true; }
    bool deleteUser(const std::string& user_id) { int pos; if (!user_index.find(user_id, pos)) return false; for (const auto& s : login_stack) if (std::string(s.user_id) == user_id) return false; user_index.erase(user_id); return true; }
    bool selectBook(const std::string& isbn) { if (login_stack.empty()) return false; strncpy(login_stack.back().selected_isbn, isbn.c_str(), 20); login_stack.back().selected_isbn[20] = '\0'; return true; }
    std::string getSelectedBook() const { return login_stack.empty() ? "" : login_stack.back().selected_isbn; }
};
#endif
