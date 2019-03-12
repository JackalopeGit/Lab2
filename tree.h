#pragma once

#include "states.h"

#include <string>

enum class Operator : char
{
    NONE, SUMM, DIFF, DIV, SCALE, EQUAL, AND, OR, CONST, ID_NAME,
    COUNT,
    OPERATOR_END_RANGE = OR
};

using Id = unsigned int;

struct Tree
{
    struct Node
    {
        enum Side { LEFT, RIGHT };

        void init(Node *p_parent, Operator, std::string);
        void destroy();

        Node*& left() { return p_next[LEFT]; }
        Node*& right() { return p_next[RIGHT]; }

        Operator    operation;
        std::string id_name;
        Node       *p_next[2];
        Node       *p_parent;
    };

    [[nodiscard]] bool init(const char *reg_expr, unsigned long length);
    [[nodiscard]] bool parse(Node *p_node, const char *reg_expr, unsigned long length);

    void destroy();

    Node* put_node(Operator type, std::string name, Node *p_node, bool is_right);


    Node *p_head;
};


