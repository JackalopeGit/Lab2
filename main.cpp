
#include "tree.h"

#include <cstring>
#include <vector>

enum CodeIds
{
    MOV_, NL, COMMA_, EAX, EDX, NEG_, ADD_, SUB_, IMUL_, IDIV_, PUSH_, POP_, XCHG_, _ERROR_
};

enum Type
{
    CODE, P_S
};

struct Code
{
    union
    {
        uint64_t id;
        std::string *p_s;
    };
    Type type;

    bool operator == (Code c) const
    {
        if (type == CODE && c.type == CODE)
            return c.id == id;
        if (type == c.type)
            return true;
        return false;
    }
};
using Codes = std::vector<Code>;

constexpr static const char * CODES[] =
{
    "mov ",
    "\n",
    ", ",
    "eax",
    "edx",
    "neg ",
    "add ",
    "sub ",
    "imul ",
    "idiv ",
    "push ",
    "pop ",
    "xchg ",
    " ERROR "
};


inline Code get_code(CodeIds id)
{
    Code code;
    code.id = id;
    code.type = CODE;
    return code;
}
inline Code get_id(std::string *p_id)
{
    Code code;
    code.p_s = p_id;
    code.type = P_S;
    return code;
}

inline std::vector<Code> name_code(Tree::Node *p_node)
{
    std::vector<Code> a;
    a.emplace_back(get_code(MOV_));
    a.emplace_back(get_code(EAX));
    a.emplace_back(get_code(COMMA_));
    a.emplace_back(get_id(&p_node->id_name));
    a.emplace_back(get_code(NL));
    return a;
}

inline std::vector<Code> name_code2(Tree::Node *p_node)
{
    std::vector<Code> a;
    a.emplace_back(get_code(MOV_));
    a.emplace_back(get_id(&p_node->id_name));
    a.emplace_back(get_code(COMMA_));
    a.emplace_back(get_code(EAX));
    a.emplace_back(get_code(NL));
    return a;
}

inline std::vector<Code> binary_code(Operator op)
{
    switch (op)
    {
        case Operator::SUMM:  return {get_code(ADD_)};
        case Operator::DIFF:  return {get_code(SUB_)};
        case Operator::SCALE: return {get_code(IMUL_)};
        case Operator::DIV:   return {get_code(IDIV_)};
        default: return {get_code(_ERROR_)};
    }
}

static std::vector<Code> get_code(Tree::Node *p_node)
{
    switch (p_node->operation)
    {
        case Operator::ID_NAME:
        case Operator::CONST: return name_code(p_node);
        case Operator::DIFF:
        {
            int count = (p_node->left() != nullptr) + (p_node->right() != nullptr);
            if (count < 2)
            {
                Codes a = name_code(p_node->p_next[p_node->right() != nullptr]);
                a.emplace_back(get_code(NEG_));
                a.emplace_back(get_code(EAX));
                a.emplace_back(get_code(NL));
                return a;
            }
        }
        case Operator::SUMM:
        case Operator::SCALE:
        case Operator::DIV:
        {
            Codes l_code, r_code;

            if (p_node->left() != nullptr)
                l_code = get_code(p_node->left());
            else l_code = name_code(p_node->right());

            if (p_node->right() != nullptr)
                r_code = get_code(p_node->right());
            else l_code = name_code(p_node->right());

            Codes a;
            a.insert(a.end(), l_code.begin(), l_code.end());
            a.emplace_back(get_code(PUSH_));
            a.emplace_back(get_code(EAX));
            a.emplace_back(get_code(NL));
            a.insert(a.end(), r_code.begin(), r_code.end());
            a.emplace_back(get_code(POP_));
            a.emplace_back(get_code(EDX));
            a.emplace_back(get_code(NL));

            a.emplace_back(get_code(XCHG_));
            a.emplace_back(get_code(EAX));
            a.emplace_back(get_code(COMMA_));
            a.emplace_back(get_code(EDX));
            a.emplace_back(get_code(NL));

            auto b = binary_code(p_node->operation);
            a.insert(a.end(), b.begin(), b.end());

            a.emplace_back(get_code(EAX));
            a.emplace_back(get_code(COMMA_));
            a.emplace_back(get_code(EDX));
            a.emplace_back(get_code(NL));

            return a;
        }
        case Operator::EQUAL:
        {
            Codes a;
            auto code = get_code(p_node->right());
            a.insert(a.end(), code.begin(), code.end());
            auto n = name_code2(p_node->left());
            a.insert(a.end(), n.begin(), n.end());
            return a;
        }
        case Operator::NONE:
        {
            return Codes{get_code(_ERROR_)};
        }
        default:
        {
            return Codes{get_code(_ERROR_)};
        }
    }
}


void print_code(Codes codes)
{
    for (auto &code : codes)
    {
        switch(code.type)
        {
            case CODE: printf("%s", CODES[code.id]);
                break;
            case P_S:  printf("%s", code.p_s->c_str());
        }
        fflush(stdout);
    }
}


Codes optimize(Codes bad_code)
{
    static const Codes BAD_1_1 =
    {
        get_code(EAX), get_code(COMMA_), get_id(nullptr), get_code(NL),

        get_code(PUSH_), get_code(EAX), get_code(NL)
    };

    static const Codes BAD_1_2 =
    {
        get_code(POP_), get_code(EDX), get_code(NL),

        get_code(XCHG_), get_code(EAX), get_code(COMMA_), get_code(EDX), get_code(NL)
    };

    static const Codes BAD_1_3 =
    {
        get_code(EAX), get_code(COMMA_), get_code(EDX), get_code(NL)
    };

    static const Codes GOOD_1_1 =
    {
        get_code(EDX), get_code(COMMA_), get_id(nullptr), get_code(NL),

        get_code(XCHG_), get_code(EAX), get_code(COMMA_), get_code(EDX), get_code(NL)
    };
    static const Codes &GOOD_1_2 = BAD_1_3;

    static const Codes BAD_2_1
    {
        get_code(PUSH_), get_code(EAX), get_code(NL),

        get_code(MOV_), get_code(EAX), get_code(COMMA_), get_id(nullptr), get_code(NL),

        get_code(POP_), get_code(EDX), get_code(NL),

        get_code(XCHG_), get_code(EAX), get_code(COMMA_), get_code(EDX), get_code(NL),
    };

    static const Codes BAD_2_2
    {
        get_code(EAX), get_code(COMMA_), get_code(EDX), get_code(NL)
    };

    static const Codes GOOD_2_1
    {
        get_code(MOV_), get_code(EDX), get_code(COMMA_), get_id(nullptr), get_code(NL),

                get_code(SUB_), get_code(EAX), get_code(COMMA_), get_code(EDX), get_code(NL)
    };

    static const Codes BAD_3_1
    {
        get_code(XCHG_), get_code(EAX), get_code(COMMA_), get_code(EDX), get_code(NL),
    };


    static const Codes &GOOD_3 = BAD_2_2;


    static const Codes BAD_4_1
    {
        get_code(MOV_), get_code(EDX), get_code(COMMA_), get_id(nullptr), get_code(NL)
    };

    static const Codes &BAD_4_2 = BAD_2_2;
    static const Codes &GOOD_4
    {
        get_code(EAX), get_code(COMMA_), get_id(nullptr), get_code(NL)
    };


    Codes new_code;

    bool is_optimized = false;

    for(size_t i = 0; i < bad_code.size(); ++i)
    {
        if (BAD_1_1.size() < bad_code.size() && std::equal(BAD_1_1.begin(), BAD_1_1.end(), bad_code.begin() + i))
        {
            uint32_t pp_count = 1;
            auto offset = i;


            puts("");
            puts("OPTIMIZED:");
            puts("");
            print_code(new_code);
            puts("");

            for(size_t j = i + BAD_1_1.size(); j < bad_code.size(); ++j)
            {
                if (bad_code[j].id == PUSH_)
                    ++pp_count;
                else if (bad_code[j].id == POP_)
                    --pp_count;

                auto part_3_begin = bad_code.begin() + j + BAD_1_2.size();
                if (pp_count == 0 && std::equal(BAD_1_2.begin(), BAD_1_2.end(), bad_code.begin() + j)
                    && std::equal(BAD_1_3.begin(), BAD_1_3.end(), part_3_begin + 1))
                {
                    auto save = bad_code[i-1];
                    std::string *p_id = bad_code[i+2].p_s;
                    for (i += BAD_1_1.size() + 1; i <= j - offset; ++i)
                        new_code.emplace_back(bad_code[i]);

                    auto tmp = new_code.size();
                    new_code.emplace_back(save);

                    new_code.insert(new_code.end(), GOOD_1_1.begin(), GOOD_1_1.end());
                    new_code.insert(new_code.end(), part_3_begin, part_3_begin + GOOD_1_2.size() + 1);
                    new_code[tmp+3].p_s = p_id;
                    i+= BAD_1_2.size() + BAD_1_3.size();

                    is_optimized = true;
                    goto CONTINUE;
                }
            }
        }
        else if (BAD_2_1.size() < bad_code.size() && std::equal(BAD_2_1.begin(), BAD_2_1.end(), bad_code.begin() + i))
        {
            if (std::equal(BAD_2_2.begin(), BAD_2_2.end(), bad_code.begin() + i + 1))
            {
                auto tmp = new_code.size();
                new_code.emplace_back(*(bad_code.begin() + i));
                new_code.insert(new_code.end(), GOOD_2_1.begin(), GOOD_2_1.end());
                new_code[tmp + 3].p_s = bad_code[i + 6].p_s;

                i += BAD_2_1.size() + GOOD_2_1.size();

                is_optimized = true;
                continue;
            }
        }
        else if (BAD_3_1.size() < bad_code.size() && std::equal(BAD_3_1.begin(), BAD_3_1.end(), bad_code.begin() + i))
        {
            Code operation = bad_code[i + BAD_3_1.size()];
            if (operation.id == ADD_ || operation.id == IMUL_)
            {
                if (std::equal(GOOD_3.begin(), GOOD_3.end(), bad_code.begin() + i + BAD_3_1.size() + 1))
                {
                    new_code.emplace_back(operation);
                    auto tmp = new_code.size();
                    new_code.insert(new_code.end(), GOOD_3.begin(), GOOD_3.end());
                    is_optimized = true;
                    i += BAD_3_1.size() + GOOD_3.size();
                    goto CONTINUE;
                }

            }
        }
        else if (BAD_4_1.size() < bad_code.size() && std::equal(BAD_4_1.begin(), BAD_4_1.end(), bad_code.begin() + i))
        {
            Code operation = bad_code[i + BAD_4_1.size()];
            if (operation.id == ADD_ || operation.id == IMUL_ || operation.id == SUB_)
            {
                if (std::equal(BAD_4_2.begin(), BAD_4_2.end(), bad_code.begin() + i + BAD_4_1.size() + 1))
                {
                    new_code.emplace_back(operation);
                    new_code.insert(new_code.end(), GOOD_4.begin(), GOOD_4.end());
                    is_optimized = true;
                    new_code[new_code.size() - 2].p_s = bad_code[i+3].p_s;
                    i += BAD_4_1.size() + GOOD_4.size();
                    goto CONTINUE;
                }
            }
        }

        new_code.emplace_back(bad_code[i]);
CONTINUE: true == true;
    }

    if (is_optimized)
        new_code = optimize(new_code);

    return new_code;
}

int main()
{
    Tree tree;
    const char *expr = "A=C-(A+13)*D";
    if (not tree.init(expr, strlen(expr)))
        return -1;

    auto codes = get_code(tree.p_head);
    print_code(codes);

    auto codes_optimized = optimize(codes);

    puts("");
    puts("OPTIMIZED:");
    puts("");
    print_code(codes_optimized);
}
