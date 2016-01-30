procedure main2() async:
    let stdina = spawn StdIn
    let stdouta = spawn StdOut
    let x = stdina.readln ()
    stdouta.println ! ( x )


# main procedure is executed when program starts
procedure main():
    spawn main2 ()
