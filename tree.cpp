#include "tree.h"

#include <string>



[[nodiscard]] bool Tree::init(const char *reg_expr, unsigned long length)
{
    State current = SPACE;
    for (unsigned long i = 0; i < length; ++i)
    {
        current = update_state(current, reg_expr[i]);
        if (current == ERROR)
            return printf("syntax error at i %lu\n", i), false;
    }

    p_head = new Node;
    p_head->p_parent = nullptr;
    if (not parse(p_head, reg_expr, length))
        return this->destroy(), false;
    return true;
}


[[nodiscard]] bool Tree::parse(Node *p_node, const char *reg_expr, unsigned long length)
{
    uint64_t first_scale = UINT64_MAX;
    uint64_t first_summ  = UINT64_MAX;
    uint64_t i = 0;
    uint64_t ib_count = 0;
    bool squares = false;
    for (; i < length; ++i)
    {
        switch (reg_expr[i])
        {
            case '(':
                ++ib_count;
                squares = true;
                continue;
            case ')':
                --ib_count;
                continue;

                {
                    case '=': goto DECODE;
                    case '*':
                    case '/':
                    if (ib_count == 0)
                        first_scale = i;
                        break;
                    case '+':
                    case '-':
                    if (ib_count == 0)
                        first_summ = i;
                        break;
                }
        }
    }

    if (first_summ != UINT64_MAX)
        i = first_summ;
    else if (first_scale != UINT64_MAX)
        i = first_scale;
    else if (not squares)
    {
        p_node->init(p_node->p_parent, reg_expr[1] > '9' ? Operator::ID_NAME : Operator::CONST,
                    {reg_expr, reg_expr + length});
        return true;
    }
    else return parse(p_node, reg_expr + 1, length - 2);

    if (i == 0)
    {
        p_node->operation = Operator::DIFF;
        p_node->left() = new Node;
        p_node->left()->init(p_node, reg_expr[1] > '9' ? Operator::ID_NAME : Operator::CONST,
                             {reg_expr + 1, reg_expr + length });
        p_node->right() = nullptr;
        p_node->id_name = '-';
        return true;
    }

DECODE:

    switch (reg_expr[i])
    {
        case '=': p_node->operation = Operator::EQUAL;
            break;
        case '*': p_node->operation = Operator::SCALE;
            break;
        case '/': p_node->operation = Operator::DIV;
            break;
        case '+': p_node->operation = Operator::SUMM;
            break;
        case '-': p_node->operation = Operator::DIFF;
            break;
    }

    p_node->id_name   = {reg_expr[i]};
    p_node->left() = new Node;
    p_node->left()->p_parent = p_node;
    p_node->right() = new Node;
    p_node->right()->p_parent = p_node;

    if (parse(p_node->left(), reg_expr, i))
    {
        if(parse(p_node->right(), reg_expr + i+1, length - i - 1))
            return true;
    }

    delete p_node->left();
    delete p_node->right();
    return false;
}

void Tree::destroy()
{
    p_head->destroy();
}

void Tree::Node::init(Node *p_parent_, Operator o, std::string s)
{
    operation = o;
    id_name = s;
    p_next[LEFT] = p_next[RIGHT] = nullptr;
    p_parent = p_parent_;
}

void Tree::Node::destroy()
{
    if (p_next[LEFT] != nullptr)
        p_next[LEFT]->destroy();
    if (p_next[RIGHT] != nullptr)
        p_next[RIGHT]->destroy();

    delete this;
}
