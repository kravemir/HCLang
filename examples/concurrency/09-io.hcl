procedure main2(stdoutt : StdOut) async:
    # TODO: fix
    let stdout : StdOut= stdoutt
    let stdina = spawn StdIn
    stdout.println ! (  stdina.readln() )


# main procedure is executed when program starts
procedure main():
    spawn main2 (stdout)
