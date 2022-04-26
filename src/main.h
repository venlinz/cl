

#define MAX_KEYWORD_LEN 32
#define TEST_PROGRAM "./examples/test.cl"
#define MAX_STACK_SIZE 1024

#define STR_KEYWORD_END "end"
#define STR_KEYWORD_WHILE "while"
#define STR_KEYWORD_ELSE "else"
#define STR_KEYWORD_DUP "dup"

enum Operations {
    OP_PUSH,
    OP_PLUS,
    OP_MINUS,
    OP_DUMP,
    OP_EQUALS,
    OP_LESS_THAN,
    OP_IF,
    OP_ELSE,
    OP_END,
    OP_DUP,
    OP_WHILE,
    OP_CNT, // This value is treated as UNKNOWN OPERATION
};


class Operation {
    private:
        Operations m_op = OP_CNT;
        uint64_t m_opr = 0;
        int m_line = -1;
        int m_col = -1;
        uint64_t m_jump_loc = 0;

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

        void jump_loc(uint64_t j) {
            m_jump_loc = j;
        }
        uint64_t jump_loc() const {
            assert((op_type() == Operations::OP_IF ||
                    op_type() == Operations::OP_ELSE)
                  && "jump_loc should not be called with other operands");
            return m_jump_loc;
        }
};


#define STR_OPT_COMPILE "c"
#define STR_OPT_SIMULATE "s"
#define STR_OPT_HELP "help"

#define OUTPUT_FILENAME "output"

[[nodiscard]] std::list<Operation> parse_program(std::string program_file_name);
[[nodiscard]] std::list<Operation> parse_op_from_line(std::string line);
[[maybe_unused]] size_t lstrip(std::string &str);
void simulate_program(std::string program_file_name,
        std::list<Operation> operations_list);
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
void add_boilerplate_asm(std::ofstream& out_file);
uint64_t while_loop_span(std::list<Operation>::iterator begin,
        std::list<Operation>::iterator end);
void crossreference_conditional(std::list<Operation>::iterator begin,
        std::list<Operation>::iterator end);
