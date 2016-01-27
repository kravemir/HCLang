# HelloWorld example showing


system HelloWorld:
    slot hello():
        stdout ! println("HelloWorld")


# main procedure is executed when program starts
procedure main():
    # create new instance of HelloWorld system
    let helloWorld = spawn HelloWorld

    # send hello message to the system
    helloWorld ! hello()
