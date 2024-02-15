#pragma once


typedef enum {
    Empty       = 0,
    Blue        = 1,
    Green       = 2,
    Orange      = 3,
    Purple      = 4,
    Red         = 5,
    Turqoise    = 6,
    Yellow      = 7
} BlockColor;

// blue
#define E Empty
#define B Blue
#define G Green
#define O Orange
#define P Purple
#define R Red
#define T Turqoise
#define Y Yellow

 BlockColor I_block[4][4][4] = {
    {
        {E, E, E, E},
        {E, E, E, E},
        {B, B, B, B},
        {E, E, E, E}
    },
    {
        {E, B, E, E},
        {E, B, E, E},
        {E, B, E, E},
        {E, B, E, E}
    },
    {
        {E, E, E, E},
        {B, B, B, B},
        {E, E, E, E},
        {E, E, E, E}
    },
    {
        {E, E, B, E},
        {E, E, B, E},
        {E, E, B, E},
        {E, E, B, E}
    }
};

 BlockColor J_block[4][4][4] = {
    {
        {E, E, E, E},
        {E, G, E, E},
        {E, G, G, G},
        {E, E, E, E}
    },
    {
        {E, E, E, E},
        {E, E, G, E},
        {E, E, G, E},
        {E, G, G, E}
    },
    {
        {E, E, E, E},
        {G, G, G, E},
        {E, E, G, E},
        {E, E, E, E}
    },
    {
        {E, G, G, E},
        {E, G, E, E},
        {E, G, E, E},
        {E, E, E, E}
    }
};

 BlockColor L_block[4][4][4] = {
    {
        {E, E, E, E},
        {E, E, P, E},
        {P, P, P, E},
        {E, E, E, E}
    },
    {
        {E, P, P, E},
        {E, E, P, E},
        {E, E, P, E},
        {E, E, E, E}
    },
    {
        {E, E, E, E},
        {E, E, P, E},
        {P, P, P, E},
        {E, E, E, E},
    },
    {
        {E, P, E, E},
        {E, P, E, E},
        {E, P, P, E},
        {E, E, E, E}
    }
};

 BlockColor O_block[4][4][4] = {
    {
        {E, E, E, E},
        {E, Y, Y, E},
        {E, Y, Y, E},
        {E, E, E, E}
    },
    {
        {E, E, E, E},
        {E, Y, Y, E},
        {E, Y, Y, E},
        {E, E, E, E}
    },
    {
        {E, E, E, E},
        {E, Y, Y, E},
        {E, Y, Y, E},
        {E, E, E, E}
    },
    {
        {E, E, E, E},
        {E, Y, Y, E},
        {E, Y, Y, E},
        {E, E, E, E}
    }
};

 BlockColor S_block[4][4][4] = {
    {
        {E, E, E, E},
        {E, E, T, T},
        {E, T, T, E},
        {E, E, E, E}
    },
    {
        {E, T, E, E},
        {E, T, T, E},
        {E, E, T, E},
        {E, E, E, E}
    },
    {
        {E, E, E, E},
        {E, E, T, T},
        {E, T, T, E},
        {E, E, E, E}
    },
    {
        {E, T, E, E},
        {E, T, T, E},
        {E, E, T, E},
        {E, E, E, E}
    }
};

 BlockColor T_block[4][4][4] = {
    {
        {E, E, E, E},
        {E, R, E, E},
        {R, R, R, E},
        {E, E, E, E}
    },
    {
        {E, R, E, E},
        {E, R, R, E},
        {E, R, E, E},
        {E, E, E, E}
    },
    {
        {E, E, E, E},
        {R, R, R, E},
        {E, R, E, E},
        {E, E, E, E}
    },
    {
        {E, R, E, E},
        {R, R, E, E},
        {E, R, E, E},
        {E, E, E, E}
    }
};

 BlockColor Z_block[4][4][4] = {
    {
        {E, E, E, E},
        {O, O, E, E},
        {E, O, O, E},
        {E, E, E, E}
    },
    {
        {E, E, O, E},
        {E, O, O, E},
        {E, O, E, E},
        {E, E, E, E}
    },
    {
        {E, E, E, E},
        {O, O, E, E},
        {E, O, O, E},
        {E, E, E, E}
    },
    {
        {E, E, O, E},
        {E, O, O, E},
        {E, O, E, E},
        {E, E, E, E}
    }
};

