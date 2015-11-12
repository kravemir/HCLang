type Msg     = ( a : String, b : String )
type TestUnion = Msg | String

fn msgToString( m : Msg) String => m.a + m.b
fn unionToString ( u : TestUnion ) String:
    result = match u:
        case Msg => msgToString(u)
        case String => u
    return result

procedure main():
    let msgs = [
        Msg("first ", "message"),
        Msg("second ", "extra message"),
        Msg("yet ", "third message"),
    ]
    for msg in msgs:
        stdout ! println( msgToString(msg))

    let unions : TestUnion[] = [
        "String value for TestUnion",
        Msg("msg", "test"),
    ]
    for u in unions:
        stdout ! println( unionToString(u))
