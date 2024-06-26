;
; �㭪樨 ��� �८�ࠧ������ 䠩�� *.stl � *.3ds
;
; ������� ᮧ��������� 䠩�� *.3ds:
; CHUNK_MAIN (40+n+v+f)
; + CHUNK_OBJMESH (34+n+v+f)
;   + CHUNK_OBJBLOCK (28+n+v+f)
;     + CHUNK_TRIMESH (22+v+f)
;       + CHUNK_VERTLIST (8+v)
;       + CHUNK_FACELIST (8+f)
;
; � ᪮���� 㪠���� ࠧ���� ������:
;   n - ������ ��� ����� ��ꥪ�
;   v - ������ ��� ���設
;   f - ������ ��� �࠭��

; ��ଠ� ����୮�� *.stl:
; char[80] - ���������
; uint32 - �᫮ �࠭��
; ��� ������ �࠭�:
; float[3] - ����� ��ଠ��
; float[9] - ���設� 1,2,3
; uint16 - ��ਡ���

 ; Language support for locales: ru_RU (CP866), en_US.

if lang eq ru_RU
txt_err_stl_open:
	db '"STL',13,10
	db '���� *.stl �祭� ����让." -tW',0
txt_err_stl_null_v:
	db '"STL',13,10
	db '���設� �� �������." -tE',0
else ; Default to en_US
txt_err_stl_open:
	db '"STL',13,10
	db 'File *.stl is very large." -tW',0
txt_err_stl_null_v:
	db '"STL',13,10
	db 'Vertexes not found." -tE',0
end if

;output:
; eax - 㪠��⥫� �� ��ନ஢��� 䠩� 3ds (� ��砥 ��㤠� 0)
; ecx - ࠧ��� ��ନ஢����� 䠩�� 3ds
align 4
proc convert_stl_3ds uses ebx edx edi esi, f_data:dword, f_size:dword
locals
	c_mem dd ? ;������ ��� �८�ࠧ������ (convert memory)
	c_size dd ? ;ࠧ��� ����� ��� �८�ࠧ������ (convert memory size)
	vert_c dd ? ;�᫮ ���設 (vertex count)
	face_c dd ? ;�᫮ �࠭�� (faces count)
endl
	xor eax,eax
	mov esi,[f_data]
	cmp dword[esi],'soli'
	jne .bin_stl
	cmp word[esi+4],'d '
	jne .bin_stl

	jmp @f
	.bin_stl:
		;�஢��塞 䠩� �� ������ �ଠ�
		mov ecx,[esi+80]
		imul ecx,50
		add ecx,84 ;���������
		cmp ecx,[f_size]
		jne .no_stl
		call convert_binary_stl
		jmp .no_stl
	@@:

	;� ��砫� ������� 'solid '
	stdcall txt_next_line, 80

	mov eax,[f_data]
	sub eax,esi
	add eax,[f_size]
	stdcall get_stl_vertex_count, esi,eax
	or eax,eax
	jnz @f
		notify_window_run txt_err_stl_null_v
		jmp .no_stl
	@@:
	cmp eax,0xffff
	jle @f
		notify_window_run txt_err_stl_open
		mov eax,0xffff
	@@:

	mov [vert_c],eax
	mov ecx,3
	xor edx,edx
	div ecx
	mov [face_c],eax
	shl eax,3
	mov ecx,[vert_c]
	imul ecx,12
	lea ecx,[ecx+eax+40]
	lea edx,[esi-6]
	sub edx,[f_data]
	cmp edx,2
	jge @f
		mov edx,2 ;��������� ࠧ��� ��� ����� ��ꥪ�
	@@:
	add ecx,edx ;for object name
	mov [c_size],ecx
	stdcall mem.Alloc,ecx
	mov [c_mem],eax
	mov ebx,eax

	mov word[ebx],CHUNK_MAIN
	mov dword[ebx+2],ecx
	add ebx,6

	;3d3d
	mov word[ebx],CHUNK_OBJMESH
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	;4000
	mov word[ebx],CHUNK_OBJBLOCK
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	push ecx esi
		mov ecx,edx
		mov edi,ebx
		mov esi,[f_data]
		add esi,6 ;�ய�᪠�� 'solid '
		rep movsb ;�����㥬 ��� ��ꥪ�
		mov byte[edi-1],0
		add ebx,edx
	pop esi ecx

	;4100
	mov word[ebx],CHUNK_TRIMESH
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	;4110
	mov word[ebx],CHUNK_VERTLIST
	mov dword[ebx+2],8 ;+ �᫮ ���設 * 12
	add ebx,6

	mov edx,ebx
	mov word[edx],0 ;���. ���設
	add ebx,2
	finit
	.cycle0:
		call txt_ignore_space
		cmp dword[esi],'face'
		jne .end_v
		stdcall txt_next_line, 80

		call txt_ignore_space
		cmp dword[esi],'oute'
		jne .end_v
		stdcall txt_next_line, 80

		mov ecx,3 ;3 �窨 �� 1 �࠭�
		@@:
			stdcall stl_vertex_init, ebx
			or eax,eax
			jz .end_v
			add ebx,12
			inc word[edx]
			loop @b

		mov eax,[vert_c]
		cmp word[edx],ax
		jge .end_v
		call txt_ignore_space
		cmp dword[esi],'endl'
		jne .end_v
		stdcall txt_next_line, 80

		call txt_ignore_space
		cmp dword[esi],'endf'
		jne .end_v
		stdcall txt_next_line, 80
		jmp .cycle0
	.end_v:

	movzx eax,word[edx]
	imul eax,12
	add [edx-4],eax ;��ࠢ�塞 ࠧ��� ����� 4110

	;4120
	mov word[ebx],CHUNK_FACELIST
	mov ecx,[face_c]
	mov edx,ecx
	shl edx,3 ;���. �࠭�� * 8
	add edx,8
	mov [ebx+2],edx
	add ebx,6
	mov [ebx],cx ;���. �࠭��
	add ebx,2
	xor eax,eax
	@@:
		mov [ebx],ax
		inc eax
		mov [ebx+2],ax
		inc eax
		mov [ebx+4],ax
		inc eax
		mov word[ebx+6],0 ;��ਡ���
		add ebx,8
		loop @b

	mov eax,[c_mem]
	mov ecx,[c_size]
	.no_stl:
	ret
endp

;input:
; esi - 㪠��⥫� �� ��砫� 䠩��
; ecx - ࠧ��� 䠩��
align 4
proc convert_binary_stl
locals
	c_mem dd ? ;������ ��� �८�ࠧ������ (convert memory)
	c_size dd ? ;ࠧ��� ����� ��� �८�ࠧ������ (convert memory size)
	vert_c dd ? ;�᫮ ���設 (vertex count)
	face_c dd ? ;�᫮ �࠭�� (faces count)
endl
	mov eax,[esi+80]
	or eax,eax
	jnz @f
		notify_window_run txt_err_stl_null_v
		jmp .no_stl
	@@:
	cmp eax,0xffff/3
	jle @f
		notify_window_run txt_err_stl_open
		mov eax,0xffff/3
	@@:

	mov [face_c],eax
	mov ecx,eax
	imul eax,3
	mov [vert_c],eax
	shl ecx,3
	imul eax,12
	lea ecx,[ecx+eax+40+4] ;+4 ࠧ��� ��� ����� ��ꥪ�
	mov [c_size],ecx
	stdcall mem.Alloc,ecx
	mov [c_mem],eax
	mov ebx,eax

	mov word[ebx],CHUNK_MAIN
	mov dword[ebx+2],ecx
	add ebx,6

	;3d3d
	mov word[ebx],CHUNK_OBJMESH
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	;4000
	mov word[ebx],CHUNK_OBJBLOCK
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6
	mov dword[ebx],'Stl' ;��� ��ꥪ�
	add ebx,4

	;4100
	mov word[ebx],CHUNK_TRIMESH
	sub ecx,6
	mov dword[ebx+2],ecx
	add ebx,6

	;4110
	mov word[ebx],CHUNK_VERTLIST
	mov dword[ebx+2],8 ;+ �᫮ ���設 * 12
	add ebx,6

	mov edx,ebx
	mov ecx,[vert_c]
	mov word[edx],cx ;���. ���設
	add ebx,2

	add esi,80+4+12 ;�ய�� ���������, �᫠ �࠭��, 1-�� ����� ��ଠ���
	mov edi,ebx
	mov eax,[face_c]
	@@: ;横� �� �࠭�
		mov ecx,9
		rep movsd ;�����㥬 ���न���� 3-� ���設
		add esi,14 ;�ய�� ����� ��ଠ��� � ��ਡ�⮢
		dec eax
		jnz @b
	mov ebx,edi

	movzx eax,word[edx]
	imul eax,12
	add [edx-4],eax ;��ࠢ�塞 ࠧ��� ����� 4110

	;4120
	mov word[ebx],CHUNK_FACELIST
	mov ecx,[face_c]
	mov edx,ecx
	shl edx,3 ;���. �࠭�� * 8
	add edx,8
	mov [ebx+2],edx
	add ebx,6
	mov [ebx],cx ;���. �࠭��
	add ebx,2
	xor eax,eax
	@@:
		mov [ebx],ax
		inc eax
		mov [ebx+2],ax
		inc eax
		mov [ebx+4],ax
		inc eax
		mov word[ebx+6],0 ;��ਡ���
		add ebx,8
		loop @b

	mov eax,[c_mem]
	mov ecx,[c_size]
	.no_stl:
	ret
endp

;output:
; eax - vertex count
align 4
proc get_stl_vertex_count uses ebx ecx edi, f_data:dword, f_size:dword
	mov al,'v'
	xor ebx,ebx
	mov ecx,[f_size]
	mov edi,[f_data]
	.cycle0:
	repne scasb
	cmp dword[edi],'erte'
	jne @f
		inc ebx
		add edi,4
		sub ecx,4
		cmp ecx,4
		jg .cycle0
	@@:
	mov eax,ebx
	ret
endp

;input:
; esi - input description text
; ppoi - pointer to vertex struct
;output:
; eax - 0 (if error init) or 1
; esi - output description text
align 4
proc stl_vertex_init uses ebx ecx edi, ppoi:dword
	call txt_ignore_space
	cmp dword[esi],'vert'
	jne .err_init
	cmp word[esi+4],'ex'
	jne .err_init
		add esi,6
		mov ebx,[ppoi]

		call txt_copy_data
		call String_to_DoubleFloat
		fld qword[Data_Double]
		fstp dword[ebx] ;coord X

		call txt_copy_data
		call String_to_DoubleFloat
		fld qword[Data_Double]
		fstp dword[ebx+4] ;coord X

		call txt_copy_data
		call String_to_DoubleFloat
		fld qword[Data_Double]
		fstp dword[ebx+8] ;coord Z

		stdcall txt_next_line, 80
		jmp @f
	.err_init:
		xor eax,eax
		jmp .end_f
	@@:
		xor eax,eax
		inc eax
	.end_f:
	ret
endp

;input:
; esi - 㪠��⥫� �� ��砫� ��ப� � �஡�����
;output:
; al - destroy
; ecx - destroy
; esi - 㪠��⥫� �� ���� ���஡���� ᨬ���
align 4
txt_ignore_space:
	mov	ecx,64 ;���� �� ��横�������
	@@:
		lodsb
		cmp	al,' '
		jne	@f
		loop @b
	@@:
	dec esi
	ret

;input:
; esi - 㪠��⥫� �� ��ப� � �᫮� (�஡��� � ��砫� ��ப� ������������)
;output:
; al - destroy
; ecx - destroy
; edi - destroy
; esi - 㪠��⥫� �� ����� ����஢������ �᫠
; Data_String - ��ப� � �᫮� �� esi
align 4
txt_copy_data:
	call txt_ignore_space
	mov ecx,32
	mov edi,esi
	@@:
		lodsb
		or al,al
		jz @f
		cmp al,' '
		je @f
		cmp al,13
		je @f
		loop @b
	@@:
	mov esi,edi
	sub ecx,32
	neg ecx
	mov edi,Data_String
	rep movsb
	mov byte[edi],0
	ret

;input:
; esi - text pointer
align 4
proc txt_next_line uses eax ecx edi, mlen:dword
	mov al,13
	mov ecx,[mlen]
	mov edi,esi
	repne scasb
	cmp byte[edi],10
	jne @f
		inc edi
	@@:
	mov esi,edi
	ret
endp
