procedure main():
    # declare variables with implicit type
    var str_concat = ""
    var str_num = 0

    let strs = [
        "AAAA",
        "BBBB",
        "CCCC"
    ]

    for str in strs:
        # update content of variables
        str_concat = str_concat + str
        str_num = str_num + 1

        # debug print results
        stdout ! println( str )
        stdout ! printfln( "Total %d strings printed, concat: %s", str_num, str_concat )
