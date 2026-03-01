#ifndef LOG_HPP
#define LOG_HPP
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <sstream>
struct FinanceRecord { double income, expenditure; char user_id[31], isbn[21], operation[20]; int quantity; FinanceRecord() : income(0), expenditure(0), quantity(0) { user_id[0] = '\0'; isbn[0] = '\0'; operation[0] = '\0'; } FinanceRecord(double inc, double exp, const std::string& uid, const std::string& is, int qty, const std::string& op) : income(inc), expenditure(exp), quantity(qty) { strncpy(user_id, uid.c_str(), 30); user_id[30] = '\0'; strncpy(isbn, is.c_str(), 20); isbn[20] = '\0'; strncpy(operation, op.c_str(), 19); operation[19] = '\0'; } };
class LogSystem {
private: std::string finance_file; mutable std::fstream fin_file; int finance_count;
    void openFinanceFile() { if (!fin_file.is_open()) { fin_file.open(finance_file, std::ios::in | std::ios::out | std::ios::binary); if (!fin_file) { fin_file.open(finance_file, std::ios::out | std::ios::binary); fin_file.close(); fin_file.open(finance_file, std::ios::in | std::ios::out | std::ios::binary); finance_count = 0; } } }
public:
    LogSystem() : finance_file("finance.dat"), finance_count(0) {}
    void addFinance(double income, double expenditure, const std::string& user_id, const std::string& isbn, int quantity, const std::string& op) { openFinanceFile(); FinanceRecord rec(income, expenditure, user_id, isbn, quantity, op); fin_file.seekp(finance_count * sizeof(FinanceRecord)); fin_file.write(reinterpret_cast<const char*>(&rec), sizeof(FinanceRecord)); finance_count++; }
    int getFinanceCount() { openFinanceFile(); return finance_count; }
    std::pair<double, double> showFinance(int count) { openFinanceFile(); if (count == 0) return {0, 0}; if (count > finance_count) return {-1, -1}; double ti = 0, te = 0; for (int i = finance_count - count; i < finance_count; i++) { FinanceRecord rec; fin_file.seekg(i * sizeof(FinanceRecord)); fin_file.read(reinterpret_cast<char*>(&rec), sizeof(FinanceRecord)); ti += rec.income; te += rec.expenditure; } return {ti, te}; }
    std::pair<double, double> showAllFinance() { return showFinance(finance_count); }
    std::string getLog() { openFinanceFile(); std::ostringstream oss; oss << std::fixed << std::setprecision(2); for (int i = 0; i < finance_count; i++) { FinanceRecord rec; fin_file.seekg(i * sizeof(FinanceRecord)); fin_file.read(reinterpret_cast<char*>(&rec), sizeof(FinanceRecord)); oss << rec.user_id << " " << rec.operation << " " << rec.isbn << " qty=" << rec.quantity << " income=" << rec.income << " expense=" << rec.expenditure << "\n"; } return oss.str(); }
};
#endif
