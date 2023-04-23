fun fact() : int {
    ret 2;
}

fun main() : int {
    let x : int = 1*2+3;
    
    if 1 {
        let x : short = 2;
        if 1 {
            let x : short = 1;
            dbg x; 
        } else {
            dbg 0;
        }
    } else { 
        let y : short = 0; 
        dbg y;
    }
    
    let y: int = fact();
    dbg y;
    dbg 3;
    
    ret 0;
}