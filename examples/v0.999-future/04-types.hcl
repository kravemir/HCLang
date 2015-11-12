type Circle     = ( radius : int )
type Square     = ( length : int )
type Rectangle  = ( width : int, height : int )
type Shape      = Circle | Square | Rectangle

# self recursive structures
# TODO: indirect recursive structure
type XMLNode = { String : ( String | XMLNode ) }

fn toShape( element : XMLNode ) Shape:
    let result = match element['type']:
        'circle'    => Circle ( element['radius'] )
        'square'    => Square ( element['length'] )
        'rectangle' => Rectangle ( element['width'], element['height'] )
    return result

fn toString ( shape : Shape ) String:
    let result = match shape:
        Circle ( r )        => 'circle : {0}' % r
        Square ( l )        => 'square : {0}' % l
        Rectangle ( w, h )  => 'rectangle : {0}, {1}' % ( w, h )
    return result

procedure main():
    let elements : XMLNode[] = [
        { 'type' : 'circle', 'radius' : 2 },
        { 'type' : 'square', 'length' : 5 },
        { 'type' : 'rectangle', 'width' : 3, 'height' : 2 }
    ]
    stdout ! print( '\n'.join(elements.map(toString).map(toShape)))
