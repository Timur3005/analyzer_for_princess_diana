#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <algorithm>
#include <sstream>

struct ErrorInfo {
    std::string errorMessage;
    int line;
};

bool checkBrackets(const std::string& code, ErrorInfo& errorInfo) {
    std::stack<char> bracketStack;

    bool inString = false;

    for (size_t i = 0; i < code.size(); ++i) {
        if (code[i] == '"' && (i == 0 || code[i - 1] != '\\')) {
            // Переключаем флаг, если встречается кавычка, не являющаяся экранированной
            inString = !inString;
        }

        if (!inString) {
            if (code[i] == '(' || code[i] == '{' || code[i] == '[') {
                bracketStack.push(code[i]);
            } else if (code[i] == ')' || code[i] == '}' || code[i] == ']') {
                if (bracketStack.empty()) {
                    errorInfo.errorMessage = "Unmatched closing bracket";
                    errorInfo.line = 1 + std::count(code.begin(), code.begin() + i, '\n');
                    return false;
                } else {
                    char openBracket = bracketStack.top();
                    bracketStack.pop();
                    if ((openBracket == '(' && code[i] != ')') ||
                        (openBracket == '{' && code[i] != '}') ||
                        (openBracket == '[' && code[i] != ']')) {
                        errorInfo.errorMessage = "Mismatched brackets";
                        errorInfo.line = 1 + std::count(code.begin(), code.begin() + i, '\n');
                        return false;
                    }
                }
            }
        }
    }

    if (!bracketStack.empty()) {
        errorInfo.errorMessage = "Unmatched opening bracket";
        // В этом случае line будет не очень точным, так как нельзя определить точное место ошибки внутри строки
        errorInfo.line = 0;
        return false;
    }

    return true;
}



bool checkSemicolons(const std::string& code, ErrorInfo& errorInfo) {
    std::istringstream iss(code);
    std::string line;
    int lineNumber = 0;

    bool inString = false; // Флаг, указывающий, находится ли внутри строки
    bool inBlock = false;  // Флаг, указывающий, находится ли внутри блока кода

    while (std::getline(iss, line)) {
        lineNumber++;

        // Убираем пробелы и табуляции в конце строки
        size_t lastChar = line.find_last_not_of(" \t");
        if (lastChar != std::string::npos) {
            line = line.substr(0, lastChar + 1);
        }

        // Проверка точки с запятой, пропуская строки внутри строк
        size_t quotePos = line.find('"');
        while (quotePos != std::string::npos) {
            inString = !inString; // Переключаем флаг внутри строки
            quotePos = line.find('"', quotePos + 1); // Ищем следующую кавычку
        }

        // Проверка фигурных скобок
        size_t openBracePos = line.find('{');
        size_t closeBracePos = line.find('}');
        if (openBracePos != std::string::npos) {
            inBlock = true;
        }
        if (closeBracePos != std::string::npos) {
            inBlock = false;
        }

        // Проверка, что строка не пуста и не требуется точка с запятой
        if (!inString && !inBlock && !line.empty() && line.back() != ';' && line.back() != '}') {
            // Проверка наличия открывающей или закрывающей скобки в строке
            if (openBracePos == std::string::npos && closeBracePos == std::string::npos) {
                errorInfo.errorMessage = "Missing semicolon at the end of the line";
                errorInfo.line = lineNumber;
                return false;
            }
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_file.txt>\n";
        return 1;
    }

    std::ifstream file(argv[1]);

    //проверка успешности чтения файла
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << "\n";
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    ErrorInfo errorInfo;

    //вывод о проверки кода
    if (checkBrackets(code, errorInfo) && checkSemicolons(code, errorInfo)) {
        std::cout << "Code analysis passed successfully.\n";
    } else {
        std::cerr << "Code analysis failed: " << errorInfo.errorMessage << " at line " << errorInfo.line << "\n";
    }

    return 0;
}
