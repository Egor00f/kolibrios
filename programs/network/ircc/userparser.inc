;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


user_parser:

        mov     ebp, [window_active]                    ; print to the current window
        mov     [window_print], ebp

        mov     ecx, [edit1.size]
        test    ecx, ecx
        jz      .ret                                    ; ignore empty commands

        mov     esi, input_text
        mov     edi, user_command
        call    recode                                  ; Convert to UTF-8
        mov     byte[edi], 0x0                          ; Terminate string with \0
        sub     edi, user_command
        mov     [user_command.size], edi

        cmp     byte[user_command], '/'                 ; is it a server command ?
        je      .command

        cmp     [status], STATUS_LOGGED_IN
        jne     .not_loggedin

        cmp     [ebp + window.type], WINDOWTYPE_CHANNEL
        je      .send_privmsg
        cmp     [ebp + window.type], WINDOWTYPE_CHAT
        jne     .not_channel

  .send_privmsg:
        if TIMESTAMP
        call    print_timestamp
        end if

        mov     al, '<'
        call    print_char

        mov     esi, user_nick
        call    print_asciiz

        mov     al, '>'
        call    print_char
        mov     al, ' '
        call    print_char

        mov     eax, [user_command.size]
        mov     byte[user_command + eax],0
        mov     esi, user_command
        call    print_asciiz

        mov     al, 10
        call    print_char

; and now send it to the server
        mov     dword[packetbuf], 'PRIV'
        mov     dword[packetbuf+4], 'MSG '

        lea     esi, [ebp + window.name]
        mov     edi, packetbuf+8
        mov     ecx, MAX_WINDOWNAME_LEN
  .loop:
        lodsb
        test    al, al
        jz      .done
        stosb
        dec     ecx
        jnz     .loop
  .done:

        mov     ax, ' :'
        stosw

        mov     esi, user_command
        mov     ecx, [user_command.size]
;        inc     ecx
        rep     movsb

; end the command with a CRLF
        mov     ax, 0x0a0d
        stosw

        lea     esi, [edi - packetbuf]
        mcall   send, [socketnum], packetbuf, , 0

  .ret:
        ret

; Text begins with a '/' let's try to find the command in the lookup table.
  .command:
        mov     eax, dword[user_command+1]      ; skip '/'
        or      eax, 0x20202020                 ; convert to lowercase

        mov     edi, user_commands
        mov     ecx, user_commands.number
        cmp     [status], STATUS_CONNECTED
        jae     .cmd_loop
        mov     ecx, user_commands.number2
  .cmd_loop:
        scasd
        je      .got_cmd
        add     edi, 4
        dec     ecx
        jnz     .cmd_loop

        cmp     [status], STATUS_CONNECTED
        jb      .not_connected

; Commands shorter then 3 chars are placed here
        and     eax, 0x00ffffff
        cmp     eax, 'me '
        je      cmd_usr_me

; If none of the listed commands, send text straight to server
        jmp     cmd_usr_send

  .got_cmd:
        jmp     dword[edi]

  .not_loggedin:
        mov     esi, str_notloggedin
        call    print_asciiz
        ret

  .not_connected:
        mov     esi, str_notconnected
        call    print_asciiz
        ret

  .not_channel:
        mov     esi, str_notchannel
        call    print_asciiz
        ret


; user commands lookup table
user_commands:
        dd      'nick', cmd_usr_nick
        dd      'real', cmd_usr_real
        dd      'serv', cmd_usr_server
        dd      'help', cmd_usr_help
        dd      'code', cmd_usr_code

        .number2 = ($ - user_commands) / 8

; All following commands require a connection to the server.
        dd      'quer', cmd_usr_quer
        dd      'quit', cmd_usr_quit
        dd      'part', cmd_usr_part
        dd      'ctcp', cmd_usr_ctcp
        dd      'msg ', cmd_usr_msg

        .number = ($ - user_commands) / 8



cmd_usr_msg:

        lea     esi, [user_command+5]

        mov     dword[packetbuf], 'PRIV'
        mov     dword[packetbuf+4], 'MSG '
        lea     edi, [packetbuf+8]

  @@:
        lodsb
        test    al, al
        jz      .fail
        cmp     al, 10
        je      .fail
        cmp     al, 13
        je      .fail
        stosb
        cmp     al, ' '
        jne     @r

        mov     al, ':'
        stosb

        push    edi
  @@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, 10
        je      @f
        cmp     al, 13
        je      @f
        stosb
        jmp     @r
  @@:
; end the command with a CRLF
        mov     ax, 0x0a0d
        stosw
        mov     byte[edi], 0

; now print to the window
        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, msg_header
        call    print_asciiz

        mov     esi, packetbuf+8
        mov     bl, ' '
        call    print_string

        mov     al, '*'
        call    print_char

        mov     al, ' '
        call    print_char

        pop     esi
        call    print_asciiz

        lea     esi, [edi - packetbuf]
        mcall   send, [socketnum], packetbuf, , 0

  .fail:
        ret



cmd_usr_quit:

        mov     esi, quit_msg
        cmp     byte[user_command+5], ' '
        jne     quit_server
        lea     esi, [user_command+6]

; esi = quit message
quit_server:

; User wants to close a channel, send PART command to server
        mov     dword[packetbuf], 'QUIT'
        mov     word[packetbuf+4], ' :'
        lea     edi, [packetbuf+6]
; Append our quit msg
  @@:
        lodsb
        cmp     al, 13
        je      @f
        test    al, al
        jz      @f
        stosb
        jmp     @r
  @@:
; end the command with a CRLF
        mov     ax, 0x0a0d
        stosw

        lea     esi, [edi - packetbuf]                  ; calculate length
        mcall   send, [socketnum], packetbuf, , 0       ; and finally send to server

        mov     ebp, windows
  .window_loop:
        cmp     [ebp + window.type], WINDOWTYPE_NONE
        je      .next_window
        mov     [window_print], ebp
        if TIMESTAMP
        call    print_timestamp
        end if
        mov     esi, str_disconnected
        call    print_asciiz
        cmp     [ebp + window.type], WINDOWTYPE_CHANNEL
        jne     .next_window
        call    user_remove_all
  .next_window:
        add     ebp, sizeof.window
        cmp     ebp, windows + (MAX_WINDOWS * sizeof.window)
        jb      .window_loop

        mov     [status], STATUS_DISCONNECTED
        mcall   close, [socketnum]

        ret




cmd_usr_nick:

        cmp     [user_command.size], 5
        je      .justprint
        cmp     byte[user_command+5], ' '
        jne     .fail
        cmp     [socketnum], 0
        je      .dontsend

; Request nickname change to server
        mov     dword[user_command+1], 'NICK'
        mov     esi, [user_command.size]
        mov     word[user_command + esi], 0x0a0d
        inc     esi
        mcall   send, [socketnum], user_command+1, , 0

  .fail:
        ret

; We aren't logged in yet, directly change user_nick field and print notification to user.
  .dontsend:
        mov     ecx, MAX_NICK_LEN
        mov     esi, user_command+6
        mov     edi, user_nick
  @@:
        lodsb
        cmp     al, 13
        je      @f
        stosb
        dec     ecx
        jnz     @r
  @@:
        xor     al, al
        stosb

  .justprint:
        mov     esi, str_nickchange
        call    print_asciiz

        mov     esi, user_nick
        call    print_asciiz

        mov     al, 10
        call    print_char

        ret



cmd_usr_real:

        cmp     byte[user_command+5], ' '
        jne     cmd_usr_send

        mov     ecx, MAX_REAL_LEN
        mov     esi, user_command+6
        mov     edi, user_real_name
  .loop:
        lodsb
        cmp     al, 13
        je      .done
        stosb
        dec     ecx
        jnz     .loop
  .done:
        xor     al, al
        stosb

        mov     esi, str_realchange
        call    print_asciiz

        mov     esi, user_real_name
        call    print_asciiz

        mov     al, 10
        call    print_char

        ret



cmd_usr_server:

        mov     eax, dword[user_command+5]      ; check for 'er ', we only checked 'serv'
        or      eax, 0x00002020
        and     eax, 0x00ffffff
        cmp     eax, 'er '
        jne     cmd_usr_send

; Server window is always first window in the list, switch to it.
        mov     [window_print], windows
        mov     [window_active], windows

        mov     ecx, [user_command.size]        ; ok now set the address
        sub     ecx, 8

        mov     esi, user_command+8
  .now:
        push    esi
        mov     edi, irc_server_name
  .loop:                                        ; copy until zero byte, or ecx reaches zero.
        lodsb
        stosb
        test    al, al
        jz      .done
        dec     ecx
        jnz     .loop
        xor     al, al
        stosb
  .done:
        pop     esi

; set it also in window name
        mov     ebx, [window_print]
        call    window_set_name

; now connect
        call    socket_connect

        ret


cmd_usr_quer:

        mov     esi, user_command+7
        call    window_open
;        test    ebx, ebx
;        jz      .fail

        ret



cmd_usr_help:

        mov     esi, str_help
        call    print_asciiz

        ret



cmd_usr_code:

        ; TODO

        ret



; User typed a part command
cmd_usr_part:

        cmp     byte[user_command+5], 13        ; parameters given?
        jne     cmd_usr_send                    ; yes, send command straight to server

; close active window
cmd_usr_close_window:

        mov     esi, [window_active]
        mov     [window_print], esi
        cmp     [esi + window.type], WINDOWTYPE_SERVER
        je      .not_channel

        lea     esi, [esi + window.name]
        call    cmd_usr_part_channel
        call    window_close
        ret

  .not_channel:
        cmp     [esi + window.type], WINDOWTYPE_CHAT
        jne     .not_chat

        call    window_close
  .not_chat:

        ret



; Send part command to server
; esi must point to channel name (ASCIIZ)
cmd_usr_part_channel:

; User wants to close a channel, send PART command to server
        mov     dword[packetbuf], 'PART'
        mov     byte[packetbuf+4], ' '
        lea     edi, [packetbuf+5]
  @@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, 13
        je      @f
        cmp     al, 10
        je      @f
        stosb
        jmp     @r
  @@:
; end the command with a CRLF
        mov     ax, 0x0a0d
        stosw

        lea     esi, [edi - packetbuf]                  ; calculate length
        mcall   send, [socketnum], packetbuf, , 0       ; and finally send to server

        ret



cmd_usr_ctcp:

        cmp     byte[user_command+5], ' '
        jne     cmd_usr_send

        mov     esi, user_command+6
; prepare a 'PRIVMSG '
        mov     dword[packetbuf], 'PRIV'
        mov     dword[packetbuf+4], 'MSG '
        lea     edi, [packetbuf+8]

; append the destination (nickname/channel)
  @@:
        lodsb
        test    al, al
        jz      .fail
        cmp     al, ' '
        je      @f
        stosb
        jmp     @r
  @@:

        mov     ax, ' :'
        stosw
        mov     al, 0x01
        stosb

        push    esi

; copy the message itself
  @@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, 13
        je      @f
        cmp     al, 10
        je      @f
        stosb
        jmp     @r
  @@:

; end of message
        mov     al, 0x01
        stosb
        mov     ax, 0x0a0d
        stosw

; now print to the window
        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, ctcp_header
        call    print_asciiz

        mov     esi, user_command+6
        mov     bl, ' '
        call    print_string

        mov     al, ']'
        call    print_char

        mov     al, ' '
        call    print_char

        pop     esi
        call    print_asciiz

        mov     al, 10
        call    print_char

; now send it away
        lea     esi, [edi - packetbuf]                  ; calculate length
        mcall   send, [socketnum], packetbuf, , 0       ; and finally send to server
  .fail:

        ret



cmd_usr_me:

; prepare a 'PRIVMSG '
        mov     dword[packetbuf], 'PRIV'
        mov     dword[packetbuf+4], 'MSG '
        lea     edi, [packetbuf+8]

; append the destination (nickname/channel)
        mov     esi, [window_active]
        lea     esi, [esi + window.name]
  @@:
        lodsb
        test    al, al
        je      @f
        stosb
        jmp     @r
  @@:

; Make the CTCP action header
        mov     eax, ' :' + 0x01 shl 16 + 'A' shl 24
        stosd
        mov     eax, 'CTIO'
        stosd
        mov     al, 'N'
        stosb

; copy the message itself (including first space)
        mov     esi, user_command+3
  @@:
        lodsb
        cmp     al, 13
        je      @f
        stosb
        jmp     @r
  @@:

; end of CTCP message
        mov     al, 0x01
        stosb
        mov     ax, 0x0a0d
        stosw

; now send it to the server
        lea     esi, [edi - packetbuf]                  ; calculate length
        mcall   send, [socketnum], packetbuf, , 0       ; and finally send to server

; print to local window
        if TIMESTAMP
        call    print_timestamp
        end if

        mov     esi, action_header
        call    print_asciiz

        mov     esi, user_nick
        call    print_asciiz

        mov     esi, user_command+3
        mov     bl, 13
        call    print_string

        mov     al, 10
        call    print_char

        ret



; The user typed some undefined command, just send it to the server
cmd_usr_send:

        mov     esi, [user_command.size]
        mov     eax, [user_command.size]
        add     eax, user_command
        mov     word[eax], 0x0a0d
        inc     esi                     ; Skip / add \r\n
        mcall   send, [socketnum], user_command+1, , 0

        ret





