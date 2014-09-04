# -*- coding: iso-8859-1 -*-


import trap

#---------------------------------------------------------
# Instruction Encoding
#---------------------------------------------------------
# Lets now start with defining the instructions, i.e. their bitstring and
# mnemonic and their behavior. Note the zero* field: it is a special identifier and it
# means that all those bits have value 0; the same applies for one*
# As stated in page 44 of "The SPARC Architecture Manual V8" there are
# mainly 6 different format types

# Call instruction format
call_format = trap.MachineCode([('op', 2), ('disp30', 30)])
call_format.setBitfield('op', [0, 1])

# Branch and sethi instructions format
b_sethi_format1 = trap.MachineCode([('op', 2), ('rd', 5), ('op2', 3), ('imm22', 22)])
b_sethi_format1.setBitfield('op', [0, 0])
b_sethi_format1.setVarField('rd', ('REGS', 0), 'out')
b_sethi_format2 = trap.MachineCode([('op', 2), ('a', 1), ('cond', 4), ('op2', 3), ('disp22', 22)])
b_sethi_format2.setBitfield('op', [0, 0])

# Memory instruction format
mem_format1 = trap.MachineCode([('op', 2), ('rd', 5), ('op3', 6), ('rs1', 5), ('zero', 1), ('asi', 8), ('rs2', 5)])
mem_format1.setBitfield('op', [1, 1])
mem_format1.setVarField('rs1', ('REGS', 0), 'in')
mem_format1.setVarField('rs2', ('REGS', 0), 'in')

mem_format2 = trap.MachineCode([('op', 2), ('rd', 5), ('op3', 6), ('rs1', 5), ('one', 1), ('simm13', 13)])
mem_format2.setBitfield('op', [1, 1])
mem_format2.setVarField('rs1', ('REGS', 0), 'in')

# Store Barrier format
stbar_format = trap.MachineCode([('op', 2), ('zero', 5), ('op3', 6), ('rs1', 5), ('zero', 14)])
stbar_format.setBitfield('op', [1, 0])
stbar_format.setBitfield('op3', [1, 0, 1, 0, 0, 0])
stbar_format.setBitfield('rs1', [0, 1, 1, 1, 1])

# logical and remainig instructions format
dpi_format1 = trap.MachineCode([('op', 2), ('rd', 5), ('op3', 6), ('rs1', 5), ('zero', 1), ('asi', 8), ('rs2', 5)])
dpi_format1.setBitfield('op', [1, 0])
dpi_format1.setVarField('rd', ('REGS', 0), 'out')
dpi_format1.setVarField('rs1', ('REGS', 0), 'in')
dpi_format1.setVarField('rs2', ('REGS', 0), 'in')

dpi_format2 = trap.MachineCode([('op', 2), ('rd', 5), ('op3', 6), ('rs1', 5), ('one', 1), ('simm13', 13)])
dpi_format2.setBitfield('op', [1, 0])
dpi_format2.setVarField('rd', ('REGS', 0), 'out')
dpi_format2.setVarField('rs1', ('REGS', 0), 'in')

# Format for reading special instructions
read_special_format = trap.MachineCode([('op', 2), ('rd', 5), ('op3', 6), ('asr', 5), ('zero', 14)])
read_special_format.setBitfield('op', [1, 0])
read_special_format.setVarField('rd', ('REGS', 0), 'out')

# Format for writing special instructions
write_special_format1 = trap.MachineCode([('op', 2), ('rd', 5), ('op3', 6), ('rs1', 5), ('zero', 9), ('rs2', 5)])
write_special_format1.setBitfield('op', [1, 0])
write_special_format1.setVarField('rs1', ('REGS', 0), 'in')
write_special_format1.setVarField('rs2', ('REGS', 0), 'in')

write_special_format2 = trap.MachineCode([('op', 2), ('rd', 5), ('op3', 6), ('rs1', 5), ('one', 1), ('simm13', 13)])
write_special_format2.setBitfield('op', [1, 0])
write_special_format2.setVarField('rs1', ('REGS', 0), 'in')

# Trap on integer condition code format
ticc_format1 = trap.MachineCode([('op', 2), ('reserved1', 1), ('cond', 4), ('op3', 6), ('rs1', 5), ('zero', 1), ('asi', 8), ('rs2', 5)])
ticc_format1.setBitfield('op', [1, 0])
ticc_format1.setVarField('rs1', ('REGS', 0), 'in')
ticc_format1.setVarField('rs2', ('REGS', 0), 'in')

ticc_format2 = trap.MachineCode([('op', 2), ('reserved1', 1), ('cond', 4), ('op3', 6), ('rs1', 5), ('one', 1), ('reserved2', 6), ('imm7', 7)])
ticc_format2.setBitfield('op', [1, 0])
ticc_format2.setVarField('rs1', ('REGS', 0), 'in')

# Coprocessor of fpu instruction format
coprocessor_format = trap.MachineCode([('op', 2), ('rd', 5), ('op3', 6), ('rs1', 5), ('opf', 9), ('rs2', 5)])
coprocessor_format.setBitfield('op', [1, 0])
coprocessor_format.setVarField('rd', ('REGS', 0), 'out')
coprocessor_format.setVarField('rs1', ('REGS', 0), 'in')
coprocessor_format.setVarField('rs2', ('REGS', 0), 'in')
