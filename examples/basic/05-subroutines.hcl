fn min( a : int, b : int ) int => if a < b then a else b

procedure main():
    let x = 3
    let y = 5

    stdout.println ! ( sprintf("min(x,y) = %d", min(x,y)) )
