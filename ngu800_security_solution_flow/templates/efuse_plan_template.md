# NGU800 eFuse / OTP Plan Template

## Purpose
Use this template to output NGU800 eFuse / OTP planning suggestions at bit / field level.

## Required Sections

### 1. Planning principles
State:
- what must be one-time programmable
- what must never be directly software-readable
- which fields are policy only
- which fields are monotonic
- which fields affect BootROM vs SEC vs eHSM

### 2. Region partition
At minimum consider:
- Root secret / seed region
- Device identity binding region
- Lifecycle region
- Debug control region
- Key revoke / key select region
- Rollback counter region
- Board policy / board binding region if needed

### 3. Bit allocation table
Prefer this format:

| Region | Bit Range | Width | Purpose | Readable By | Notes |
|---|---|---:|---|---|---|

### 4. Sensitive material rule
Clarify:
- whether Root Secret is stored directly, obfuscated, wrapped, or only internally visible
- whether BootROM can read value or only invoke service / observe derived status
- whether SEC can read raw material
- whether eHSM exclusively owns raw secret visibility

### 5. Lifecycle encoding
Recommend concrete encoding for:
- MANUFACTURE
- DEV
- PROD
- RMA
- DECOMMISSIONED
Clarify irreversible transitions.

### 6. Debug control bits
Recommend fields for:
- JTAG enable
- authenticated debug enable
- trace enable
- RMA override
- secure erase / close option if relevant

### 7. Rollback counters
Clarify:
- per firmware class or shared
- width
- update authority
- monotonic rules
- BootROM vs SEC check responsibility

### 8. Key revoke and key select bits
Clarify:
- revoke bitmap
- active key index policy
- ownership transfer / field key update implications if relevant

### 9. Board-related fuse suggestions
Consider whether fields are needed for:
- board_id binding
- board security policy
- BMC/OOB policy enable
- SMBus access restriction mode

### 10. Open issues checklist
List unresolved constraints such as:
- total fuse budget
- redundancy / ECC
- manufacturing write sequence
- late-stage field patchability limits
