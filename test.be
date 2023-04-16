fun abc(a: int, b: int, c: int) : long {
    let d: long = a * b;
    if a {
        d = d + c;
    }
    else {
        d = d * c;
    }
    ret d * d;
}

fun main() : int {
    let a: int = 5;
}