
include 'lang.inc' ; Language support for locales: ru_RU (CP866), en_US.

if lang eq ru_RU

text		db ' �唠��:','��唠��:','   ����:'
	.line_size = ($-text)/3

s_compile	db '������.'
s_run		db ' ���  '
s_debug 	db '�⫠���'
s_dbgdescr	db '��������� �⫠����� ���ଠ��',0

sz_EPnotFound	db 'Not found entry point to ',0
sizeof.sz_EPnotFound = $-sz_EPnotFound

sz_cantLL	db 'Cannot load library ',0
sizeof.sz_cantLL = $-sz_cantLL

sz_system_error db 'System error: ',0
sizeof.sz_system_error = $-sz_system_error

else ; Default to en_US

text	db    ' InFile:','OutFile:','   Path:'
	.line_size = ($-text)/3

s_compile	db    'COMPILE'
s_run		db    '  RUN  '
s_debug 	db    ' DEBUG '
s_dbgdescr	db    'Generate debug information',0

sz_EPnotFound	db 'Not found entry point to ',0
sizeof.sz_EPnotFound = $-sz_EPnotFound

sz_cantLL	db 'Cannot load library ',0
sizeof.sz_cantLL = $-sz_cantLL

sz_system_error db 'System error: ',0
sizeof.sz_system_error = $-sz_system_error

end if
