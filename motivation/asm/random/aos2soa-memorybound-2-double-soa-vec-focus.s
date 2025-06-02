	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	mov    -0x1450(%rbp),%r9
	mov    %rax,-0x1438(%rbp)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	nop
	mov    (%r15),%rdx
	add    $0x8,%r15
	mov    %rdx,%rax
	shl    $0x4,%rax
	lea    (%r14,%rax,1),%rcx
	add    %r13,%rax
	vmovsd 0x8(%rax),%xmm1
	vmovsd (%rax),%xmm0
	vsubsd 0x8(%rcx),%xmm1,%xmm1
	vsubsd (%rcx),%xmm0,%xmm0
	vmulsd %xmm1,%xmm1,%xmm1
	vfmadd132sd %xmm0,%xmm1,%xmm0
	vmovsd %xmm0,(%rbx,%rdx,8)
	cmp    %r9,%r15
	jne    <_Z17BM_MemoryBoundAoSI10AoSBaseVecIdLm10000000E2S2XadL_Z10GenerateS2IdES1_IT_EiEEEEvRN9benchmark5StateE+0x7c0>
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
