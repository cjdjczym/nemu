#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
    NOTYPE = 256, EQ, NUM, NEG, HEXNUM, REG, NEQ, AND, OR, POINTER

    /* TODO: Add more token types */

};

static struct rule {
    char *regex;
    int token_type;
    int precedence;
} rules[] = {

        /* TODO: Add more rules.
         * Pay attention to the precedence level of different rules.
         */

        {"\\b[0-9]+\\b",            NUM,    0},      // %d number
        {"\\b0[xX][0-9a-fA-F]+\\b", HEXNUM, 0},      // %x number
        {"\\$[a-zA-Z]+",            REG,    0},      // register
        {" +",                      NOTYPE, 0},      // spaces
        {"\\|\\|",                  OR,     1},      // or
        {"&&",                      AND,    2},      // and
        {"==",                      EQ,     3},      // equal
        {"!=",                      NEQ,    3},      // not-equal
        {"\\+",                     '+',    4},      // plus
        {"-",                       '-',    4},      // minus
        {"\\*",                     '*',    5},      // multi
        {"/",                       '/',    5},      // divide
        {"!",                       '!',    6},      // not
        {"\\(",                     '(',    7},      // left bracket
        {"\\)",                     ')',    7},      // right bracket
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
        }
    }
}

typedef struct token {
    int type;
    char str[32];
    int precedence;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;

//                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position,
//                    substr_len, substr_len, substr_start);

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                switch (rules[i].token_type) {
                    case NOTYPE:
                        break;
                    case REG:
                        tokens[nr_token].precedence = rules[i].precedence;
                        tokens[nr_token].type = rules[i].token_type;
                        strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
                        nr_token++;
                        break;
                    default:
                        tokens[nr_token].type = rules[i].token_type;
                        tokens[nr_token].precedence = rules[i].precedence;
                        strncpy(tokens[nr_token].str, substr_start, substr_len);
                        tokens[nr_token].str[substr_len] = '\0';
                        nr_token++;
                }
                position += substr_len;
                break;
            }
        }

        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }
    return true;
}

bool check_parentheses(int p, int q) {
    int i;
    if (tokens[p].type == '(' && tokens[q].type == ')') {
        int leftCount = 0, rightCount = 0;
        for (i = p + 1; i < q; i++) {
            if (tokens[i].type == '(')leftCount++;
            else if (tokens[i].type == ')')rightCount++;
            if (leftCount < rightCount)return false;
        }
        if (leftCount == rightCount) return true;
    }
    return false;
}

int dominant_operator(int p, int q) {
    int i, j;
    int min_level = 8;
    int res = p;
    for (i = p; i <= q; i++) {
        if (tokens[i].type == NUM || tokens[i].type == HEXNUM) continue;
        bool is_dominant = true;
        int count = 0;
        for (j = i - 1; j >= p; j--) {
            if (tokens[j].type == '(' && !count) {
                is_dominant = false;
                break;
            }
            if (tokens[j].type == '(')count--;
            if (tokens[j].type == ')')count++;
        }
        if (!is_dominant)continue;
        if (tokens[i].precedence <= min_level) {
            min_level = tokens[i].precedence;
            res = i;
        }
    }
    return res;
}

uint32_t eval(int p, int q) {
    if (p > q) {
        assert(1);
        return -1;
    } else if (p == q) {
        uint32_t number = 0;
        if (tokens[p].type == NUM)
            sscanf(tokens[p].str, "%d", &number);
        if (tokens[p].type == HEXNUM)
            sscanf(tokens[p].str, "%x", &number);
        if (tokens[p].type == REG) {
//            printf("!!!!!!!!!!!!register name: %s\n", tokens[p].str);
//            printf("????????????register length: %d\n", (int)strlen(tokens[p].str));
            if (strlen(tokens[p].str) == 3) {
                int i;
                for (i = R_EAX; i <= R_EDI; i++) if (strcmp(tokens[i].str, regsl[i]) == 0)break;
                if (i <= R_EDI) number = reg_l(i);
                else if (i > R_EDI && strcmp(tokens[i].str, "eip") == 0) number = cpu.eip;
                else printf("1Wrong register name: %s\n", tokens[i].str);
            } else if (strlen(tokens[p].str) == 2) {
                if (tokens[p].str[1] == 'x' || tokens[p].str[1] == 'p' || tokens[p].str[1] == 'i') {
                    int i;
                    for (i = R_AX; i <= R_DI; i++)if (strcmp(tokens[i].str, regsw[i]) == 0)break;
                    number = reg_w(i);
                } else if (tokens[p].str[1] == 'l' || tokens[p].str[1] == 'h') {
                    int i;
                    for (i = R_AL; i <= R_BH; i++)if (strcmp(tokens[i].str, regsb[i]) == 0)break;
                } else printf("2Wrong register name: %s\n", tokens[p].str);
            } else printf("3Wrong register name: %s\n", tokens[p].str);
        }
        return number;
    } else if (check_parentheses(p, q)) {
        return eval(p + 1, q - 1);
    } else {
        int main_op = dominant_operator(p, q);
        if (p == main_op || tokens[main_op].type == NEG || tokens[main_op].type == '!' ||
            tokens[main_op].type == POINTER) {
            uint32_t num = eval(p + 1, q);
            switch (tokens[p].type) {
                case '!':
                    return !num;
                case NEG:
                    return -num;
                case POINTER:
                    return swaddr_read(num, 4);
            }
        }
        uint32_t first = eval(p, main_op - 1);
        uint32_t second = eval(main_op + 1, q);
//        printf("first: %d\tsecond: %d\t%c\n", first, second, tokens[main_op].type);
        switch (tokens[main_op].type) {
            case '+':
                return first + second;
            case '-':
                return first - second;
            case '*':
                return first * second;
            case '/':
                return first / second;
            case EQ:
                return first == second;
            case NEQ:
                return first != second;
            case AND:
                return first && second;
            case OR:
                return first || second;
            default:
                break;
        }
    }
    assert(1);
    return -1;
}

uint32_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }
    int i;
    for (i = 0; i < nr_token; i++) {
        if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != NUM && tokens[i - 1].type != HEXNUM &&
                                                 tokens[i - 1].type != REG && tokens[i - 1].type != ')'))) {
            tokens[i].type = NEG;
            tokens[i].precedence = 6;
        }
        if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != NUM && tokens[i - 1].type != HEXNUM &&
                                                 tokens[i - 1].type != REG && tokens[i - 1].type != ')'))) {
            tokens[i].type = POINTER;
            tokens[i].precedence = 6;
        }
    }

    /* TODO: Insert codes to evaluate the expression. */
    *success = true;
    return eval(0, nr_token - 1);
}

