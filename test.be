#def LOL 10
#def LEL 20
let x = 1;
let y = 2;

#undef LOL

#ifdef LOL
dbg x;
dbg x;
#elif LEL
dbg y;
#else
dbg 3;
#endif
