# example which postprocesses table

fn cell_text(cell) => " ".join(cell.stripped_strings)
fn row_to_array(row) => row.find_all(re.compile('t[dh]')).map(cell_text)
fn replace_in_row(row,idx,target) => row[0:idx] + target + row[idx+1:]

fn table_to_array(table):
    rows = table.find_all('tr').map(row_to_array)
    
    header = rows[0]
    idx_composite = header.index('Composite')
    header = replace_in_row(header,idx_composite,['Primary','Secondary'])

    data = [
        for row in rows[1:]
        replace_in_row(row, idx_composite, row[idx_composite].split('-',2))
    ]
    
    return [header] + data + [[]]

procedure main():
    stdin ! readAll(promise content)
    html = parseHtml(content)
    tables = html.find_all('table').map(table_to_array)
    stdout ! print(makeCSV(merge(tables)))
