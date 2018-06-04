section .text
global switch_stack

switch_stack:
  push  r12
  push  r13
  push  r14
  push  r15
  push  rbx
  push  rbp

  mov   rax, rsp
  mov   rsp, rdx

  pop   rbp
  pop   rbx
  pop   r15
  pop   r14
  pop   r13
  pop   r12

  pop   r9

  mov   rdx, rax
  jmp   r9
