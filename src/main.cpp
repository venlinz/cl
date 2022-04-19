#include <fstream>
#include <iostream>
#include <list>
#include <stack>
#include <string>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#define MAX_KEYWORD_LEN 32
#define TEST_PROGRAM "./examples/test.cl"
#define MAX_STACK_SIZE 1024

#define STR_KEYWORD_END "end"
#define STR_KEYWORD_ELSE "else"

enum Operations {
    OP_PUSH,
    OP_PLUS,
    OP_MINUS,
    OP_DUMP,
    OP_EQUAL,
    OP_IF,
    OP_ELSE,
    OP_END,
    OP_CNT, // This value is treated as UNKNOWN OPERATION
};

#define OPS_IMPLEMENTED 4


class Operation {
    private:
        Operations m_op;
        uint64_t m_opr;
        int m_line;
        int m_col;

    public:
        Operation() {};
        Operation(Operations op_type, int line_no, int col_no)
            : m_op(op_type), m_line(line_no), m_col(col_no)
        { }
        int line() const {
            return m_line;
        }
        void line(int line) {
            m_line = line;
        }

        Operations op_type() const {
            return m_op;
        }
        void op_type(Operations op_type) {
            m_op = op_type;
        }

        int col() const {
            return m_col;
        }
        void col(int col) {
            m_col = col;
        }

        uint64_t operand() const {
            return m_opr;
        }
        void operand(uint64_t operand) {
            m_opr = operand;
        }
};


#define STR_OPT_COMPILE "c"
#define STR_OPT_SIMULATE "s"
#define STR_OPT_HELP "help"

#define OUTPUT_FILENAME "output"

[[nodiscard]] std::list<Operation> parse_program(std::string program_file_name);
[[nodiscard]] std::list<Operation> parse_op_from_line(std::string line);
[[maybe_unused]] size_t lstrip(std::string &str);
void simulate_program(std::list<Operation> operations_list);
void print_usage(std::string program);
void print_help();
void compile_program(std::string output_filename,
        std::list<Operation> operations_list);
void exec(const std::string cmd);
int generate_asm_for_if_else(std::ofstream& out_file,
        std::list<Operation>::iterator begin,
        std::list<Operation>::iterator end,
        uint64_t ip);
void print_error(const std::string& program_file_name, const int line_num,
        const int col, const std::string msg);


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

    // Two options: subcommand and file_path to compile
    std::string program_file_name = argv[2];
    std::list<Operation> operations = parse_program(program_file_name);

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


// parse the program file into list<Operation>.
[[nodiscard]] std::list<Operation> parse_program(std::string program_file_name) {

    int exit_code = EXIT_SUCCESS;
    std::ifstream input_program_file;
    input_program_file.open(program_file_name);

    if (!input_program_file) {
        std::cerr << "ERROR: Could not read file: " << program_file_name
            << '\n';
        exit(EXIT_FAILURE);
    }


    std::list<Operation> operations_list;
    int line_num = 0;
    for (std::string line; std::getline(input_program_file, line);)
    {
        ++line_num;
        // TODO: change this to lamda expressions for error reporting.
        std::list<Operation> ops_in_line = parse_op_from_line(line);
        if (ops_in_line.empty()) {
            continue;
        }
        for (auto op = ops_in_line.begin();
                op != ops_in_line.end();
                ++op) {
            if (op->op_type() >= Operations::OP_CNT) {
                exit_code = EXIT_FAILURE;
                print_error(program_file_name, line_num, op->col(), "Invalid operation");
            }
            op->line(line_num);
        }
        operations_list.splice(operations_list.end(), ops_in_line);
    }
    input_program_file.close();

    if (exit_code != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    return operations_list;
}


// TODO: change this function to return Option like in rust
// parse_op_from_line return list of Operations in a line
// and return empty list if no Operations is on line.
[[nodiscard]] std::list<Operation> parse_op_from_line(std::string line) {
    if (line.empty()) {
        return std::list<Operation> {};
    }

    std::list<Operation> line_ops = {};
    char number_str[32] = {0};
    for (uint32_t i = 0; i < line.size(); ++i) {

        Operation op;
        if (isdigit(line.at(i))) {
            int col_start = i + 1;
            int num_idx = 0;
            for (; isdigit(line.at(i)) && i < line.size(); ++i) {
                number_str[num_idx] = line.at(i);
                ++num_idx;
            }
            number_str[num_idx] = '\0';
            op.operand(atoi(number_str));
            op.op_type(Operations::OP_PUSH);
            op.col(col_start);
            line_ops.push_back(op);
        }

        if (isspace(line.at(i))) {
            continue;
        }

        // Check for whether implemented every operation in Operations
        assert(8 == Operations::OP_CNT && "Implement every operation");
        int col_start = i + 1;
        switch (line.at(i)) {
            case '+':
                op.op_type(Operations::OP_PLUS);
                break;
            case '-':
                op.op_type(Operations::OP_MINUS);
                break;
            case '.':
                op.op_type(Operations::OP_DUMP);
                break;
            case '=':
                op.op_type(Operations::OP_EQUAL);
                break;
            case 'i':
                if (line.at(i + 1) == 'f') {
                    op.op_type(Operations::OP_IF);
                }
                else {
                    op.op_type(Operations::OP_CNT);
                }
                ++i;
                break;
            case 'e':
                if (line.compare(i, strlen(STR_KEYWORD_END),
                            STR_KEYWORD_END) == 0) {
                    op.op_type(Operations::OP_END);
                    i += strlen(STR_KEYWORD_END);
                }
                else if (line.compare(i, strlen(STR_KEYWORD_ELSE),
                            STR_KEYWORD_ELSE) == 0) {
                    op.op_type(Operations::OP_ELSE);
                    i += strlen(STR_KEYWORD_ELSE);
                }
                else {
                    op.op_type(Operations::OP_CNT);
                }
                break;

            default:
                std::cerr << line.at(i) << '\n';
                op.op_type(Operations::OP_CNT);
        }
        op.col(col_start);
        line_ops.push_back(op);
    }
    return line_ops;
}


// strips spaces only from left of the string in place
// and return removed no of spaces
[[maybe_unused]] size_t lstrip(std::string &str) {
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

    bool SKIP_IF_BODY = false;
    bool SKIP_ELSE_BODY = false;
    for (auto it = operations_list.begin(); it != operations_list.end(); ++it)
    {
        // Check for whether implemented every operation in Operations
        assert(8 == Operations::OP_CNT && "Implement every operation");

        if (it->op_type() == OP_END) {
            SKIP_IF_BODY = false;
            SKIP_ELSE_BODY = false;
        }

        if (SKIP_ELSE_BODY) {
            continue;
        }

        if (SKIP_IF_BODY) {
            if (it->op_type() == OP_ELSE) {
                SKIP_IF_BODY = false;
            }
            continue;
        }

        switch (it->op_type()) {
            case Operations::OP_PUSH:
                if (program_stack.size() >= MAX_STACK_SIZE) {
                    std::cerr << "Stack size exceeded limit\n";
                    exit(EXIT_FAILURE);
                }
                program_stack.push(it->operand());
                break;

            case Operations::OP_PLUS:
                if (program_stack.size() >= 2) {
                    uint64_t a = program_stack.top();
                    program_stack.pop();
                    uint64_t b = program_stack.top();
                    program_stack.pop();
                    program_stack.push(a + b);
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "OP_PLUS(+) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;

            case Operations::OP_MINUS:
                if (program_stack.size() >= 2) {
                    uint64_t a = program_stack.top();
                    program_stack.pop();
                    uint64_t b = program_stack.top();
                    program_stack.pop();
                    program_stack.push(b - a);
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "OP_MINUS(-) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;

            case Operations::OP_DUMP:
                if (program_stack.size() < 1) {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "OP_DUMP operation\n";
                    exit(EXIT_FAILURE);
                }
                else {
                    std::cout << program_stack.top() << '\n';
                    program_stack.pop();
                }
                break;

            case Operations::OP_EQUAL:
                if (program_stack.size() < 2) {
                    std::cout << "ERROR: Not enough elements in stack for "
                        << "OP_EQUAL operation\n";
                    exit(EXIT_FAILURE);
                }
                else {
                    uint64_t a = program_stack.top();
                    program_stack.pop();
                    uint64_t b = program_stack.top();
                    program_stack.pop();

                    program_stack.push(a == b);
                }
                break;

            case Operations::OP_IF:
                if (program_stack.size() < 1) {
                    std::cout << "ERROR: Not enough elements in stack for "
                        << "OP_IF operation\n";
                    exit(EXIT_FAILURE);
                }
                else {
                    // if statement will not consume the bool_result
                    uint64_t bool_result = program_stack.top();
                    if (bool_result == 0) {
                        SKIP_IF_BODY = true;
                    }
                    else {
                        SKIP_IF_BODY = false;
                    }
                }
                break;

            case Operations::OP_END:
                // This Operation is handled outside the switch case.
                break;

            case Operations::OP_ELSE:
                if (!SKIP_IF_BODY) {
                    SKIP_ELSE_BODY = true;
                }
                break;

            default:
                std::cerr << "ERROR: Operation unknown\n";
                std::cerr << "op_type: " << it->op_type() << '\n';
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


// Compiles the program and creates executable ./a.out and generated assembly
// file %output_filename%.asm and relocatable %output_filename%.o
void compile_program(std::string output_filename, std::list<Operation> operations_list) {
    std::cout << "Compiling\n";

    int mock_stack_size = 0;

    std::ofstream out_file;
    out_file.open(output_filename + ".asm");

    out_file << "global _start\n";
    out_file << "segment .text\n";

    // Created using a C program to print a number
    // with a new line
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

    // Check for whether implemented every operation in Operations
    assert(8 == Operations::OP_CNT && "Implement every operation");
    uint64_t branch_counter = 0;
    uint64_t ip = 0;
    for (auto it = operations_list.begin(); it != operations_list.end(); ++it, ++ip)
    {
        switch (it->op_type()) {
            case Operations::OP_PUSH:
                if (mock_stack_size >= MAX_STACK_SIZE) {
                    std::cerr << "Stack size exceeded limit\n";
                    exit(EXIT_FAILURE);
                }
                out_file << "    ;; OP_PUSH\n";
                out_file << "    push " << it->operand() << '\n';
                ++mock_stack_size;
                break;

            case Operations::OP_PLUS:
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
                        << "OP_PLUS(+) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;

            case Operations::OP_MINUS:
                if (mock_stack_size >= 2) {
                    out_file << "    ;; OP_MINUS\n";
                    out_file << "    pop rdx\n";
                    out_file << "    pop rsi\n";
                    out_file << "    sub rsi, rdx\n";
                    out_file << "    push rsi\n";
                    --mock_stack_size;
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "OP_MINUS(-) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;

            case Operations::OP_DUMP:
                if (mock_stack_size >= 1) {
                    out_file << "    ;; OP_DUMP\n";
                    out_file << "    pop rdi\n";
                    out_file << "    call dump\n";
                    --mock_stack_size;
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "OP_DUMP operation\n";
                    exit(EXIT_FAILURE);
                }
                break;

            case Operations::OP_EQUAL:
                if (mock_stack_size >= 2) {
                    out_file << "    ;; OP_EQUAL\n";
                    out_file << "    pop rax\n";
                    out_file << "    pop rbx\n";

                    out_file << "    mov rcx, 0\n";
                    out_file << "    mov rdx, 1\n";
                    out_file << "    cmp rax, rbx\n";
                    out_file << "    cmove rcx, rdx\n";
                    out_file << "    push rcx\n";
                    --mock_stack_size;
                }
                else {
                    std::cerr << "ERROR: Not enough elements in stack for "
                        << "OP_EQUAL(-) operation\n";
                    exit(EXIT_FAILURE);
                }
                break;

            case Operations::OP_IF:
                if (mock_stack_size < 1) {
                    std::cout << "ERROR: Not enough elements in stack for "
                        << "OP_IF operation\n";
                    exit(EXIT_FAILURE);
                }
                else {
                    // if statement should not consume the bool_result
                    branch_counter = ip;
                    int err_num = generate_asm_for_if_else(out_file, it,
                            operations_list.end(), ip);
                    if (err_num) {
                        print_error(output_filename, it->line(), it->col(),
                                "Non closed if statement");
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            case Operations::OP_END:
                out_file << "branch" << branch_counter << ":\n";
                break;

            case Operations::OP_ELSE:
                out_file << "    jmp branch" << branch_counter << "\n";
                out_file << "branch" << branch_counter << "else:\n";
                break;

            default:
                std::cerr << "Compilation failed!\n";
                std::cerr << "ERROR: Operation unknown\n";
                exit(EXIT_FAILURE);
        }
    }

    // exiting with zero
    out_file << "    ;; returning from function with zero exit code\n";
    out_file << "    mov rax, 60\n";
    out_file << "    mov rdi, 0\n";
    out_file << "    syscall\n";
    out_file << "    ret\n";

    out_file.close();

    // Creating relocatable object
    std::string nasm_cmd = "nasm -felf64 ";
    nasm_cmd += OUTPUT_FILENAME;
    nasm_cmd += ".asm -o";
    nasm_cmd += OUTPUT_FILENAME;
    nasm_cmd += ".o";
    exec(nasm_cmd);

    // Creating executable
    std::string ld_cmd = "ld ";
    ld_cmd += OUTPUT_FILENAME;
    ld_cmd += ".o -o ./a.out";
    exec(ld_cmd);
}


// Helper funtion for echoing the command being
// executed.
void exec(const std::string cmd) {
    std::cout << "Exec: " << cmd << '\n';
    if (std::system(cmd.c_str()) != 0) {
        std::cerr << "ERROR: Failed executing " << cmd << '\n';
        exit(EXIT_FAILURE);
    }
}


int generate_asm_for_if_else(std::ofstream& out_file, std::list<Operation>::iterator begin,
        std::list<Operation>::iterator end, uint64_t ip) {

    assert(begin->op_type() == OP_IF && "Expects OP_IF only");

    out_file << "    ;; OP_IF\n";
    out_file << "    pop rax\n";
    out_file << "    test rax, rax\n";
    ++begin;

    bool has_else_block = false;
    while (begin != end) {
        if (begin->op_type() == OP_END) {
            break;
        }
        else if (begin->op_type() == OP_ELSE) {
            out_file << "    jz branch" << ip << "else\n";
            has_else_block = true;
            break; // only supports if-else
                   // does not support elifs
                   // later I may remove this break
        }
        ++begin;
    }

    if (begin == end) {
        return 1;
    }
    if (!has_else_block) {
        out_file << "    jz branch" << ip << "\n";
    }
    return 0;
}


void print_error(const std::string& program_file_name, const int line_num,
        const int col, const std::string msg) {
    std::cerr << program_file_name << ':' << line_num << ':'
        << col << ": ERROR: " << msg << '\n';
}
