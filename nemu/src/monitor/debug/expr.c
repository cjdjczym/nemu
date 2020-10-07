#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
    NOTYPE = 256, EQ, NUM

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

        {"\\b[0-9]+\\b", NUM,    0},        // %d number
        {" +",           NOTYPE, 0},        // spaces
        {"==",           EQ,     3},        // equal
        {"\\+",          '+',    4},        // plus
        {"-",            '-',    4},        // minus
        {"\\*",          '*',    5},        // multi
        {"/",            '/',    5},        // divide
        {"\\(",          '(',    7},        // left bracket
        {"\\)",          ')',    7},        // right bracket
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

                Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position,
                    substr_len, substr_len, substr_start);

                /* TODO: Now a new token is recognized with rules[i]. Add codes
                 * to record the token in the array `tokens'. For certain types
                 * of tokens, some extra actions should be performed.
                 */

                switch (rules[i].token_type) {
                    case NOTYPE:
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
            if (leftCount <= rightCount)return false;
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
        if (tokens[i].type == NUM) continue;
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
        return number;
    } else if (check_parentheses(p, q)) {
        printf("%c!!!!!!!!!!!!!!!!%c",tokens[p].type,tokens[q].type);
        return eval(p + 1, q - 1);
    }
    else {
        int main_op = dominant_operator(p, q);
        uint32_t first = eval(p, main_op - 1);
        uint32_t second = eval(main_op + 1, q);
        printf("first: %d\tsecond: %d\t%c\n", first, second, tokens[main_op].type);
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

    /* TODO: Insert codes to evaluate the expression. */
    *success = true;
    return eval(0, nr_token - 1);
}

