fun fact() : int {
    ret 2;
}

fun main() : int {
    let x : int = 1*2+3;
    
    if 0 {
        dbg 1;
    } else {
        dbg 2;
    }
    
    let y: int = fact();
    dbg y;
    dbg 3;
    
    ret 0;
}
