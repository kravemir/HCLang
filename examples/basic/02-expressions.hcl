# Expressions example:
#  - arithmetic expressions
#  - string expressions (TODO)

procedure main():
    let a = 7
    let b = 5

    stdout.println ! ( sprintf("a + b = %d", a + b) )
    stdout.println ! ( sprintf("a - b = %d", a - b) )
    stdout.println ! ( sprintf("a * b = %d", a * b) )
    stdout.println ! ( sprintf("a / b = %d", a / b) )
