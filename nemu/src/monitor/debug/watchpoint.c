#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;
    free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp() {
    WP *ret = free_, *sup = head;
    free_ = free_->next;
    ret->next = NULL;
    if (!sup) {
        head = ret;
        sup = head;
    } else {
        while (sup->next)sup = sup->next;
        sup->next = ret;
    }
    return ret;
}

void free_wp(WP *wp) {
    WP *sup1 = free_, *sup2 = head;
    if (!sup1) {
        free_ = wp;
        sup1 = free_;
    } else {
        while (sup1->next)sup1 = sup1->next;
        sup1->next = wp;
    }
    if (!head) assert(0);
    if (head->NO == wp->NO) head = head->next;
    else {
        while (sup2->next && sup2->next->NO != wp->NO)sup2 = sup2->next;
        if (sup2->next->NO == wp->NO)sup2->next = sup2->next->next;
        else assert(0);
    }
    wp->next = NULL;
    wp->expr[0] = '\0';
    wp->result = 0;
}

bool check_wp() {
    WP *wp = head;
    bool is_success;
    bool blocked = false;
    while (wp) {
        uint32_t res = expr(wp->expr, &is_success);
        if (!is_success)assert(1);
        if (res != wp->result) {
            blocked = true;
            printf("Watchpoint NO. %d blocked: %s\n", wp->NO, wp->expr);
            printf("Before blocked: %d\n", wp->result);
            printf("After blocked: %d\n", res);
            wp->result = res;
        }
        wp = wp->next;
    }
    return blocked;
}

void print_wp() {
    WP *wp = head;
    while (wp) {
        printf("Watchpoint NO. %d: %s = %d\n", wp->NO, wp->expr, wp->result);
        wp = wp->next;
    }
}

void delete_wp(int index) {
    free_wp(&wp_pool[index]);
}
