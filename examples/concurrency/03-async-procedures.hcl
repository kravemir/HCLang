procedure print_normal(stdout : StdOut, str : String):
    stdout.println ! ( str )

procedure print_async(stdout : StdOut, str : String) async:
    stdout.println ! (str)

procedure main():
    print_normal(stdout, "1st print")
    spawn print_async (stdout, "2nd print")
    print_normal(stdout, "3rd print")
