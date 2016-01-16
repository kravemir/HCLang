# HelloWorld example showing:
#  * system instantiation 
#  * message sending

system HelloWorld:
    slot hello():
        self    ! info_a ()
        stdout  ! println("HelloWorld")
        self    ! info_x ( "B" )

    slot info_a():
        stdout  ! println("Info A")
        self    ! info_x( "C" )

    slot info_x(x : String):
        stdout  ! println("Info " + x)

procedure main():
    let helloWorld = spawn HelloWorld
    helloWorld ! hello()
