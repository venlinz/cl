#include <fstream>
#include <iostream>
#include <list>
#include <stack>
#include <string>

#include <cassert>
#include <cstdlib>
#include <cstdint>

#define MAX_KEYWORD_LEN 32
#define TEST_PROGRAM "./examples/test.cl"
#define MAX_STACK_SIZE 1024

enum Operations {
    PUSH,
    PLUS,
    MINUS,
    DUMP,
    OP_CNT,
};

#define OPS_IMPLEMENTED 4


class Operation {
    private:
        Operations m_op;
        int m_opr;
        int m_line;
        int m_col;

    public:
        Operation(Operations op, int line_no, int col_no)
            : m_op(op), m_line(line_no), m_col(col_no)
        { }
        int line() const {
            return m_line;
        }
        void line(int line) {
            m_line = line;
        }

        Operations op() const {
            return m_op;
        }
        void op(Operations op) {
            m_op = op;
        }

        int col() const {
            return m_col;
        }
        void col(int col) {
            m_col = col;
        }

        int opr() const {
            return m_opr;
        }
        void opr(int opr) {
            m_opr = opr;
        }
};

uint64_t opr_global = 0;

#define STR_OPT_COMPILE "c"
#define STR_OPT_SIMULATE "s"
#define STR_OPT_HELP "help"

#define OUTPUT_FILENAME "output"

std::list<Operation> parse_program(std::string program_file_name);
size_t parse_op_from_line(Operations *op, std::string &line);
size_t strip_front_str(std::string &str);
void simulate_program(std::list<Operation> operations_list);
void print_usage(std::string program);
void print_help();
void compile_program(std::string output_filename, std::list<Operation> operations_list);
void exec(const std::string cmd);


int main(int argc, char **argv) {

    // sanity check for execl with 0 arg
    if (argc < 1) {
        std::cerr << "ERROR: Invalid program name\n";
        exit(EXIT_FAILURE);
    }
    std::string compiler_program_name = argv[0];

    if (argc < 2) {
        std::cerr << "ERROR: Invalid subcommand\n";
        print_help();
    }

    // Subcommand
    std::string opt_command = argv[1];
    if (opt_command == STR_OPT_HELP) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    if (argc < 3) {
        std::cerr << "ERROR: Invalid number of arguments\n";
        print_usage(compiler_program_name);
        exit(EXIT_FAILURE);
    }

    std::string program_file_name = argv[2];
    std::list<Operation> operations = parse_program(program_file_name);

    std::string opt_command = argv[1];
    if (opt_command == STR_OPT_COMPILE) {
        compile_program(OUTPUT_FILENAME, operations);
        exit(0);
    }
    else if (opt_command == STR_OPT_SIMULATE) {
        simulate_program(operations);
        exit(0);
    }
    else {
        std::cerr << "ERROR: Invalid command\n";
        print_usage(compiler_program_name);
    }

    return 0;
}


std::list<Operation> parse_program(std::string program_file_name) {
    std::ifstream input_program_file;
    input_program_file.open(program_file_name);

    if (!input_program_file) {
        std::cerr << "ERROR: Could not read file: " << program_file_name
            << '\n';
        exit(EXIT_FAILURE);
    }

    char op[MAX_KEYWORD_LEN];

    std::list<Operation> operations_list;
    int line_num = 0;
    for (std::string line; std::getline(input_program_file, line);)
    {
        ++line_num;
        Operations op_type;
        size_t rel_col = 0;
        size_t col = 1;
        while ((rel_col = parse_op_from_line(&op_type, line))) {
            if (op_type >= Operations::OP_CNT) {
                std::cerr << "OP_CNT: " << line << '\n';
                exit(EXIT_FAILURE);
            }
            Operation op = Operation(op_type, line_num, 0);
            if (op_type == Operations::PUSH) {
                op.opr(opr_global);
                col += rel_col;
            }
            operations_list.push_back(op);
        }
    }
    input_program_file.close();

    return operations_list;
}


size_t parse_op_from_line(Operations *op, std::string &line) {
    if (line.empty())
        return 0;
    size_t striped_front = strip_front_str(line);

    char number_str[128];
    bool is_number = false;
    size_t idx = 0;
    if (isdigit(line.at(0))) {
        for (size_t i = 0; i < line.size() && isdigit(line.at(i)); ++i) {
            number_str[idx] = line.at(i);
            ++idx;
        }
        number_str[idx] = '\0';
        line.erase(0, idx);
        *op = Operations::PUSH;
        opr_global = atoi(number_str);


        return striped_front + idx;
    }

    // Check for whether implemented every operation in Operations
    assert(OPS_IMPLEMENTED == Operations::OP_CNT && "Implement every operation");
    switch (line.at(0)) {
        case '+':
            *op = Operations::PLUS;
            line.erase(0, 1);
            return striped_front + 1;
        case '-':
            *op = Operations::MINUS;
            line.erase(0, 1);
            return striped_front + 1;
        case '.':
            *op = Operations::DUMP;
            line.erase(0, 1);
            return striped_front + 1;
        default:
            std::cerr << "Unhandled function: " << line.at(0);

    }
    return 0;
}

size_t strip_front_str(std::string &str) {
    if (str.empty())
        return 0;

    size_t idx = 0;
    if (str[0] == ' ')
    {
        for (; idx < str.size(); ++idx)
        {
            if (!isspace(str[idx]))
            {
                break;
            }
        }
        str.erase(0, idx);
    }
    return idx;
}


void simulate_program(std::list<Operation> operations_list) {
    std::cout << "Simulating\n";
    std::stack<uint64_t> program_stack;
    for (auto it = operations_list.begin(); it != operations_list.end(); ++it)
    {
        // Check for whether implemented every operation in Operations
        assert(OPS_IMPLEMENTED == Operations::OP_CNT && "Implement every operation");
        switch (it->op()) {
            case Operations::PUSH:
                if (program_stack.size() >= MAX_STACK_SIZE) {
                    std::cerr << "Stack size exceeded limit\n";
                    exit(EXIT_FAILURE);
                }
                program_stack.push(it->opr());
                break;
            case Operations::PLUS:
                if (program_stack.size() >= 2) {
                    int a = program_stack.top();
                    program_stack.pop();
                    int b = program_stack.top();
                    program_stack.pop();
                    program_stack.push(a + b);
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "PLUS(+) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;
            case Operations::MINUS:
                if (program_stack.size() >= 2) {
                    int a = program_stack.top();
                    program_stack.pop();
                    int b = program_stack.top();
                    program_stack.pop();
                    program_stack.push(b - a);
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "MINUS(-) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;
            case Operations::DUMP:
                if (program_stack.size() >= 1) {
                    std::cout << program_stack.top() << '\n';
                    program_stack.pop();
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "DUMP operation\n";
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                std::cerr << "ERROR: Operation unknown\n";
                std::cerr << "op: " << it->op() << '\n';
                exit(EXIT_FAILURE);
        }
    }
}


void print_help() {
    print_usage("cl");
}


void print_usage(std::string program) {
    std::cout << "Usage: " << program << " option file\n";
    std::cout << "    options:\n";
    std::cout << "        c - compile\n";
    std::cout << "        s - simulate\n";
}


void compile_program(std::string output_filename, std::list<Operation> operations_list) {
    std::cout << "Compiling\n";

    int mock_stack_size = 0;

    std::ofstream out_file;
    out_file.open(output_filename + ".asm");

    out_file << "global _start\n";
    out_file << "segment .text\n";

    out_file << "dump:\n";
    out_file << "    push    rbp\n";
    out_file << "    mov     rbp, rsp\n";
    out_file << "    sub     rsp, 64\n";
    out_file << "    mov     QWORD [rbp-56], rdi\n";
    out_file << "    mov     DWORD [rbp-4], 1\n";
    out_file << "    mov     edx, DWORD [rbp-4]\n";
    out_file << "    mov     eax, 32\n";
    out_file << "    sub     rax, rdx\n";
    out_file << "    mov     BYTE [rbp-48+rax], 10\n";
    out_file << ".L2:\n";
    out_file << "    mov     rcx, QWORD [rbp-56]\n";
    out_file << "    mov     rdx, -3689348814741910323\n";
    out_file << "    mov     rax, rcx\n";
    out_file << "    mul     rdx\n";
    out_file << "    shr     rdx, 3\n";
    out_file << "    mov     rax, rdx\n";
    out_file << "    sal     rax, 2\n";
    out_file << "    add     rax, rdx\n";
    out_file << "    add     rax, rax\n";
    out_file << "    sub     rcx, rax\n";
    out_file << "    mov     rdx, rcx\n";
    out_file << "    mov     eax, edx\n";
    out_file << "    lea     ecx, [rax+48]\n";
    out_file << "    mov     edx, DWORD [rbp-4]\n";
    out_file << "    mov     eax, 31\n";
    out_file << "    sub     rax, rdx\n";
    out_file << "    mov     edx, ecx\n";
    out_file << "    mov     BYTE [rbp-48+rax], dl\n";
    out_file << "    add     DWORD [rbp-4], 1\n";
    out_file << "    mov     rax, QWORD [rbp-56]\n";
    out_file << "    mov     rdx, -3689348814741910323\n";
    out_file << "    mul     rdx\n";
    out_file << "    mov     rax, rdx\n";
    out_file << "    shr     rax, 3\n";
    out_file << "    mov     QWORD [rbp-56], rax\n";
    out_file << "    cmp     QWORD [rbp-56], 0\n";
    out_file << "    jne     .L2\n";
    out_file << "    mov     eax, DWORD [rbp-4]\n";
    out_file << "    mov     edx, DWORD [rbp-4]\n";
    out_file << "    mov     ecx, 32\n";
    out_file << "    sub     rcx, rdx\n";
    out_file << "    lea     rdx, [rbp-48]\n";
    out_file << "    add     rcx, rdx\n";
    out_file << "    mov     rdx, rax\n";
    out_file << "    mov     rsi, rcx\n";
    out_file << "    mov     edi, 1\n";
    out_file << "    mov     rax, 1\n";
    out_file << "    syscall\n";
    out_file << "    nop\n";
    out_file << "    leave\n";
    out_file << "    ret\n";

    out_file << "_start:\n";

    for (auto it = operations_list.begin(); it != operations_list.end(); ++it)
    {
        switch (it->op()) {
            case Operations::PUSH:
                if (mock_stack_size >= MAX_STACK_SIZE) {
                    std::cerr << "Stack size exceeded limit\n";
                    exit(EXIT_FAILURE);
                }
                out_file << "    ;; PUSH\n";
                out_file << "    push " << it->opr() << '\n';
                ++mock_stack_size;
                break;
            case Operations::PLUS:
                if (mock_stack_size >= 2) {
                    out_file << "    ;; ADD\n";
                    out_file << "    pop rdx\n";
                    out_file << "    pop rsi\n";
                    out_file << "    add rdx, rsi\n";
                    out_file << "    push rdx\n";
                    --mock_stack_size;
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "PLUS(+) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;
            case Operations::MINUS:
                if (mock_stack_size >= 2) {
                    out_file << "    ;; MINUS\n";
                    out_file << "    pop rdx\n";
                    out_file << "    pop rsi\n";
                    out_file << "    sub rsi, rdx\n";
                    out_file << "    push rsi\n";
                    --mock_stack_size;
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "MINUS(-) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;
            case Operations::DUMP:
                if (mock_stack_size >= 1) {
                    out_file << "    ;; DUMP\n";
                    out_file << "    pop rdi\n";
                    out_file << "    call dump\n";
                    --mock_stack_size;
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "DUMP operation\n";
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                std::cerr << "ERROR: Operation unknown\n";
                std::cerr << "op: " << it->op() << '\n';
                exit(EXIT_FAILURE);
        }
    }

    out_file << "    mov rax, 60\n";
    out_file << "    mov rdi, 0\n";
    out_file << "    syscall\n";
    out_file << "    ret\n";

    out_file.close();

    std::string nasm_cmd = "nasm -felf64 ";
    nasm_cmd += OUTPUT_FILENAME;
    nasm_cmd += ".asm -o";
    nasm_cmd += OUTPUT_FILENAME;
    nasm_cmd += ".o";
    exec(nasm_cmd);

    std::string ld_cmd = "ld ";
    ld_cmd += OUTPUT_FILENAME;
    ld_cmd += ".o -o ./a.out";
    exec(ld_cmd);
}


void exec(const std::string cmd) {
    std::cout << "Exec: " << cmd << '\n';
    std::system(cmd.c_str());
}
