# system state example


system Counter:
    var count : int
    var max : int

    slot init(max : int):
        self.count = 0
        self.max = max

    slot next():
        self.count = self.count + 1

        if self.count > self.max:
            self.count = self.max

        stdout.printfln ! ("Counter state is: %d.", self.count)


system CounterController:
    var counter : Counter

    slot run():
        self.counter = spawn Counter
        self.counter.init ! (7)
        self.counting ! (10)

    slot counting( n : int ):
        self.counter.next ! ()
        stdout.printfln ! ("Controller %d", n)
        if 1 < n:
            self.counting ! ( n-1 )


procedure main():
    let controller = spawn CounterController
    controller.run ! ()