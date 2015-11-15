system Counter:
    var count : int

    slot init( start : int ):
        self.count = start
        stdout ! printfln( "Counter initialized at %d", start )

    slot request( callback : slot(int) ):
        callback ! (self.count)
        self.count = self.count + 1

system Requester:
    var id : int
    var counter : Counter

    slot init( id : int, counter : Counter ):
        self.counter = counter
        self.counter ! request( self.response )
        
    slot response( n : int );
        stdout ! printfln( "#%d received %d", self.id, n )
        if n < 30:
            self.counter ! request( self.response )

procedure main() async:
    # spawn counter
    let counter = spawn Counter
    counter ! init ( 0 )

    # spawn requesters
    let requester1 = spawn Requester
    let requester2 = spawn Requester
    requester1 ! init ( 1, counter )
    requester2 ! init ( 2, counter )
