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
    if(!sup1->next){
        free_ = wp;
        sup1 = free_;
    } else {
        while(sup1->next)sup1 = sup1->next;
        sup1->next = wp;
    }
    if(head->NO == wp->NO) head = head->next;
    else {
        while(sup2->next && sup2->next->NO != wp->NO)sup2 = sup2->next;
        if(sup2->next->NO == wp->NO)sup2->next = sup2->next->next;
        else assert(0);
    }
    wp->next = NULL;
}

