	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	mov    -0x1430(%rbp),%rcx
	mov    -0x1448(%rbp),%rsi
	mov    %rax,-0x1450(%rbp)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	nop
	mov    (%r15),%rdx
	add    $0x8,%r15
	lea    (%rdx,%rdx,4),%rax
	shl    $0x5,%rax
	lea    (%rsi,%rax,1),%rdi
	add    %rcx,%rax
	vmovsd 0x8(%rax),%xmm1
	vmovsd (%rax),%xmm0
	vsubsd 0x8(%rdi),%xmm1,%xmm1
	vsubsd (%rdi),%xmm0,%xmm0
	vmulsd %xmm1,%xmm1,%xmm1
	vfmadd132sd %xmm0,%xmm1,%xmm0
	vmovsd %xmm0,0x0(%r13,%rdx,8)
	cmp    %r15,%r14
	jne    <_Z17BM_MemoryBoundAoSI10AoSBaseVecIdLm10000000E3S20XadL_Z11GenerateS20IdES1_IT_EiEEEEvRN9benchmark5StateE+0xd00>
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
