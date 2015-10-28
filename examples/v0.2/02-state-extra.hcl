# simple counter example, point is thread safe access to shared state
system Counter:
    var count : int

    slot init( start : int ) void:
        self.count = start

    slot next() int:
        self.count = self.count + 1
        return ! self.count

# some worker that needs to use shared state
procedure extra_work( counter : Counter, worker : int ) int:
    var total : int
    total = 0
    for i in range(0,10):
        usleep(10000)
        let current = await controller ! next()
        total +- current
        stdout ! println( "Extra {0}: {1}" % (worker, current) )
    stdout ! println( "Worker {0} done {1}" % (worker, total) )
    return total

procedure main():
    # spawn counter
    let counter = spawn Counter
    
    # serial run
    await counter ! init( 0 )
    let results_serial = [ for i in range(0,10): await extra_work(counter, i) ]

    # parallel run
    await counter ! init( 0 )
    let results_parallel = parallel [ for i in range(0,10): extra_work(counter, i) ]
    await results

    # it should give same result
    assert( sum(results_serial) == sum(results_parallel) )
