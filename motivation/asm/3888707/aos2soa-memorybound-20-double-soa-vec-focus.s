	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	cmpq   $0x0,-0x5d8(%rbp)
	mov    %rax,-0x5c8(%rbp)
	jle    <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0xa98>
	cmpq   $0x8,-0x5d8(%rbp)
	je     <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0xbe0>
	cmpb   $0x0,-0x5e8(%rbp)
	je     <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0xbe0>
	cmpq   $0x2,-0x5f8(%rbp)
	jbe    <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0xca2>
	mov    -0x608(%rbp),%rdx
	xor    %eax,%eax
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	nopl   0x0(%rax,%rax,1)
	vmovupd (%r14,%rax,1),%ymm1
	vmovupd 0x0(%r13,%rax,1),%ymm0
	vsubpd (%r12,%rax,1),%ymm1,%ymm1
	vsubpd (%rbx,%rax,1),%ymm0,%ymm0
	vmulpd %ymm1,%ymm1,%ymm1
	vfmadd132pd %ymm0,%ymm1,%ymm0
	vmovupd %ymm0,(%r15,%rax,1)
	add    $0x20,%rax
	cmp    %rax,%rdx
	jne    <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0x9c0>
	mov    -0x600(%rbp),%rsi
	mov    %rsi,%r11
	and    $0xfffffffffffffffc,%r11
	test   $0x3,%sil
	je     <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0xc58>
	lea    0x0(,%r11,8),%rax
	sub    %r11,%rsi
	lea    (%rbx,%rax,1),%rcx
	lea    0x0(%r13,%rax,1),%r10
	lea    (%r12,%rax,1),%rdx
	lea    (%r14,%rax,1),%r9
	lea    (%r15,%rax,1),%rdi
	cmp    $0x1,%rsi
	je     <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0xce3>
	vzeroupper
	vmovupd (%r14,%rax,1),%xmm1
	vmovupd 0x0(%r13,%rax,1),%xmm0
	vsubpd (%r12,%rax,1),%xmm1,%xmm1
	vsubpd (%rbx,%rax,1),%xmm0,%xmm0
	vmulpd %xmm1,%xmm1,%xmm1
	vfmadd132pd %xmm0,%xmm1,%xmm0
	vmovupd %xmm0,(%r15,%rax,1)
	test   $0x1,%sil
	je     <_Z17BM_MemoryBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0xa98>
	and    $0xfffffffffffffffe,%rsi
	shl    $0x3,%rsi
	add    %rsi,%rcx
	add    %rsi,%r10
	add    %rsi,%rdx
	add    %rsi,%r9
	add    %rsi,%rdi
	vmovsd (%r9),%xmm1
	vmovsd (%r10),%xmm0
	vsubsd (%rdx),%xmm1,%xmm1
	vsubsd (%rcx),%xmm0,%xmm0
	vmulsd %xmm1,%xmm1,%xmm1
	vfmadd132sd %xmm0,%xmm1,%xmm0
	vmovsd %xmm0,(%rdi)
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
