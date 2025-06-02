	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	mov    %rax,%rsi
	xor    %eax,%eax
	test   %rbx,%rbx
	jle    <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0x9ba>
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	nopl   0x0(%rax,%rax,1)
	mov    -0x5c8(%rbp),%rcx
	movsd  (%r14,%rax,1),%xmm0
	movsd  0x0(%r13,%rax,1),%xmm1
	subsd  (%r12,%rax,1),%xmm1
	subsd  (%rcx,%rax,1),%xmm0
	mulsd  %xmm1,%xmm1
	mulsd  %xmm0,%xmm0
	addsd  %xmm1,%xmm0
	movsd  %xmm0,(%r15,%rax,1)
	add    $0x8,%rax
	cmp    %rbx,%rax
	jne    <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0x980>
	mov    %rsi,-0x5e0(%rbp)
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
