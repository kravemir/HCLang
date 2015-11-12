# simple counter example, point is thread safe access to shared state
system Counter:
    var count : int

    # return void - nothing, but make it synchronous
    slot init( start : int ) void:
        self.count = start

    # returns int
    slot next() int:
        self.count = self.count + 1
        return self.count

# some worker that needs to use shared state
procedure work( counter : Counter, worker : int ) int async:
    var total : int = 0
    for i in range(0,1000):
        let current = await controller ! next()
        total = total + current
        stdout ! printfln( "Extra %d %d", worker, current )
    stdout ! printfln( "Worker %d done %d", worker, total )
    return total

procedure main() async:
    # spawn counter
    let counter = spawn Counter
    
    # serial run
    await counter ! init( 0 )
    let ress_1 = work(counter,1)
    let ress_2 = work(counter,2)
    let ress_3 = work(counter,3)
    let ress_4 = work(counter,4)
    let results_serial : int[] = [ ress_1, ress_2, ress_3, ress_4 ]

    # parallel run
    await counter ! init( 0 )
    let resp_1 = spawn work(counter,1)
    let resp_2 = spawn work(counter,2)
    let resp_3 = spawn work(counter,3)
    let resp_4 = spawn work(counter,4)
    let results_parallel : int[] = [ await ress_1, await ress_2, await ress_3, await ress_4 ]

    # both executions should give the same result
    assert( sum(results_serial) == sum(results_parallel) )
