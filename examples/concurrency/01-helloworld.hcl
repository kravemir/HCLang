# HelloWorld example showing:
#  - system instantiation
#  - message sending

system HelloWorld:
    var stdout : StdOut
    slot init(stdout : StdOut):
        self.stdout = stdout
    slot hello():
        self.stdout.println ! ("HelloWorld")

# main procedure is executed when program starts
procedure main():
    # create new instance of HelloWorld system
    let helloWorld = spawn HelloWorld

    # init and send hello message to the system
    helloWorld.init ! (stdout)
    helloWorld.hello ! ()
