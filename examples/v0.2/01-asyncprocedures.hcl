# simple procedure that just prints stuff
procedure print(start : int, end : int):
    stdout ! printfln( "%d %d", start, end )

# similar to previous, but gets compiled as asynchronous procedure
procedure print_async(start : int, end : int) async:
    stdout ! printfln( "%d %d", start, end )

procedure main():
    print(1,10)
    spawn print_async (2,20)
    print(3,30)
