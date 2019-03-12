#pragma once

enum State
{
    SPACE,
    ID_NAME, ID, CONST, NEGATE,
    OPERATOR, BRACKET_LEFT, BRACKET_RIGHT,
    AND_a, AND_n, AND_d, OR_o, OR_r,
    COUNT, ERROR = -1
};

struct Chars
{
    [[nodiscard]] inline char operator[](unsigned long i) const { return chars[i]; }

    const char    *chars;
    unsigned long  count;
};

constexpr static const Chars CHARS[State::COUNT] =
{
    { " \n",        2 },
    { "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 },
    { "0123456789", 10 },
    { "0123456789", 10 },
    { "-",          10 },
    { "+-/*=",      5  },
    { "(",          1  },
    { ")",          1  },
    { "a",          1  },
    { "n",          1  },
    { "d",          1  },
    { "o",          1  },
    { "r",          1  },
};

struct Vertex
{
    template<State ...states>
    constexpr static Vertex build(State src)
    {
        constexpr auto count = sizeof...(states);
        return { src, count, states... };
    }
    State src;
    unsigned int dst_count;
    State dst[COUNT];
};

constexpr static const Vertex VERTICES[COUNT] =
{
    Vertex::build<SPACE, ID_NAME, CONST, NEGATE>(SPACE),
    Vertex::build<ID, ID_NAME, OPERATOR, AND_a, OR_o, BRACKET_RIGHT, SPACE>(ID_NAME),
    Vertex::build<ID, OPERATOR, AND_a, OR_o, SPACE>(ID),
    Vertex::build<CONST, SPACE, OPERATOR, BRACKET_RIGHT>(CONST),
    Vertex::build<ID_NAME, NEGATE, BRACKET_LEFT, CONST>(OPERATOR),
    Vertex::build<ID_NAME, BRACKET_LEFT, CONST>(NEGATE),
    Vertex::build<ID_NAME, BRACKET_LEFT, NEGATE, CONST>(BRACKET_LEFT),
    Vertex::build<OPERATOR, AND_a, OR_o, BRACKET_RIGHT, SPACE>(BRACKET_RIGHT),
    Vertex::build<AND_n>(AND_a),
    Vertex::build<AND_d>(AND_n),
    Vertex::build<ID_NAME, BRACKET_LEFT, CONST, SPACE>(AND_d),
    Vertex::build<OR_r>(OR_o),
    Vertex::build<ID_NAME, BRACKET_LEFT, CONST, SPACE>(OR_r),
};


[[nodiscard]] inline State update_state(State current, char c)
{
    for (auto &ver : VERTICES)
    {
        if (ver.src == current)
        {
            for (unsigned int i = 0; i < ver.dst_count; ++i)
            {
                const auto &chars = CHARS[ver.dst[i]];
                for (unsigned int c_i = 0; c_i < chars.count; ++c_i)
                    if (c == chars[c_i])
                        return ver.dst[i];
            }
        }
    }
    return State::ERROR;
}
