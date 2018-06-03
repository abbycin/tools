section .text
global switch_stack

switch_stack:
  push  r12
  push  r13
  push  r14
  push  r15
  push  rbx
  push  rbp

  mov rax, rsp
  mov rsp, rsi

  pop   rbp
  pop   rbx
  pop   r15
  pop   r14
  pop   r13
  pop   r12

  mov   r8, [rsp]
  add   rsp, 8

  mov   rsi, rax
  jmp   r8
