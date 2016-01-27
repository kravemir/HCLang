type Message = ( to_address : String, content : String )

procedure main():
    let msgs = [
        Message("some@developer.eu", "1st message"),
        Message("enthusiast-in@lowlevel-programming.sk", "2nd message"),
        Message("loves@eazy-highlevel-programming.com", "3rd message"),
    ]

    for msg in msgs:
        stdout.println ! ( "An message for " + msg.to_address + ": " + msg.content )
