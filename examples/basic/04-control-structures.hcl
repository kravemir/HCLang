# Control structures example
#  - for
#  - while

procedure main():
    let numbers = [ 1, 2, 3, 5, 8, 13, 21, 34 ]
    for n in numbers:
        stdout.println ! ( sprintf("%d", n) )

# TODO: printfln -> int to string