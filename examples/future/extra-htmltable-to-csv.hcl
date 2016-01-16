# Simple example which convertes HTML table to CSV

# join stripped text strings
fn cell_text(cell) => " ".join(cell.strings.map(strip))

# creates array of cells from found columns
fn row_to_array(row) => row.find_all(re.compile('t[dh]')).map(cell_text)

# creates flat(array of arrays) representation of the provided table
fn table_to_array(table) => table.find_all('tr').map(row_to_array) + []

procedure main():
    stdin ! readAll(promise content)
    html = parseHtml(content)
    tables = html.find_all('table').map(table_to_array)
    stdout ! print(makeCSV(merge(tables)))
