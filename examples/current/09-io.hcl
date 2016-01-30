procedure main2() async:
    let stdina = spawn StdIn
    let stdouta = spawn StdOut
    stdouta.println ! ( stdina.readln () )


# main procedure is executed when program starts
procedure main():
    spawn main2 ()
