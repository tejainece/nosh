#!/usr/bin/python3

import sys
import csv
import xlsxwriter

if len(sys.argv) != 3:
    print("Usage: report.py <test-file-path> <module-name>")
    exit(1)

filename = sys.argv[1]
moduleName = sys.argv[2]

rows = []

with open(filename, 'r') as csvfile:
    input = csv.reader(csvfile)
    for row in input:
        rows.append(row)

workbook = xlsxwriter.Workbook('report.xlsx')
worksheet = workbook.add_worksheet(name=moduleName)

heading = workbook.add_format()
heading.set_bold()
heading.set_align('center')
heading.set_bg_color('#efefef')
heading.set_border()

table = workbook.add_format()
table.set_border()

nameFormat = workbook.add_format()
nameFormat.set_border()
nameFormat.set_bold()

descFormat = workbook.add_format()
descFormat.set_border()
descFormat.set_text_wrap()

#worksheet.set_row(0, 0, cell_format=heading)

worksheet.set_column(2, 2, 20)
worksheet.set_column(4, 4, 550)

worksheet.write(1, 1, "#", heading)
worksheet.write(1, 2, "Name", heading)
worksheet.write(1, 3, "Status", heading)
worksheet.write(1, 4, "Description", heading)

rowIdx = 2

for row in rows:
    worksheet.write(rowIdx, 1, rowIdx-1, table)
    worksheet.write(rowIdx, 2, row[0], nameFormat)
    worksheet.write(rowIdx, 3, row[1], table)
    worksheet.write(rowIdx, 4, row[2], descFormat)
    rowIdx += 1

passed = workbook.add_format()
passed.set_font_color('green')
passed.set_bold()

worksheet.conditional_format('D3:D' + str(rowIdx), {'type':     'cell',
                                        'criteria': '=',
                                        'value':    '"Pass"',
                                        'format':   passed})
rowIdx += 1

failed = workbook.add_format()
failed.set_font_color('red')
failed.set_bold()

worksheet.conditional_format('D3:D' + str(rowIdx), {'type':     'cell',
                                                    'criteria': '=',
                                                    'value':    '"Fail"',
                                                    'format':   failed})

workbook.close()
