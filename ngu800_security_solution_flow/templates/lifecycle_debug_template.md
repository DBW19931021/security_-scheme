# NGU800 Lifecycle and Secure Debug Template

## Purpose
Use this template when defining lifecycle states, debug policy, authenticated debug, and trace restrictions.

## Required Sections

### 1. Lifecycle model
Recommend concrete states:
- MANUFACTURE
- DEV
- PROD
- RMA
- DECOMMISSIONED

### 2. State table
Prefer this format:

| State | Boot Policy | Debug Policy | Trace Policy | Update Policy | Notes |
|---|---|---|---|---|---|

### 3. Transition rules
Clarify:
- who triggers transition
- whether fuse-programmed
- reversible vs irreversible
- preconditions

### 4. Debug policy
Clarify:
- JTAG availability
- authenticated debug requirement
- secure monitor / SEC participation
- memory access restrictions
- trace capture restrictions

### 5. RMA policy
Clarify:
- what relaxations exist
- what remains prohibited
- whether user data erase is required
- whether attestation must expose RMA state

### 6. Board interaction
Clarify whether board policy or BMC can influence debug access.

### 7. Open issues checklist
Examples:
- lifecycle encoding width
- authenticated debug token format
- trace ownership
- board-assisted debug exceptions
