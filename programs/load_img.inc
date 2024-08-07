;
; ������ ��� ������஢���� ����ࠦ���� �१ �㭪樨 ������⥪� libimg.
;
; (1) ����� load_image_file:
;
; �஢���� ࠧ��� ����祭���� 䠩�� � ����ࠦ�����.
; �뤥��� ��� ���� ������ � ����㦠�� �㤠 䠩�.
; ��।���� ࠧ��� ����� ����室���� ��� �ᯠ����� ����ࠦ����.
; ��ॢ뤥��� ������, � �ᯠ���뢠�� ����ࠦ���� �� �ଠ⮢ jpg, png, ...
;    � �ଠ� rgb ����� ����� �뢮���� �� ��࠭ �१ ���. �㭪��.
; �� ����室����� ����� ⠪�� ������� ࠧ���� ���뢠����� ����ࠦ���� ���
;    �� ���쭥�襣� �ᯮ�짮����� � �ணࠬ��.
;
; � �ᯮ����饩 �ணࠬ�� ������ ���� ������祭� ������⥪� libimg,
;    ������ ��६����� file_name ࠧ��஬ ����� 4096 ����,
;    � ⠪�� ������� run_file_70 ⨯� FileInfoBlock.
;
; (2) ����� include_image_file:
;
; ������� ����ࠦ���� � 䠩� �ணࠬ��.
; ��।���� ࠧ��� ����� ����室���� ��� �ᯠ����� ����ࠦ����.
; �뤥��� ������, � �ᯠ���뢠�� ����ࠦ���� �� �ଠ⮢ jpg, png, ...
;    � �ଠ� rgb ����� ����� �뢮���� �� ��࠭ �१ ���. �㭪��.
; �� ����室����� ����� ⠪�� ������� ࠧ���� ����७���� ����ࠦ���� ���
;    �� ���쭥�襣� �ᯮ�짮����� � �ணࠬ��.

; ������祭�� �������� ����室���� 䠩���:
include 'dll.inc'
;include 'load_lib.mac'
include 'develop/libraries/libs-dev/libio/libio.inc'

align 4
open_b rb 560

; Language support for locales: ru_RU (CP866), en_US.
txt_err_img_file:
if lang eq ru_RU
	db '�訡�� N'
.n: db '* �� ����⨨ 䠩�� '
else ; Default to en_US
	db 'Error N'
.n: db '*. Can',39,'t open file '
end if
.f: dd 0,0

; path - ����� ���� ��६����� ��� ��ப��� ��ࠬ��஬
; buf - ��६����� �㤠 �㤥� ����ᠭ 㪠��⥫� �� ����ࠦ���� � �ଠ� rgb,
;    � ��砥 �᫨ 䠩� �� ��஥��� � �㤥� ����ᠭ 0
; img_w, img_h - ��६���� �㤠 ���� ����ᠭ� ࠧ���� ���뢠�����
;    ����ࠦ����, �� ��易⥫�� ��ࠬ����
; bytes_p_p - ᪮�쪮 ���� ����� ���� �� ���ᥫ�, �� ��易⥫�� ��ࠬ���
;    �᫨ �� 㪠��� ������ 3. �������쭮� ���祭�� ������ ���� �� ����� 3.
macro load_image_file path, buf, img_w, img_h, bytes_p_p
{
local .err_open
local .end_open

if path eqtype '' ;�஢��塞 ����� �� ��ப�� ��ࠬ��� path
	local .path_str
	jmp @f
		.path_str db path ;�ନ�㥬 �������� ��६�����
		db 0
	@@:
	;32 - �⠭����� ���� �� ���஬� ������ ���� ���� � ��⥬�� ��⥬
	copy_path .path_str,[32],file_name,0
else
	copy_path path,[32],file_name,0 ;�ନ�㥬 ����� ���� � 䠩�� ����ࠦ����, ���ࠧ㬥���� �� �� � ����� ����� � �ணࠬ���
end if
	mov dword[buf],0

	mov [run_file_70.Function], SSF_GET_INFO
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], 0
	mov dword[run_file_70.Buffer], open_b
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], file_name
	mcall SF_FILE,run_file_70
	or eax,eax
	jnz .err_open

		mov ecx,dword[open_b+32] ;+32 qword: ࠧ��� 䠩�� � �����
		stdcall mem.Alloc,ecx ;�뤥�塞 ������ ��� ����ࠦ����
		mov [buf],eax
		mov [run_file_70.Function], SSF_READ_FILE
		mov [run_file_70.Position], 0
		mov [run_file_70.Flags], 0
		mov [run_file_70.Count], ecx
		mov [run_file_70.Buffer], eax
		mov byte[run_file_70+20], 0
		mov [run_file_70.FileName], file_name
		mcall SF_FILE,run_file_70 ;����㦠�� 䠩� ����ࠦ����
		test eax,eax
		jnz .err_open
		cmp ebx,0xffffffff
		je .end_open
			;��।��塞 ��� ����ࠦ���� � ��襬 ��� ��ࠬ����
			stdcall [img_decode], [buf],ebx,0
			mov ebx,eax
			;��।��塞 ࠧ��� ������஢������ ����ࠦ����
			mov ecx,[eax+4] ;+4 = image width
if img_w eq
else
			mov dword[img_w],ecx
end if
if img_h eq
			imul ecx,[eax+8] ;+8 = image height
else
			mov eax,[eax+8] ;+8 = image height
			mov dword[img_h],eax
			imul ecx,eax
end if
if bytes_p_p eq
			imul ecx,3 ;need for r,g,b
else
			imul ecx,bytes_p_p
end if
			stdcall mem.ReAlloc,[buf],ecx ;�����塞 ࠧ��� ��� ����
			mov [buf],eax
			stdcall [img_to_rgb2], ebx,[buf] ;�८�ࠧ㥬 ����ࠦ���� � �ଠ�� rgb
			stdcall [img_destroy], ebx ;㤠�塞 �६���� ���� � ��ࠬ��ࠬ� ����ࠦ����
			jmp .end_open
	.err_open:
		add al,'0'
		mov byte[txt_err_img_file.n],al
if path eqtype ''
		mov eax,dword[.path_str]
		mov dword[txt_err_img_file.f],eax
		mov eax,dword[.path_str+4]
else
		mov eax,dword[path]
		mov dword[txt_err_img_file.f],eax
		mov eax,dword[path+4]
end if
		mov dword[txt_err_img_file.f+4],eax
		mov byte[txt_err_img_file.f+7],0
		notify_window_run txt_err_img_file
	.end_open:
}



; path - ��� ����砥���� 䠩��
; buf - ��६����� �㤠 �㤥� ����ᠭ 㪠��⥫� �� ����ࠦ���� � �ଠ� rgb
; img_w, img_h - ��६���� �㤠 ���� ����ᠭ� ࠧ���� ����ࠦ����,
;    �� ��易⥫�� ��ࠬ����
; bytes_p_p - ᪮�쪮 ���� ����� ���� �� ���ᥫ�, �� ��易⥫�� ��ࠬ���
;    �᫨ �� 㪠��� ������ 3. �������쭮� ���祭�� ������ ���� �� ����� 3.
macro include_image_file path, buf, img_w, img_h, bytes_p_p
{
local .beg_file
local .end_file

jmp .end_file
align 4
.beg_file:
	file path
.end_file:

	;��।��塞 ��� ����ࠦ���� � ��襬 ��� ��ࠬ����
	stdcall [img_decode], .beg_file,.end_file-.beg_file,0
	mov ebx,eax
	;��।��塞 ࠧ��� ������஢������ ����ࠦ����
	mov ecx,[eax+4] ;+4 = image width
if img_w eq
else
	mov dword[img_w],ecx
end if
if img_h eq
	imul ecx,[eax+8] ;+8 = image height
else
	mov eax,[eax+8] ;+8 = image height
	mov dword[img_h],eax
	imul ecx,eax
end if
if bytes_p_p eq
	imul ecx,3 ;need for r,g,b
else
	imul ecx,bytes_p_p
end if
	stdcall mem.Alloc,ecx ;�����塞 ࠧ��� ��� ����
	mov [buf],eax
	stdcall [img_to_rgb2], ebx,[buf] ;�८�ࠧ㥬 ����ࠦ���� � �ଠ�� rgb
	stdcall [img_destroy], ebx ;㤠�塞 �६���� ���� � ��ࠬ��ࠬ� ����ࠦ����
}
