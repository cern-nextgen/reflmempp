	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
	mov    %r14,-0x1448(%rbp)
	lea    0x8(%r12),%r8
	lea    0x8(%rbx),%rdi
	mov    %rax,-0x1440(%rbp)
	nopl   0x0(%rax)
	mov    0x10(%r13),%r10
	mov    0x0(%r13),%r14
	add    $0x20,%r13
	mov    -0x18(%r13),%r11
	mov    -0x8(%r13),%r9
	mov    %r14,%rdx
	mov    %r10,%rsi
	shl    $0x4,%rdx
	shl    $0x4,%rsi
	mov    %r11,%rax
	mov    %r9,%rcx
	vmovsd (%rbx,%rsi,1),%xmm0
	vmovsd (%rbx,%rdx,1),%xmm3
	shl    $0x4,%rax
	shl    $0x4,%rcx
	vmovsd (%rdx,%r8,1),%xmm6
	vmovsd (%rdx,%rdi,1),%xmm4
	vmovhpd (%rbx,%rcx,1),%xmm0,%xmm0
	vmovhpd (%rbx,%rax,1),%xmm3,%xmm3
	vmovsd (%r12,%rdx,1),%xmm5
	vinsertf128 $0x1,%xmm0,%ymm3,%ymm3
	vmovsd (%r8,%rsi,1),%xmm0
	vmovhpd (%r8,%rax,1),%xmm6,%xmm6
	vmovhpd (%rdi,%rax,1),%xmm4,%xmm4
	vmovhpd (%r12,%rax,1),%xmm5,%xmm5
	vmovhpd (%r8,%rcx,1),%xmm0,%xmm0
	vinsertf128 $0x1,%xmm0,%ymm6,%ymm6
	vmovsd (%rdi,%rsi,1),%xmm0
	vaddpd %ymm3,%ymm6,%ymm9
	vsubpd %ymm6,%ymm3,%ymm12
	vmovhpd (%rdi,%rcx,1),%xmm0,%xmm0
	vinsertf128 $0x1,%xmm0,%ymm4,%ymm4
	vmovsd (%r12,%rsi,1),%xmm0
	vsubpd %ymm6,%ymm4,%ymm8
	vaddpd %ymm6,%ymm4,%ymm2
	vmovhpd (%r12,%rcx,1),%xmm0,%xmm0
	vinsertf128 $0x1,%xmm0,%ymm5,%ymm5
	vsubpd %ymm5,%ymm4,%ymm11
	vaddpd %ymm4,%ymm5,%ymm10
	vaddpd %ymm3,%ymm5,%ymm7
	vmulpd %ymm11,%ymm9,%ymm1
	vmulpd %ymm12,%ymm11,%ymm0
	vdivpd %ymm11,%ymm12,%ymm13
	vfmadd132pd 0x55bc4(%rip),%ymm1,%ymm0        # <_ZSt19piecewise_construct+0xe5>
	vaddpd %ymm1,%ymm0,%ymm0
	vsubpd %ymm5,%ymm3,%ymm1
	vfmadd231pd %ymm9,%ymm10,%ymm0
	vfmadd231pd %ymm9,%ymm10,%ymm0
	vdivpd %ymm12,%ymm11,%ymm12
	vfmadd231pd %ymm1,%ymm1,%ymm0
	vfmadd231pd %ymm8,%ymm8,%ymm0
	vfmadd231pd %ymm7,%ymm7,%ymm0
	vfmadd231pd %ymm2,%ymm2,%ymm0
	vdivpd %ymm8,%ymm8,%ymm8
	vfmadd231pd %ymm1,%ymm1,%ymm0
	vsubpd %ymm13,%ymm0,%ymm0
	vsubpd %ymm12,%ymm0,%ymm0
	vdivpd %ymm11,%ymm9,%ymm12
	vdivpd %ymm9,%ymm11,%ymm11
	vsubpd %ymm12,%ymm0,%ymm0
	vdivpd %ymm1,%ymm1,%ymm1
	vsubpd %ymm11,%ymm0,%ymm0
	vdivpd %ymm10,%ymm9,%ymm11
	vdivpd %ymm9,%ymm10,%ymm10
	vsubpd %ymm11,%ymm0,%ymm0
	vdivpd %ymm7,%ymm7,%ymm7
	vsubpd %ymm10,%ymm0,%ymm0
	vsubpd %ymm1,%ymm0,%ymm0
	vsubpd %ymm8,%ymm0,%ymm0
	vdivpd %ymm2,%ymm2,%ymm2
	vsubpd %ymm7,%ymm0,%ymm0
	vdivpd %ymm6,%ymm4,%ymm1
	vsubpd %ymm2,%ymm0,%ymm0
	vfnmadd231pd %ymm3,%ymm5,%ymm0
	vfnmadd231pd %ymm3,%ymm5,%ymm0
	vdivpd %ymm5,%ymm3,%ymm2
	vfmadd231pd %ymm3,%ymm6,%ymm0
	vfnmadd231pd %ymm4,%ymm5,%ymm0
	vfmadd231pd %ymm4,%ymm5,%ymm0
	vfnmadd231pd %ymm3,%ymm6,%ymm0
	vfmadd231pd %ymm3,%ymm6,%ymm0
	vfnmadd231pd %ymm4,%ymm5,%ymm0
	vfmadd231pd %ymm4,%ymm5,%ymm0
	vfnmadd231pd %ymm3,%ymm6,%ymm0
	vfmadd231pd %ymm3,%ymm6,%ymm0
	vfnmadd231pd %ymm4,%ymm5,%ymm0
	vfmadd231pd %ymm4,%ymm5,%ymm0
	vfnmadd231pd %ymm3,%ymm6,%ymm0
	vfmadd231pd %ymm3,%ymm5,%ymm0
	vfnmadd231pd %ymm3,%ymm5,%ymm0
	vfmadd231pd %ymm6,%ymm4,%ymm0
	vfnmadd231pd %ymm6,%ymm4,%ymm0
	vdivpd %ymm5,%ymm4,%ymm4
	vfmadd231pd %ymm3,%ymm5,%ymm0
	vfnmadd231pd %ymm3,%ymm5,%ymm0
	vaddpd %ymm0,%ymm1,%ymm0
	vaddpd %ymm1,%ymm0,%ymm0
	vdivpd %ymm6,%ymm3,%ymm3
	vsubpd %ymm2,%ymm0,%ymm0
	vaddpd %ymm2,%ymm0,%ymm0
	vsubpd %ymm3,%ymm0,%ymm0
	vaddpd %ymm4,%ymm0,%ymm0
	vsubpd %ymm4,%ymm0,%ymm0
	vaddpd %ymm3,%ymm0,%ymm0
	vsubpd %ymm3,%ymm0,%ymm0
	vaddpd %ymm4,%ymm0,%ymm0
	vsubpd %ymm4,%ymm0,%ymm0
	vaddpd %ymm3,%ymm0,%ymm0
	vsubpd %ymm3,%ymm0,%ymm0
	vaddpd %ymm4,%ymm0,%ymm0
	vsubpd %ymm4,%ymm0,%ymm0
	vaddpd %ymm3,%ymm0,%ymm0
	vsubpd %ymm2,%ymm0,%ymm0
	vaddpd %ymm2,%ymm0,%ymm0
	vsubpd %ymm1,%ymm0,%ymm0
	vaddpd %ymm1,%ymm0,%ymm0
	vsubpd %ymm2,%ymm0,%ymm0
	vaddpd %ymm2,%ymm0,%ymm0
	vsubpd %ymm1,%ymm0,%ymm0
	vaddpd %ymm1,%ymm0,%ymm0
	vmovlpd %xmm0,(%r15,%r14,8)
	vmovhpd %xmm0,(%r15,%r11,8)
	vextractf128 $0x1,%ymm0,%xmm0
	vmovlpd %xmm0,(%r15,%r10,8)
	vmovhpd %xmm0,(%r15,%r9,8)
	cmp    %r13,-0x13f8(%rbp)
	jne    <_Z18BM_ComputeBoundAoSI10AoSBaseVecIdLm10000000E2S2XadL_Z10GenerateS2IdES1_IT_EiEEEEvRN9benchmark5StateE+0x768>
	mov    -0x1448(%rbp),%r14
	vzeroupper
	call   <_ZNSt6chrono3_V212steady_clock3nowEv@plt>
