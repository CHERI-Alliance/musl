#include <sys/auxv.h>

/*
 * https://git.morello-project.org/morello/kernel/linux/-/wikis/Morello-pure-capability-kernel-user-Linux-ABI-specification#auxiliary-vector-auxv
 * Auxiliary vectors are either capabilities or values. If an auxiliary 
 * vector is a pointer, it can appear in both.
 */

#define AUX_PTR_CASES \
	case AT_ENTRY:\
	case AT_PHDR:\
    case AT_BASE:\
	case AT_SYSINFO_EHDR:\
	case AT_EXECFN:\
	case AT_RANDOM:\
	case AT_PLATFORM:\
    case AT_ARGV:\
    case AT_ENVP:\
	case AT_CHERI_EXEC_RW_CAP:\
	case AT_CHERI_EXEC_RX_CAP:\
	case AT_CHERI_INTERP_RW_CAP:\
	case AT_CHERI_INTERP_RX_CAP:\
	case AT_CHERI_STACK_CAP:\
	case AT_CHERI_SEAL_CAP:\
    case AT_CHERI_CID_CAP:\

#define AUX_VAL_CASES \
	case AT_ENTRY:\
    case AT_BASE:\
	case AT_SYSINFO_EHDR:\
	case AT_EXECFN:\
	case AT_RANDOM:\
	case AT_PLATFORM:\
    case AT_ARGC:\
    case AT_ENVC:\
	case AT_CHERI_EXEC_RW_CAP:\
	case AT_CHERI_EXEC_RX_CAP:\
	case AT_CHERI_INTERP_RW_CAP:\
	case AT_CHERI_INTERP_RX_CAP:\
	case AT_CHERI_STACK_CAP:\
	case AT_CHERI_SEAL_CAP:\
	case AT_CHERI_CID_CAP:\
