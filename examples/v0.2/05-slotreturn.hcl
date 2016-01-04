system Counter:
    var count : int

    slot init( start : int ) void:
        self.count = start

    slot next() int:
        self.count = self.count + 1
        stdout ! printfln("Counter received: %d", self.count)
        return self.count

procedure main2() async:
    # spawn counter
    let counter = spawn Counter

    counter.init( 0 )
    stdout ! printfln( "%d", counter.next() )

procedure main():
    spawn main2()
