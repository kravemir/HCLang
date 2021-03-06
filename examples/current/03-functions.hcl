fn min( a : int, b : int ) int => if a < b then a else b

# TODO: rework as while/for loop
system LimitedAccelerator:
    var speed : int
    var max_speed : int

    slot run():
        self.speed = 0
        self.max_speed = 12
        self.accelerate ! (20)

    slot accelerate ( times : int ):
        self.speed = min( self.speed + 1, self.max_speed )
        stdout.printfln ! ( "Current speed: %d", self.speed )

        if times > 1:
            self.accelerate ! ( times - 1 )

procedure main():
    let accelerator = spawn LimitedAccelerator
    accelerator.run ! ()
