#include <iostream>
#include <string>
#include "account.hpp"
#include "book.hpp"
#include "log.hpp"
#include "command.hpp"
int main() {
    AccountSystem accounts; BookSystem books; LogSystem logs;
    accounts.init();
    CommandProcessor processor(accounts, books, logs);
    std::string line;
    while (std::getline(std::cin, line)) processor.processCommand(line);
    return 0;
}
