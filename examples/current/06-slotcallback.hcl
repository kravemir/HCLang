system Counter:
    var count : int

    slot init( start : int ):
        self.count = start
        stdout ! printfln( "Counter initialized at %d", start )

    slot request( callback : slot(result:int) ):
        stdout ! printfln( "Served %d", self.count )
        callback ! (self.count)
        self.count = self.count + 1

system Requester:
    var id : int
    var counter : Counter

    slot init( id : int, counter : Counter ):
        self.id = id
        self.counter = counter
        self.counter ! request( self.response )
        
    slot response( n : int ):
        stdout ! printfln( "#%d received %d", self.id, n )
        if n < 30:
            self.counter ! request( self.response )

procedure main():
    # spawn counter
    let counter = spawn Counter
    counter ! init ( 0 )

    # spawn requesters
    let requester1 = spawn Requester
    let requester2 = spawn Requester
    let requester3 = spawn Requester
    let requester4 = spawn Requester
    let requester5 = spawn Requester
    let requester6 = spawn Requester
    requester1 ! init ( 1, counter )
    requester2 ! init ( 2, counter )
    requester3 ! init ( 3, counter )
    requester4 ! init ( 4, counter )
    requester5 ! init ( 5, counter )
    requester6 ! init ( 6, counter )
