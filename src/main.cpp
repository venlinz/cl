// Create MyString - inherit publically and add methods that I like ;) .
#include <fstream>
#include <iostream>
#include <list>
#include <string>

#include <cassert>
#include <cstdlib>

#define MAX_KEYWORD_LEN 32
#define TEST_PROGRAM "./examples/test.cl"

enum Operations {
    OP_UNKNOWN,
    PUSH,
    PLUS,
    MINUS,
    DUMP,
    OP_CNT,
};

class Operation {
    private:
    Operations m_op;
    int m_line;
    int m_col;

    public:
    Operation(Operations op, int line_no, int col_no)
        : m_op(op), m_line(line_no), m_col(col_no)
    { }
    int line() const {
        return m_line;
    }

    Operations op() const {
        return m_op;
    }

    int col() const {
        return m_col;
    }
};

std::list<Operation> parse_program(std::string program_file_name);
bool parse_op_from_line(Operation &op, std::string line);
std::string strip_front_str(std::string str);

int main(void)
{
    std::string a = "   abc";
    std::cout << a << '\n';
    a = strip_front_str(a);
    std::cout << a << '\n';
    std::cout << a.size() << '\n';

    assert(a.size() == 3);
    return 0;
}

int main1(int argc, char **argv)
{
    std::list<Operation> operations = parse_program(TEST_PROGRAM);
    std::cout << "Iterating through lines:\n";
    for (auto it = operations.begin(); it != operations.end(); ++it)
    {
        std::cout << (*it).line() << '\n';
    }
    std::cout << '\n';
    return 0;
}


std::list<Operation> parse_program(std::string program_file_name)
{
    std::ifstream input_program_file;
    input_program_file.open(program_file_name);

    if (!input_program_file) {
        std::cerr << "ERROR: Could not read file: " << TEST_PROGRAM
            << '\n';
        exit(EXIT_FAILURE);
    }

    char op[MAX_KEYWORD_LEN];

    std::list<Operation> operations;
    int line_num = 0;
    for (std::string line; std::getline(input_program_file, line);)
    {
        ++line_num;
        std::cout << line << '\n';
        /* Operation op = Operation(0, 0, 0); */
        /* while (parse_op_from_line(&op, line)) { */
        /*     operations.push_back(op); */
        /* } */
    }
    input_program_file.close();
    std::cout << "END\n";

    return operations;
}


/* bool parse_op_from_line(Operation &op, std::string line) */
/* { */
/*     if (line.empty()) */
/*         return false; */
/* } */


std::string strip_front_str(std::string str)
{
    if (str.empty())
        return "";

    if (str[0] == ' ')
    {
        auto idx = 0;
        for (; idx < str.size(); ++idx)
        {
            if (!isspace(str[idx]))
            {
                break;
            }
        }
        str.erase(0, idx);
        std::cout << idx << '\n';
    }
    return str;
}
