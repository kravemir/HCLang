# system state example


system Counter:
    var count : int
    var max : int
    var stdout : StdOut

    slot init(max : int, stdout : StdOut):
        self.count = 0
        self.max = max
        self.stdout = stdout

    slot next():
        self.count = self.count + 1

        if self.count > self.max:
            self.count = self.max

        self.stdout.println ! ("Counter state is: %d.".format(self.count))


system CounterController:
    var counter : Counter
    var stdout : StdOut

    slot run(stdout : StdOut):
        self.stdout = stdout
        self.counter = spawn Counter
        self.counter.init ! (7,stdout)
        self.counting ! (10)

    slot counting( n : int ):
        self.counter.next ! ()
        self.stdout.println ! ("Controller %d".format(n))
        if 1 < n:
            self.counting ! ( n-1 )


procedure main():
    let controller = spawn CounterController
    controller.run ! (stdout)