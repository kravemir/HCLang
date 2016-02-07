procedure print_normal(str : String):
    stdout.println ! ( str )

procedure print_async(str : String) async:
    stdout.println ! (str)

procedure main():
    print_normal("1st print")
    spawn print_async ("2nd print")
    print_normal("3rd print")
