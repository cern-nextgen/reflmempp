	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	xor    %edx,%edx
	cmpq   $0x0,-0x1c8(%rbp)
	mov    %rax,%rsi
	jle    <_Z17BM_MemoryBoundSoAI7SoA2VecIdLm10000000EEEvRN9benchmark5StateE+0x4b7>
	data16 cs nopw 0x0(%rax,%rax,1)
	nop
	movsd  (%r14,%rdx,1),%xmm0
	movsd  (%rbx,%rdx,1),%xmm1
	subsd  0x0(%r13,%rdx,1),%xmm0
	subsd  (%r12,%rdx,1),%xmm1
	mulsd  %xmm0,%xmm0
	mulsd  %xmm1,%xmm1
	addsd  %xmm1,%xmm0
	movsd  %xmm0,(%r15,%rdx,1)
	add    $0x8,%rdx
	cmp    %rdx,-0x1c8(%rbp)
	jne    <_Z17BM_MemoryBoundSoAI7SoA2VecIdLm10000000EEEvRN9benchmark5StateE+0x480>
	mov    %rsi,-0x1e0(%rbp)
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>