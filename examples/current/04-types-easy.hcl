type Msg     = ( a : String, b : String )

fn msgToString( m : Msg) String => m.a + m.b


procedure main():
    let msgs = [
        Msg("first ", "message"),
        Msg("second ", "extra message"),
        Msg("yet ", "third message"),
    ]
    for msg in msgs:
        stdout ! println( msgToString(msg))
