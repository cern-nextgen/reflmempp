	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	mov    -0x2d08(%rbp),%r8
	mov    -0x2d10(%rbp),%rcx
	mov    %rax,-0x2cf8(%rbp)
	mov    -0x2d28(%rbp),%rdx
	cs nopw 0x0(%rax,%rax,1)
	mov    (%rcx),%rax
	add    $0x8,%rcx
	vmovsd (%r12,%rax,8),%xmm3
	vmovsd (%r15,%rax,8),%xmm5
	vmovsd 0x0(%r13,%rax,8),%xmm6
	vmovsd (%r14,%rax,8),%xmm4
	vaddsd %xmm5,%xmm3,%xmm8
	vaddsd %xmm6,%xmm3,%xmm2
	vsubsd %xmm5,%xmm4,%xmm11
	vsubsd %xmm6,%xmm3,%xmm12
	vaddsd %xmm5,%xmm4,%xmm10
	vsubsd %xmm6,%xmm4,%xmm9
	vaddsd %xmm4,%xmm6,%xmm7
	vmulsd %xmm2,%xmm11,%xmm0
	vmulsd %xmm11,%xmm12,%xmm1
	vdivsd %xmm11,%xmm12,%xmm13
	vfmadd132sd 0x55d85(%rip),%xmm0,%xmm1        # <_ZSt19piecewise_construct+0x97>
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
	vdivsd %xmm2,%xmm10,%xmm2
	vsubsd %xmm11,%xmm0,%xmm0
	vdivsd %xmm8,%xmm8,%xmm8
	vsubsd %xmm2,%xmm0,%xmm0
	vsubsd %xmm1,%xmm0,%xmm0
	vsubsd %xmm9,%xmm0,%xmm0
	vdivsd %xmm7,%xmm7,%xmm7
	vsubsd %xmm8,%xmm0,%xmm0
	vdivsd %xmm6,%xmm4,%xmm1
	vsubsd %xmm7,%xmm0,%xmm0
	vfnmadd231sd %xmm5,%xmm3,%xmm0
	vfnmadd231sd %xmm5,%xmm3,%xmm0
	vdivsd %xmm5,%xmm3,%xmm2
	vfmadd231sd %xmm6,%xmm3,%xmm0
	vfnmadd231sd %xmm5,%xmm4,%xmm0
	vfmadd231sd %xmm5,%xmm4,%xmm0
	vfnmadd231sd %xmm6,%xmm3,%xmm0
	vfmadd231sd %xmm6,%xmm3,%xmm0
	vfnmadd231sd %xmm5,%xmm4,%xmm0
	vfmadd231sd %xmm5,%xmm4,%xmm0
	vfnmadd231sd %xmm6,%xmm3,%xmm0
	vfmadd231sd %xmm6,%xmm3,%xmm0
	vfnmadd231sd %xmm5,%xmm4,%xmm0
	vfmadd231sd %xmm5,%xmm4,%xmm0
	vfnmadd231sd %xmm6,%xmm3,%xmm0
	vfmadd231sd %xmm5,%xmm3,%xmm0
	vfnmadd231sd %xmm5,%xmm3,%xmm0
	vfmadd231sd %xmm4,%xmm6,%xmm0
	vfnmadd231sd %xmm4,%xmm6,%xmm0
	vdivsd %xmm5,%xmm4,%xmm4
	vfmadd231sd %xmm5,%xmm3,%xmm0
	vfnmadd231sd %xmm5,%xmm3,%xmm0
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
	vmovsd %xmm0,(%rdx,%rax,8)
	cmp    %r8,%rcx
	jne    <_Z18BM_ComputeBoundSoAI8SoA20VecIdLm10000000EEEvRN9benchmark5StateE+0xe40>
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
