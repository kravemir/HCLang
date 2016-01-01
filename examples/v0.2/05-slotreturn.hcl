system Counter:
    var count : int

    slot init( start : int ) void:
        self.count = start

    slot next() int:
        self.count = self.count + 1
        return self.count

procedure main() async:
    # spawn counter
    let counter = spawn Counter

    counter.init( 0 )
    stdout ! printfln( "%d", counter.next() )
    stdout ! printfln( "%d", counter.next() )
    stdout ! printfln( "%d", counter.next() )
    stdout ! printfln( "%d", counter.next() )
