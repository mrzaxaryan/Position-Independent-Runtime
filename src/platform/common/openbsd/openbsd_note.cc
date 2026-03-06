/**
 * @file openbsd_note.cc
 * @brief OpenBSD ELF identification note.
 *
 * @details The OpenBSD kernel requires an NT_OPENBSD_IDENT note in a PT_NOTE
 * segment to accept the binary for execution. LLD does not emit this note
 * for OpenBSD targets (verified: the only PT_NOTE LLD generates is for
 * .note.gnu.property when CET/BTI is enabled). This file adds the note via
 * module-level inline assembly. The ".section" directive creates a SHT_NOTE /
 * SHF_ALLOC section that the linker places in a PT_NOTE program header
 * segment. The note is not extracted into the PIC binary (only .text is
 * extracted), so it has no impact on shellcode size.
 */

__asm__(
	".section .note.openbsd.ident, \"a\", @note\n"
	".p2align 2\n"
	".long 8\n"                    /* namesz (including NUL) */
	".long 4\n"                    /* descsz */
	".long 1\n"                    /* type = NT_OPENBSD_IDENT */
	".ascii \"OpenBSD\\0\"\n"      /* name */
	".long 0\n"                    /* desc (version) */
	".previous\n"
);
