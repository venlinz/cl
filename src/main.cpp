#include <fstream>
#include <iostream>
#include <list>
#include <stack>
#include <string>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "main.h"


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
        simulate_program(program_file_name, operations);
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
            for (; i < line.size() && isdigit(line.at(i)); ++i) {
                number_str[num_idx] = line.at(i);
                ++num_idx;
            }
            number_str[num_idx] = '\0';
            op.operand(atoi(number_str));
            op.op_type(Operations::OP_PUSH);
            op.col(col_start);
            line_ops.push_back(op);
            continue;
        }

        if (isspace(line.at(i))) {
            continue;
        }

        // Check for whether implemented every operation in Operations
        assert(14 == Operations::OP_CNT && "Implement every operation" "parse_op_from_line");
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
                op.op_type(Operations::OP_EQUALS);
                break;
            case '<':
                if (i + 1 < line.size() && line.at(i + 1) == '=') {
                    op.op_type(Operations::OP_LESS_THAN_EQ);
                    ++i;
                }
                else {
                    op.op_type(Operations::OP_LESS_THAN);
                }
                break;
            case '>':
                if (i + 1 < line.size() && line.at(i + 1) == '=') {
                    op.op_type(Operations::OP_GREATER_THAN_EQ);
                    ++i;
                }
                else {
                    op.op_type(Operations::OP_GREATER_THAN);
                }
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

            case 'w':
                if (line.compare(i, strlen(STR_KEYWORD_WHILE),
                            STR_KEYWORD_WHILE) == 0) {
                    op.op_type(Operations::OP_WHILE);
                    i += strlen(STR_KEYWORD_WHILE);
                }
                else {
                    op.op_type(Operations::OP_CNT);
                }
                break;

            case 'd':
                if (line.compare(i, strlen(STR_KEYWORD_DUP),
                            STR_KEYWORD_DUP) == 0) {
                    op.op_type(Operations::OP_DUP);
                    i += strlen(STR_KEYWORD_DUP);
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


void simulate_program(std::string program_file_name,
        std::list<Operation> operations_list) {
    std::cout << "Simulating\n";
    std::stack<uint64_t> program_stack;

    crossreference_conditional(operations_list.begin(), operations_list.end());
    uint64_t ip = 0;
    for (auto it = operations_list.begin(); it != operations_list.end(); ++it, ++ip)
    {
        assert(14 == Operations::OP_CNT && "Implement every operation"
                && "simulate_program()");

        switch (it->op_type()) {
            case Operations::OP_PUSH:
                if (program_stack.size() >= MAX_STACK_SIZE) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Stack size exceeded limit");
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
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_PLUS(+) operation");
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
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_MINUS(-) operation");
                    exit(EXIT_FAILURE);
                }
                break;

            case Operations::OP_DUMP:
                if (program_stack.size() < 1) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_DUMP operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    std::cout << program_stack.top() << '\n';
                    program_stack.pop();
                }
                break;

            case Operations::OP_EQUALS:
                if (program_stack.size() < 2) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_EQUALS operation");
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

            case Operations::OP_LESS_THAN:
                if (program_stack.size() < 2) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_LESS_THAN operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    uint64_t a = program_stack.top();
                    program_stack.pop();
                    uint64_t b = program_stack.top();
                    program_stack.pop();
                    program_stack.push(b < a);
                }
                break;

            case Operations::OP_LESS_THAN_EQ:
                if (program_stack.size() < 2) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_LESS_THAN_EQ operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    uint64_t a = program_stack.top();
                    program_stack.pop();
                    uint64_t b = program_stack.top();
                    program_stack.pop();
                    program_stack.push(b <= a);
                }
                break;

            case Operations::OP_GREATER_THAN:
                if (program_stack.size() < 2) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_GREATER_THAN operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    uint64_t a = program_stack.top();
                    program_stack.pop();
                    uint64_t b = program_stack.top();
                    program_stack.pop();
                    program_stack.push(b > a);
                }
                break;

            case Operations::OP_GREATER_THAN_EQ:
                if (program_stack.size() < 2) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_GREATER_THAN operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    uint64_t a = program_stack.top();
                    program_stack.pop();
                    uint64_t b = program_stack.top();
                    program_stack.pop();
                    program_stack.push(b >= a);
                }
                break;

            case Operations::OP_DUP:
                if (program_stack.size() < 1) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_EQUALS operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    program_stack.push(program_stack.top());
                }
                break;

            case Operations::OP_IF:
                if (program_stack.size() < 1) {
                    print_error(program_file_name, it->line(), it->col(),
                            "Not enough elements in stack for OP_IF operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    // if statement will not consume the bool_result
                    uint64_t bool_result = program_stack.top();
                    if (bool_result == 0) {
                        auto ifp = it->jump_loc();
                        while (ip != ifp) {
                            ++ip;
                            ++it;
                        }
                    }
                }
                break;

            case Operations::OP_END:
                // This Operation is handled outside the switch case.
                break;

            case Operations::OP_ELSE:
                while (it != operations_list.end() &&
                        it->op_type() != Operations::OP_END) {
                    ++it;
                    ++ip;
                }
                break;

            case Operations::OP_WHILE:
                assert(false && "Not implemented  while loop");
                /* if (program_stack.size() < 1) { */
                /*     print_error(program_file_name, it->line(), it->col(), */
                /*             "Not enough elements in stack for OP_WHILE operation"); */
                /*     exit(EXIT_FAILURE); */
                /* } */
                /* else { */
                /* } */
                break;

            default:
                print_error(program_file_name, it->line(), it->col(),
                        "Operation unknown");
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

    add_boilerplate_asm(out_file);

    crossreference_conditional(operations_list.begin(), operations_list.end());
    std::stack<uint64_t> conditional_stack;

    // Check for whether implemented every operation in Operations
    assert(14 == Operations::OP_CNT && "Implement every operation"
            "compile_program");
    uint64_t ip = 0;
    for (auto it = operations_list.begin(); it != operations_list.end(); ++it, ++ip)
    {
        if (mock_stack_size >= MAX_STACK_SIZE) {
            std::cerr << "Stack size exceeded limit\n";
            exit(EXIT_FAILURE);
        }
        switch (it->op_type()) {
            case Operations::OP_PUSH:
                out_file << "    ;; OP_PUSH\n";
                out_file << "    push " << it->operand() << '\n';
                ++mock_stack_size;
                break;

            case Operations::OP_PLUS:
                if (mock_stack_size < 2) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_PLUS(+) operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; ADD\n";
                    out_file << "    pop rdx\n";
                    out_file << "    pop rsi\n";
                    out_file << "    add rdx, rsi\n";
                    out_file << "    push rdx\n";
                    --mock_stack_size;
                }
                break;

            case Operations::OP_MINUS:
                if (mock_stack_size < 2) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_MINUS(-) operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; OP_MINUS\n";
                    out_file << "    pop rdx\n";
                    out_file << "    pop rsi\n";
                    out_file << "    sub rsi, rdx\n";
                    out_file << "    push rsi\n";
                    --mock_stack_size;
                }
                break;

            case Operations::OP_DUMP:
                if (mock_stack_size < 1) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_DUMP operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; OP_DUMP\n";
                    out_file << "    pop rdi\n";
                    out_file << "    call dump\n";
                    --mock_stack_size;
                }
                break;

            case Operations::OP_DUP:
                if (mock_stack_size < 1) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_DUP operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    pop rax\n";
                    out_file << "    push rax\n";
                    out_file << "    push rax\n";
                    ++mock_stack_size;
                }
                break;

            case Operations::OP_EQUALS:
                if (mock_stack_size < 2) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_EQUALS(=) operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; OP_EQUALS\n";
                    out_file << "    pop rax\n";
                    out_file << "    pop rbx\n";

                    out_file << "    mov rcx, 0\n";
                    out_file << "    mov rdx, 1\n";
                    out_file << "    cmp rax, rbx\n";
                    out_file << "    cmove rcx, rdx\n";
                    out_file << "    push rcx\n";
                    --mock_stack_size;
                }
                break;

            case Operations::OP_LESS_THAN:
                if (mock_stack_size < 2) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_EQUALS(=) operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; OP_EQUALS\n";
                    out_file << "    pop rax\n";
                    out_file << "    pop rbx\n";

                    out_file << "    mov rcx, 0\n";
                    out_file << "    mov rdx, 1\n";
                    out_file << "    cmp rbx, rax\n";
                    out_file << "    cmovl rcx, rdx\n";
                    out_file << "    push rcx\n";
                    --mock_stack_size;
                }
                break;

            case Operations::OP_LESS_THAN_EQ:
                if (mock_stack_size < 2) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_EQUALS(=) operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; OP_EQUALS\n";
                    out_file << "    pop rax\n";
                    out_file << "    pop rbx\n";

                    out_file << "    mov rcx, 0\n";
                    out_file << "    mov rdx, 1\n";
                    out_file << "    cmp rbx, rax\n";
                    out_file << "    cmovle rcx, rdx\n";
                    out_file << "    push rcx\n";
                    --mock_stack_size;
                }
                break;

            case Operations::OP_GREATER_THAN:
                if (mock_stack_size < 2) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_EQUALS(=) operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; OP_EQUALS\n";
                    out_file << "    pop rax\n";
                    out_file << "    pop rbx\n";

                    out_file << "    mov rcx, 0\n";
                    out_file << "    mov rdx, 1\n";
                    out_file << "    cmp rbx, rax\n";
                    out_file << "    cmovg rcx, rdx\n";
                    out_file << "    push rcx\n";
                    --mock_stack_size;
                }
                break;

            case Operations::OP_GREATER_THAN_EQ:
                if (mock_stack_size < 2) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_EQUALS(=) operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; OP_EQUALS\n";
                    out_file << "    pop rax\n";
                    out_file << "    pop rbx\n";

                    out_file << "    mov rcx, 0\n";
                    out_file << "    mov rdx, 1\n";
                    out_file << "    cmp rbx, rax\n";
                    out_file << "    cmovge rcx, rdx\n";
                    out_file << "    push rcx\n";
                    --mock_stack_size;
                }
                break;

            case Operations::OP_IF:
                if (mock_stack_size < 1) {
                    print_error(output_filename, it->line(), it->col(),
                            "Not enough elements in stack for OP_IF operation");
                    exit(EXIT_FAILURE);
                }
                else {
                    out_file << "    ;; OP_IF\n";
                    out_file << "    pop rax\n";
                    out_file << "    test rax, rax\n";
                    conditional_stack.push(it->jump_loc());
                    // Hack to make else-less if statement work
                    conditional_stack.push(0);
                    out_file << "    jz br" << it->jump_loc() << "else\n";
                    // if statement should not consume the bool_result
                    /* branch_counter = ip; */
                    /* int err_num = generate_asm_for_if_else(out_file, it, */
                    /*         operations_list.end(), ip); */
                    /* if (err_num) { */
                    /*     print_error(output_filename, it->line(), it->col(), */
                    /*             "Non closed if statement"); */
                    /*     exit(EXIT_FAILURE); */
                    /* } */
                }
                break;
            case Operations::OP_END:
                // Hack to make else-less if statement work
                if (conditional_stack.top() == 0) {
                    conditional_stack.pop();
                    out_file << "br" << conditional_stack.top() << "else:\n";
                }
                out_file << "br" << conditional_stack.top() << ":\n";
                conditional_stack.pop();
                break;

            case Operations::OP_ELSE:
                // Hack to make else-less if statement work
                conditional_stack.pop();
                out_file << "    jmp br" << conditional_stack.top() << "\n";
                out_file << "br" << conditional_stack.top() << "else:\n";
                break;

            case Operations::OP_WHILE:
                assert(false && "Not implemented OP_WHILE for compilation mode");
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


void add_boilerplate_asm(std::ofstream& out_file) {
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
}


uint64_t while_loop_span(std::list<Operation>::iterator begin,
        std::list<Operation>::iterator end) {
    assert(begin->op_type() == Operations::OP_WHILE);
    ++begin;
    uint64_t ip_count = 1;

    int if_conditions_count = 0;
    /* int conditional_end_count = 0; */
    for (; begin != end; ++begin) {
        if (begin->op_type() == Operations::OP_IF) {
            ++if_conditions_count;
        }
        else if (begin->op_type() == Operations::OP_END) {
            if (if_conditions_count == 0) {
                break;
            }
            else {
                --if_conditions_count;
            }
        }
        else {
            // pass
        }
        ++ip_count;
    }
    if (begin == end) {
        std::cerr << "ERRRROR: Unclosed end block in while\n";
        exit(EXIT_FAILURE);
    }
    if (if_conditions_count != 0) {
        std::cerr << "ERRRROR: Unclosed end block in while\n";
        exit(EXIT_FAILURE);
    }

    return ip_count + 1;
}


// Adds where a conditional statement ends
void crossreference_conditional(std::list<Operation>::iterator begin,
        std::list<Operation>::iterator end) {
    std::stack<Operation *> conditional_op;
    // Check for whether implemented conditional operation in Operations
    assert(14 == Operations::OP_CNT && "Implement conditional operations" "crossreference_conditional");
    uint64_t ip = 0;
    for (auto it = begin; it != end; ++it, ++ip) {
        if (it->op_type() == Operations::OP_IF) {
            conditional_op.push(&(*it));
        }
        else if (it->op_type() == Operations::OP_END) {
            if (!conditional_op.empty()) {
                auto c_op = conditional_op.top();
                if (c_op->jump_loc() == 0) {
                    c_op->jump_loc(ip);
                }
                conditional_op.pop();
            }
            else {
                std::cerr << "Operations on empty stack\n";
            }
        }
        else if (it->op_type() == Operations::OP_ELSE) {
            if (!conditional_op.empty()) {
                auto c_op = conditional_op.top();
                c_op->jump_loc(ip);
            }
            else {
                std::cerr << "Operations on empty stack\n";
            }
        }
    }
}
