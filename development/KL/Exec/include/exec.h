/*
 * Local definitions for Executable Loader Library
 *
 * This file is used for all those definitions that implements the 
 * Executable Loader Library functionality but are NOT part of the
 * library interface, so hidden to library user.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 28/06/00 scosta    1.0    First draft
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <elf.h>

typedef enum { 
	saveElfHeader,
	elfSectionNames,
	elfSection,
	postLoad,
	elfInvalid,
	finalState
} FsmState; /* Available states of FSM */

typedef enum {
	nextState,
	continueIteration,
	stopIteration,
	stateError 
} FsmResult; /* The result of computation of a finite state machine transition */ 

typedef struct {
	FsmState state;      /* The state of the Finite State Machine */
	FsmResult result;    /* The result of FSM state rransition */
	UWORD32 iterator;    /* An iteration count used in some states */
} ExecFSM;

typedef struct {
	ExecType type;       /* Executable type (used to parse data below) */
	VOID *exec;          /* Pointer to actual executable data */
	ExecFSM fsm;         /* Finite state machine status for given exec */
} ExecutableTable;

typedef struct {
	Elf32_Ehdr header;   /* The ELF header */
	Elf32_Shdr *section; /* The pointer to the array of sections */
	UBYTE **sectionData; /* The pointer to an array of section data */
} ELF;

#define NOT_LOADED    0xFFFF /* Mark an unloaded section */

/* Local functions prototypes */

PUBLIC FsmResult ELFSectionProcessing(ELF *elf, ExecRecord *, ExecFSM *fsm);
PUBLIC FsmResult ELFSectionNamesProcessing(INPUT ELF *elf, ExecRecord *);
PUBLIC FsmResult ELFSectionHeaderProcessing(ELF *elf, ExecRecord *);
PUBLIC VOID ELFsizeAndAssignMemToBSS(INPUT ELF *elf);
PUBLIC BOOLEAN ELFresolveRelocations(INPUT ELF *elf);

PUBLIC BOOLEAN cpuDependantCheck(INPUT Elf32_Ehdr *elfHeader);
PUBLIC VOID relocateSymbol(INPUT ELF *elf, INPUT Elf32_Sym *symbol,
                           INPUT Elf32_Rel *pRelData);
PUBLIC UBYTE * ELFsearchSymbol(INPUT ELF *elf, INPUT Esymbol *symbol);
PUBLIC UWORD32 ELFsetSymbols(INPUT ELF *elf,INPUT Esymbol *symbol, INPUT UBYTE *symbolAddr[]);

PUBLIC VOID ELFclose(INPUT ELF *elf);
