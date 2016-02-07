# Types example:
#  - tuple type
#  - array type

type Message = ( to_address : String, content : String )

procedure main():
    let msg = Message("some@developer.eu", "content")
    let a = [ "an", "array", "of", "strings" ]

    stdout.println ! ( "An message for " + msg.to_address + ": " + msg.content )
    stdout.printfln ! ( "%s %s %s %s", a[0], a[1], a[2], a[3] )

# TODO: optional/forced types
# TODO: printfln -> format
