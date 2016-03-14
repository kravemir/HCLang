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
        if str_num < 1:
            var testAlloca = "1st string: " + str
            stdout.println ! ( testAlloca )
        elif str_num < 2:
            var testAlloca = "2nd string: " + str
            stdout.println ! ( testAlloca )
        else:
            var testAlloca = "X_" + str + "_X"
            stdout.println ! ( testAlloca )


        # update content of variables
        str_concat = str_concat + str
        str_num = str_num + 1

        # debug print results
        stdout.println ! ( sprintf("Total %d strings printed, concat: %s", str_num, str_concat) )
