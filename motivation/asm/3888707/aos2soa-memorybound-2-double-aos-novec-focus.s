	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	xor    %edx,%edx
	mov    %rax,%r14
	cmp    %r15,0x8(%rsp)
	je     <_Z17BM_MemoryBoundAoSI10AoSBaseVecIdLm10000000E2S2XadL_Z10GenerateS2IdES1_IT_EiEEEEvRN9benchmark5StateE+0x634>
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	nopl   (%rax)
	movsd  (%rbx,%rdx,2),%xmm0
	movsd  0x8(%rbx,%rdx,2),%xmm1
	subsd  (%r15,%rdx,2),%xmm0
	subsd  0x8(%r15,%rdx,2),%xmm1
	mulsd  %xmm0,%xmm0
	mulsd  %xmm1,%xmm1
	addsd  %xmm1,%xmm0
	movsd  %xmm0,0x0(%r13,%rdx,1)
	add    $0x8,%rdx
	cmp    %rbp,%rdx
	jne    <_Z17BM_MemoryBoundAoSI10AoSBaseVecIdLm10000000E2S2XadL_Z10GenerateS2IdES1_IT_EiEEEEvRN9benchmark5StateE+0x600>
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>