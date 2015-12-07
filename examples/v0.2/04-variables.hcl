procedure main():
    var str_concat = ""
    let strs = [
        "AAAA",
        "BBBB",
        "CCCC"
    ]
    for str in strs:
        str_sum = str_sum + str
        stdout ! println( str )
        stdout ! println( str_sum )
