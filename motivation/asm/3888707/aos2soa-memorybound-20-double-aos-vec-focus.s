	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	mov    -0xa0(%rbp),%rsi
	mov    -0x88(%rbp),%rdx
	mov    %rax,%r13
	mov    -0x78(%rbp),%rcx
	mov    %rsi,%rax
	cmp    %rsi,%r15
	je     <_Z17BM_MemoryBoundAoSI10AoSBaseVecIdLm10000000E3S20XadL_Z11GenerateS20IdES1_IT_EiEEEEvRN9benchmark5StateE+0xbbd>
	data16 cs nopw 0x0(%rax,%rax,1)
	data16 cs nopw 0x0(%rax,%rax,1)
	nopl   0x0(%rax,%rax,1)
	vmovsd 0x8(%rdx),%xmm1
	vmovsd (%rdx),%xmm0
	add    $0xa0,%rax
	add    $0xa0,%rdx
	add    $0x8,%rcx
	vsubsd -0x98(%rax),%xmm1,%xmm1
	vsubsd -0xa0(%rax),%xmm0,%xmm0
	vmulsd %xmm1,%xmm1,%xmm1
	vfmadd132sd %xmm0,%xmm1,%xmm0
	vmovsd %xmm0,-0x8(%rcx)
	cmp    %rax,%r15
	jne    <_Z17BM_MemoryBoundAoSI10AoSBaseVecIdLm10000000E3S20XadL_Z11GenerateS20IdES1_IT_EiEEEEvRN9benchmark5StateE+0xb80>
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
