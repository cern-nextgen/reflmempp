	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	mov    -0x88(%rbp),%rsi
	mov    -0xa8(%rbp),%rcx
	mov    %r13,%rdx
	mov    %rax,%r12
	mov    %rsi,%rax
	cmp    %rsi,%r15
	je     <_Z18BM_ComputeBoundAoSI10AoSBaseVecIdLm10000000E3S20XadL_Z11GenerateS20IdES1_IT_EiEEEEvRN9benchmark5StateE+0xcfa>
	nopl   (%rax)
	vmovsd (%rax),%xmm5
	vmovsd (%rdx),%xmm3
	add    $0xa0,%rax
	add    $0xa0,%rdx
	vmovsd -0x98(%rax),%xmm6
	vmovsd -0x98(%rdx),%xmm4
	add    $0x8,%rcx
	vaddsd %xmm3,%xmm5,%xmm8
	vaddsd %xmm3,%xmm6,%xmm2
	vsubsd %xmm5,%xmm4,%xmm11
	vsubsd %xmm6,%xmm3,%xmm12
	vaddsd %xmm4,%xmm5,%xmm10
	vsubsd %xmm6,%xmm4,%xmm9
	vaddsd %xmm4,%xmm6,%xmm7
	vmulsd %xmm2,%xmm11,%xmm0
	vmulsd %xmm11,%xmm12,%xmm1
	vdivsd %xmm11,%xmm12,%xmm13
	vfmadd132sd 0x557f4(%rip),%xmm0,%xmm1        # <_ZSt19piecewise_construct+0xd>
	vaddsd %xmm1,%xmm0,%xmm0
	vsubsd %xmm5,%xmm3,%xmm1
	vfmadd231sd %xmm10,%xmm2,%xmm0
	vfmadd231sd %xmm10,%xmm2,%xmm0
	vdivsd %xmm12,%xmm11,%xmm12
	vfmadd231sd %xmm1,%xmm1,%xmm0
	vfmadd231sd %xmm9,%xmm9,%xmm0
	vfmadd231sd %xmm8,%xmm8,%xmm0
	vfmadd231sd %xmm7,%xmm7,%xmm0
	vdivsd %xmm9,%xmm9,%xmm9
	vfmadd231sd %xmm1,%xmm1,%xmm0
	vsubsd %xmm13,%xmm0,%xmm0
	vsubsd %xmm12,%xmm0,%xmm0
	vdivsd %xmm11,%xmm2,%xmm12
	vdivsd %xmm2,%xmm11,%xmm11
	vsubsd %xmm12,%xmm0,%xmm0
	vdivsd %xmm1,%xmm1,%xmm1
	vsubsd %xmm11,%xmm0,%xmm0
	vdivsd %xmm10,%xmm2,%xmm11
	vdivsd %xmm2,%xmm10,%xmm10
	vsubsd %xmm11,%xmm0,%xmm0
	vdivsd %xmm8,%xmm8,%xmm8
	vsubsd %xmm10,%xmm0,%xmm0
	vsubsd %xmm1,%xmm0,%xmm0
	vsubsd %xmm9,%xmm0,%xmm0
	vdivsd %xmm7,%xmm7,%xmm7
	vsubsd %xmm8,%xmm0,%xmm0
	vdivsd %xmm6,%xmm4,%xmm1
	vsubsd %xmm7,%xmm0,%xmm0
	vfnmadd231sd %xmm3,%xmm5,%xmm0
	vfnmadd231sd %xmm3,%xmm5,%xmm0
	vdivsd %xmm5,%xmm3,%xmm2
	vfmadd231sd %xmm3,%xmm6,%xmm0
	vfnmadd231sd %xmm4,%xmm5,%xmm0
	vfmadd231sd %xmm4,%xmm5,%xmm0
	vfnmadd231sd %xmm3,%xmm6,%xmm0
	vfmadd231sd %xmm3,%xmm6,%xmm0
	vfnmadd231sd %xmm4,%xmm5,%xmm0
	vfmadd231sd %xmm4,%xmm5,%xmm0
	vfnmadd231sd %xmm3,%xmm6,%xmm0
	vfmadd231sd %xmm3,%xmm6,%xmm0
	vfnmadd231sd %xmm4,%xmm5,%xmm0
	vfmadd231sd %xmm4,%xmm5,%xmm0
	vfnmadd231sd %xmm3,%xmm6,%xmm0
	vfmadd231sd %xmm3,%xmm5,%xmm0
	vfnmadd231sd %xmm3,%xmm5,%xmm0
	vfmadd231sd %xmm4,%xmm6,%xmm0
	vfnmadd231sd %xmm4,%xmm6,%xmm0
	vdivsd %xmm5,%xmm4,%xmm4
	vfmadd231sd %xmm3,%xmm5,%xmm0
	vfnmadd231sd %xmm3,%xmm5,%xmm0
	vaddsd %xmm1,%xmm0,%xmm0
	vaddsd %xmm1,%xmm0,%xmm0
	vdivsd %xmm6,%xmm3,%xmm3
	vsubsd %xmm2,%xmm0,%xmm0
	vaddsd %xmm2,%xmm0,%xmm0
	vsubsd %xmm3,%xmm0,%xmm0
	vaddsd %xmm4,%xmm0,%xmm0
	vsubsd %xmm4,%xmm0,%xmm0
	vaddsd %xmm3,%xmm0,%xmm0
	vsubsd %xmm3,%xmm0,%xmm0
	vaddsd %xmm4,%xmm0,%xmm0
	vsubsd %xmm4,%xmm0,%xmm0
	vaddsd %xmm3,%xmm0,%xmm0
	vsubsd %xmm3,%xmm0,%xmm0
	vaddsd %xmm4,%xmm0,%xmm0
	vsubsd %xmm4,%xmm0,%xmm0
	vaddsd %xmm3,%xmm0,%xmm0
	vsubsd %xmm2,%xmm0,%xmm0
	vaddsd %xmm2,%xmm0,%xmm0
	vsubsd %xmm1,%xmm0,%xmm0
	vaddsd %xmm1,%xmm0,%xmm0
	vsubsd %xmm2,%xmm0,%xmm0
	vaddsd %xmm2,%xmm0,%xmm0
	vsubsd %xmm1,%xmm0,%xmm0
	vaddsd %xmm1,%xmm0,%xmm0
	vmovsd %xmm0,-0x8(%rcx)
	cmp    %rax,%r15
	jne    <_Z18BM_ComputeBoundAoSI10AoSBaseVecIdLm10000000E3S20XadL_Z11GenerateS20IdES1_IT_EiEEEEvRN9benchmark5StateE+0xb38>
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
