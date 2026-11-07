from collections import namedtuple

LOADER_OFFSET       = 0x000000
LOADER_SIZE         = 0x100000
AMP_OFFSET          = 0x100000
#AMP_SIZE           = 0x020000
AMP_SIZE            = 0x200000
#OUTPUT_DATA_OFFSET = 0x120000
#OUTPUT_DATA_SIZE   = 0x1e0000
EXEC_OFFSET         = 0x300000
EXEC_SIZE           = 0x100000
OUTPUT_DATA_OFFSET = 0x2000000
OUTPUT_DATA_SIZE   = 0x4000000

Target = namedtuple(
    "Target",
    ["harts", "reserved_base", "hart1", "hart2", "hart3", "hart4"],
    defaults = [0, 0]  # given to rightmost arguments
)

# Targets
UNMATCHED = "unmatched"
BV_AHEAD = "bv_ahead"
BV_FIRE = "bv_fire"
BPI_F3 = "bpi_f3"
BPI_RV2 = "bpi_rv2"

unmatched_info = Target(harts=2, reserved_base=0x82000000, hart1=1, hart2=2)
bv_ahead_info = Target(harts=2, reserved_base=0x82000000, hart1=1, hart2=2)
bv_fire_info = Target(harts=2, reserved_base=0x82000000, hart1=2, hart2=3)
bpi_f3_info = Target(harts=2, reserved_base=0x132000000,
                     hart1=4, hart2=5, hart3=1, hart4=2)
bpi_rv2_info = Target(harts=2, reserved_base=0x82000000,
                      hart1=1, hart2=2, hart3=3, hart4=4)

# boards which can be selected with option parser
target_choices = [UNMATCHED, BV_AHEAD, BV_FIRE, BPI_F3, BPI_RV2]

# boards without SMT
no_affinity_rel_targets = [UNMATCHED, BV_AHEAD, BV_FIRE, BPI_F3, BPI_RV2]

# boards with alternative slots for 2 harts
alternative_slot_targets = [BPI_F3, BPI_RV2]

def mem_region_bases(proc_info):
    return (proc_info.reserved_base + LOADER_OFFSET,
            proc_info.reserved_base + AMP_OFFSET,
            proc_info.reserved_base + OUTPUT_DATA_OFFSET,
            proc_info.reserved_base + EXEC_OFFSET)




