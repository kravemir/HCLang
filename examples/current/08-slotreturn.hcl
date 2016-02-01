system Counter:
    var count : int

    slot init( start : int ) int:
        self.count = start
        return 1

    slot next() int:
        self.count = self.count + 1
        stdout.printfln ! ("Counter sending: %d", self.count)
        return self.count

procedure main2() async:
    # spawn counter
    let counter = spawn Counter

    counter.init( 0 )
    stdout.printfln !( "Counter Initialized" )

    stdout.printfln !( "Procedure received: %d", counter.next() )
    stdout.printfln !( "Procedure received: %d", counter.next() )
    stdout.printfln !( "Procedure received: %d", counter.next() )
    stdout.printfln !( "Procedure received: %d", counter.next() )

procedure main():
    spawn main2()
